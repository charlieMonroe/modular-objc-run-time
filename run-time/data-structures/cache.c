//
//  cache.c
//  modular_runtime
//
//  Created by Charlie Monroe on 12/9/12.
//  Copyright (c) 2012 Fuel Collective, LLC. All rights reserved.
//

#include "cache.h"
#include "method-private.h"
#include "os.h"

typedef union {
	Method one;
	Method *many;
} one_or_many;

typedef struct {
	unsigned int count;
	one_or_many elements;
} _objc_cache_bucket;

typedef struct _objc_cache_str {
	objc_rw_lock lock;
	unsigned int entry_count;
	unsigned int bucket_count;
	
	_objc_cache_bucket *buckets; // Lazily allocated!
} *_objc_cache;


/**
 * Returns a pointer to the bucket for that key.
 */
static inline _objc_cache_bucket *_objc_cache_bucket_for_selector(_objc_cache table, const SEL selector){
	if (table->buckets == NULL){
		table->buckets = objc_zero_alloc(sizeof(_objc_cache_bucket) * table->bucket_count);
	}
	return &(table->buckets[(unsigned int)selector & (table->bucket_count - 1)]);
}

static inline void _objc_cache_grow(_objc_cache table) {
	unsigned int new_size = (table->bucket_count * 2);
	unsigned int old_size = table->bucket_count;
	_objc_cache_bucket *old_buckets = table->buckets;
	_objc_cache_bucket *new_buckets = objc_zero_alloc(new_size * sizeof(_objc_cache_bucket));
	
	table->buckets = new_buckets;
	table->bucket_count = new_size;
	
	int i;
	for (i = 0; i < old_size; ++i){
		_objc_cache_bucket bucket = old_buckets[i];
		if (bucket.count == 0){
			continue;
		}
		
		if (bucket.count == 1){
			objc_cache_insert(table, bucket.elements.one);
		}else{
			// Need to go through the bucket
			unsigned int size = bucket.count;
			unsigned int o;
			for (o = 0; o < size; ++o){
				objc_cache_insert(table, bucket.elements.many[o]);
			}
			
			// Free bucket elements
			objc_dealloc(bucket.elements.many);
		}
	}
	
	// Free buckets
	objc_dealloc(old_buckets);
}

static inline void objc_cache_clear(_objc_cache table){
	int i;
	for (i = 0; i < table->bucket_count; ++i){
		_objc_cache_bucket bucket = table->buckets[i];
		if (bucket.count == 0){
			continue;
		}
		
		if (bucket.count > 1){
			objc_dealloc(bucket.elements.many);
		}
	}
	
	// Free buckets
	objc_dealloc(table->buckets);
	table->buckets = NULL;
}

static inline unsigned int log2u(unsigned int x) { return (x<2) ? 0 : log2u (x>>1)+1; };

#define GOOD_CAPACITY(c) (c <= 1 ? 1 : 1 << (log2u(c-1)+1))


/**
 * Allocates the buckets.
 */
static inline void _objc_cache_initialize_buckets(_objc_cache table){
	table->buckets = objc_zero_alloc(table->bucket_count * sizeof(_objc_cache_bucket));
}

static inline _objc_cache objc_cache_create_internal(){
	_objc_cache cache = (_objc_cache)(objc_alloc(sizeof(struct _objc_cache_str)));
	cache->entry_count = 0;
	cache->buckets = NULL;
	cache->lock = objc_rw_lock_create();
	cache->bucket_count = GOOD_CAPACITY(1);
	return cache;
}

static inline void objc_cache_destroy_internal(_objc_cache cache){
	objc_cache_clear(cache);
	if (cache->lock != NULL){
		objc_rw_lock_destroy(cache->lock);
	}
	objc_dealloc(cache);
}
static inline void objc_cache_insert_method(_objc_cache cache, Method m){
	if (cache->buckets == NULL){
		_objc_cache_initialize_buckets(cache);
	}
	
	_objc_cache_bucket *bucket = _objc_cache_bucket_for_selector(cache, m->selector);
	unsigned int bucket_size = bucket->count;
	
	if (bucket_size == 0){
		// First item in the bucket
		++bucket->count;
		++cache->entry_count;
		bucket->elements.one = m;
		return;
	}
	
	if (bucket_size == 1){
		// Are the entries equal?
		if (bucket->elements.one == m){
			// YES -> we're done
			return;
		}
		
		// No, need to become elements.many
		Method *elements = (Method*)objc_zero_alloc(2 * sizeof(void*));
		elements[1] = bucket->elements.one;
		elements[0] = m;
		bucket->elements.many = elements;
		
		++bucket->count;
		++cache->entry_count;
		if (cache->entry_count > cache->bucket_count){
			// Too many entries
			_objc_cache_grow(cache);
		}
		return;
	}
	
	// There are more items, see if any is equal:
	Method *elements = bucket->elements.many;
	unsigned int o;
	for (o = 0; o < bucket_size; ++o){
		if (elements[o] == m){
			// Yes, return
			return;
		}
	}
	
	// No luck. Reallocate the list.
	bucket->elements.many = (Method*)objc_realloc(bucket->elements.many, bucket->count + 1);
	bucket->elements.many[bucket->count] = m;
	bucket->count++;
	cache->entry_count++;
	
	if (cache->entry_count > cache->bucket_count){
		// Too many entries
		_objc_cache_grow(cache);
	}
	
}

static Method objc_cache_fetch_method(_objc_cache table, SEL key) {
	_objc_cache_bucket *bucket = _objc_cache_bucket_for_selector(table, key);
	if (bucket->count == 0){
		return NULL;
	}
	
	if (bucket->count == 1){
		if (bucket->elements.one->selector == key){
			return bucket->elements.one;
		}
	}
	
	unsigned int i;
	for (i = 0; i < bucket->count; ++i){
		if (bucket->elements.many[i]->selector == key){
			// Found it
			return bucket->elements.many[i];
		}
	}
	
	return NULL;
}

static inline void objc_cache_rlock(_objc_cache cache){
	objc_rw_lock_rlock(cache->lock);
}
static inline void objc_cache_wlock(_objc_cache cache){
	objc_rw_lock_wlock(cache->lock);
}
static inline void objc_hash_table_unlock(_objc_cache cache){
	objc_rw_lock_unlock(cache->lock);
}




objc_cache objc_cache_create(void){
	return (objc_cache)objc_cache_create_internal();
}
Method objc_cache_fetch(objc_cache cache, SEL selector){
	return objc_cache_fetch_method((_objc_cache)cache, selector);
}
void objc_cache_insert(objc_cache cache, Method method){
	objc_cache_insert_method((_objc_cache)cache, method);
}


