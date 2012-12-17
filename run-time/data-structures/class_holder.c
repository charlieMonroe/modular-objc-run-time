//
//  class_holder.c
//  modular_runtime
//
//  Created by Charlie Monroe on 11/25/12.
//  Copyright (c) 2012 Fuel Collective, LLC. All rights reserved.
//

#include "class_holder.h"
#include "class-private.h"
#include "os.h"
#include "utilities.h"

#if !OBJC_USES_INLINE_FUNCTIONS


typedef struct _class_bucket_struct {
	struct _class_bucket_struct *next;
	Class class;
} _class_bucket;

typedef struct _class_holder_str {
	objc_rw_lock lock;
	unsigned int bucket_count;
	_class_bucket **buckets; // Lazily allocated!
} *_class_holder;


/**
 * Returns a pointer to the bucket for that key.
 */
OBJC_INLINE unsigned int _class_bucket_index_for_class_name(_class_holder holder, const char *name){
	if (holder->buckets == NULL){
		holder->buckets = objc_zero_alloc(sizeof(_class_bucket) * holder->bucket_count);
	}
	return objc_hash_string(name) & (holder->bucket_count - 1);
}

OBJC_INLINE unsigned int log2u(unsigned int x) { return (x<2) ? 0 : log2u (x>>1)+1; }

#define OBJC_CACHE_GOOD_CAPACITY(c) (c <= 1 ? 1 : 1 << (log2u(c-1)+1))

/**
 * Allocates the buckets.
 */
OBJC_INLINE void _class_holder_initialize_buckets(_class_holder holder){
	holder->buckets = objc_zero_alloc(holder->bucket_count * sizeof(_class_bucket));
}

OBJC_INLINE _class_holder class_holder_create_internal(){
	_class_holder holder = (_class_holder)(objc_alloc(sizeof(struct _class_holder_str)));
	holder->buckets = NULL;
	holder->lock = objc_rw_lock_create();
	holder->bucket_count = OBJC_CACHE_GOOD_CAPACITY(64); // A usual class uses ~64 methods
	return holder;
}

OBJC_INLINE BOOL _class_holder_contains_class_in_bucket(_class_bucket *bucket, Class cl){
	while (bucket != NULL) {
		if (bucket->class == cl){
			// Already there!
			return YES;
		}
		
		bucket = bucket->next;
	}
	return NO;
}

OBJC_INLINE void class_holder_insert_class_internal(_class_holder holder, Class cl){
	unsigned int bucket_index;
	_class_bucket *bucket;
	
	if (holder->buckets == NULL){
		_class_holder_initialize_buckets(holder);
	}
	
	bucket_index = _class_bucket_index_for_class_name(holder, cl->name);
	if (_class_holder_contains_class_in_bucket(holder->buckets[bucket_index], cl)){
		// Already contains
		return;
	}
	
	// Prepare the bucket before locking in order to
	// keep the structure locked as little as possible
	bucket = (_class_bucket*)objc_alloc(sizeof(struct _class_bucket_struct));
	bucket->class = cl;
	
	// Lock the structure for insert
	objc_rw_lock_wlock(holder->lock);
	
	// Someone might have inserted it between searching and locking
	if (_class_holder_contains_class_in_bucket(holder->buckets[bucket_index], cl)){
		objc_rw_lock_unlock(holder->lock);
		objc_dealloc(bucket);
		return;
	}
	
	bucket->next = holder->buckets[bucket_index];
	holder->buckets[bucket_index] = bucket;
	
	objc_rw_lock_unlock(holder->lock);
}

OBJC_INLINE Class class_holder_fetch_class(_class_holder holder, const char *name) {
	unsigned int bucket_index = _class_bucket_index_for_class_name(holder, name);
	_class_bucket *bucket = holder->buckets[bucket_index];
	while (bucket != NULL) {
		if (objc_strings_equal(bucket->class->name, name)){
			return bucket->class;
		}
		bucket = bucket->next;
	}
	
	return NULL;
}


// Just passing all the functions to a hash table
objc_class_holder class_holder_create(void){
	return (objc_class_holder)class_holder_create_internal();
}
void class_holder_insert_class(objc_class_holder holder, Class cl){
	class_holder_insert_class_internal((_class_holder)holder, (void*)cl);
}
Class class_holder_lookup_class(objc_class_holder holder, const char *name){
	return class_holder_fetch_class((_class_holder)holder, name);
}

#endif // OBJC_USES_INLINE_FUNCTIONS
