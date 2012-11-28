//
//  hashtable.c
//  modular_runtime
//
//  Created by Charlie Monroe on 11/26/12.
//  Copyright (c) 2012 Fuel Collective, LLC. All rights reserved.
//
#include "hashtable.h"
#include "utilities.h"
#include "runtime-private.h"


typedef union {
	void *one;
	void **many;
} one_or_many;

typedef struct {
	unsigned int count;
	one_or_many elements;
} _objc_hash_table_bucket;

struct _objc_hash_table_str {
	const char*(*keyGetter)(void*);
	BOOL(*equalityFunction)(void*, void*);
	objc_rw_lock lock;
	unsigned int entry_count;
	unsigned int bucket_count;
	
	_objc_hash_table_bucket *buckets; // Lazily allocated!
};


/**
 * Computes a hash of a key.
 */
static inline unsigned int _objc_hash_table_hash(const char *key){
	register unsigned int hash = 0;
	register unsigned char *s = (unsigned char *)key;
	
	for (; ; ) {
		if (*s == '\0') break;
		hash ^= (unsigned int)*s++;
		if (*s == '\0') break;
		hash ^= (unsigned int)*s++ << 8;
		if (*s == '\0') break;
		hash ^= (unsigned int)*s++ << 16;
		if (*s == '\0') break;
		hash ^= (unsigned int)*s++ << 24;
	}
	return hash;
}

/**
 * Returns a pointer to the bucket for that key.
 */
static inline _objc_hash_table_bucket *_objc_hash_table_bucket_for_key(_objc_hash_table table, const char *key){
	if (table->buckets == NULL){
		table->buckets = objc_setup.memory.zero_allocator(sizeof(_objc_hash_table_bucket) * table->bucket_count);
	}
	return &(table->buckets[_objc_hash_table_hash(key) & (table->bucket_count - 1)]);
}
static inline _objc_hash_table_bucket *_objc_hash_table_bucket_for_obj(_objc_hash_table table, void *obj){
	return _objc_hash_table_bucket_for_key(table, table->keyGetter(obj));
}


static inline void _objc_hash_table_grow(_objc_hash_table table) {
	unsigned int new_size = (table->bucket_count * 2);
	unsigned int old_size = table->bucket_count;
	_objc_hash_table_bucket *old_buckets = table->buckets;
	_objc_hash_table_bucket *new_buckets = objc_setup.memory.zero_allocator(new_size * sizeof(_objc_hash_table_bucket));
	
	table->buckets = new_buckets;
	table->bucket_count = new_size;
	
	int i;
	for (i = 0; i < old_size; ++i){
		_objc_hash_table_bucket bucket = old_buckets[i];
		if (bucket.count == 0){
			continue;
		}
		
		if (bucket.count == 1){
			objc_hash_table_insert(table, bucket.elements.one);
		}else{
			// Need to go through the bucket
			unsigned int size = bucket.count;
			unsigned int o;
			for (o = 0; o < size; ++o){
				objc_hash_table_insert(table, bucket.elements.many[o]);
			}
			
			// Free bucket elements
			objc_setup.memory.deallocator(bucket.elements.many);
		}
	}
	
	// Free buckets
	objc_setup.memory.deallocator(old_buckets);
}

static inline void objc_hash_table_clear(_objc_hash_table table){
	int i;
	for (i = 0; i < table->bucket_count; ++i){
		_objc_hash_table_bucket bucket = table->buckets[i];
		if (bucket.count == 0){
			continue;
		}
		
		if (bucket.count > 1){
			objc_setup.memory.deallocator(bucket.elements.many);
		}
	}
	
	// Free buckets
	objc_setup.memory.deallocator(table->buckets);
	table->buckets = NULL;
}

static inline unsigned int log2u(unsigned int x) { return (x<2) ? 0 : log2u (x>>1)+1; };

#define GOOD_CAPACITY(c) (c <= 1 ? 1 : 1 << (log2u(c-1)+1))


/**
 * Allocates the buckets.
 */
static inline void _objc_hash_table_initialize_buckets(_objc_hash_table table){
	table->buckets = objc_setup.memory.zero_allocator(table->bucket_count * sizeof(_objc_hash_table_bucket));
}


_objc_hash_table objc_hash_table_create(unsigned int capacity, const char*(*keyGetter)(void*), BOOL(*equalityFunction)(void*, void*)){
	_objc_hash_table table = (_objc_hash_table)(objc_setup.memory.allocator(sizeof(struct _objc_hash_table_str)));
	table->entry_count = 0;
	table->buckets = NULL;
	table->lock = NULL;
	table->bucket_count = GOOD_CAPACITY(capacity);
	table->keyGetter = keyGetter;
	table->equalityFunction = equalityFunction;
	return table;
}

_objc_hash_table objc_hash_table_create_lockable(unsigned int capacity, const char*(*keyGetter)(void*), BOOL(*equalityFunction)(void*, void*)){
	_objc_hash_table table = objc_hash_table_create(capacity, keyGetter, equalityFunction);
	table->lock = objc_setup.sync.rwlock.creator();
	return table;
}

void objc_hash_table_destroy(_objc_hash_table table){
	objc_hash_table_clear(table);
	if (table->lock != NULL){
		objc_setup.sync.rwlock.destroyer(table->lock);
	}
	objc_setup.memory.deallocator(table);
}
void objc_hash_table_insert(_objc_hash_table table, void *obj){
	if (table->buckets == NULL){
		_objc_hash_table_initialize_buckets(table);
	}
	
	_objc_hash_table_bucket *bucket = _objc_hash_table_bucket_for_obj(table, obj);
	unsigned int bucket_size = bucket->count;
	
	if (bucket_size == 0){
		// First item in the bucket
		bucket->count++;
		bucket->elements.one = obj;
		table->entry_count++;
		return;
	}
	
	if (bucket_size == 1){
		// Are the entries equal?
		if (table->equalityFunction(obj, bucket->elements.one)){
			// YES -> we're done
			return;
		}
		
		// No, need to become elements.many
		void **elements = (void**)objc_setup.memory.zero_allocator(2 * sizeof(void*));
		elements[1] = bucket->elements.one;
		elements[0] = obj;
		bucket->elements.many = elements;
		
		bucket->count++;
		table->entry_count++;
		if (table->entry_count > table->bucket_count){
			// Too many entries
			_objc_hash_table_grow(table);
		}
		return;
	}
	
	// There are more items, see if any is equal:
	void **elements = bucket->elements.many;
	unsigned int o;
	for (o = 0; o < bucket_size; ++o){
		if (table->equalityFunction(elements[o], obj)){
			// Yes, return
			return;
		}
	}
	
	// No luck. Reallocate the list.
	bucket->elements.many = (void**)objc_setup.memory.reallocator(bucket->elements.many, bucket->count + 1);
	bucket->elements.many[bucket->count] = obj;
	bucket->count++;
	table->entry_count++;
	
	if (table->entry_count > table->bucket_count){
		// Too many entries
		_objc_hash_table_grow(table);
	}
}
	
void *objc_hash_table_get(_objc_hash_table table, const char *key) {
	_objc_hash_table_bucket *bucket = _objc_hash_table_bucket_for_key(table, key);
	if (bucket->count == 0){
		return NULL;
	}
	
	if (bucket->count == 1){
		return objc_strings_equal(key, table->keyGetter(bucket->elements.one)) ? bucket->elements.one : NULL;
	}
	
	unsigned int i;
	for (i = 0; i < bucket->count; ++i){
		if (objc_strings_equal(key, table->keyGetter(bucket->elements.many[i]))){
			// Found it
			return bucket->elements.many[i];
		}
	}
	
	return NULL;
}

void objc_hash_table_rlock(_objc_hash_table table){
	objc_setup.sync.rwlock.rlock(table->lock);
}
void objc_hash_table_wlock(_objc_hash_table table){
	objc_setup.sync.rwlock.wlock(table->lock);
}
void objc_hash_table_unlock(_objc_hash_table table){
	objc_setup.sync.rwlock.unlock(table->lock);
}
