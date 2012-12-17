//
//  selector_holder.c
//  modular_runtime
//
//  Created by Charlie Monroe on 12/9/12.
//  Copyright (c) 2012 Fuel Collective, LLC. All rights reserved.
//

#include "selector_holder.h"
#include "selector.h"
#include "os.h"
#include "utilities.h"

#if !OBJC_USES_INLINE_FUNCTIONS

typedef struct _selector_bucket_struct {
	struct _selector_bucket_struct *next;
	SEL selector;
} _selector_bucket;

typedef struct _selector_holder_str {
	objc_rw_lock lock;
	unsigned int bucket_count;
	_selector_bucket **buckets; // Lazily allocated!
} *_selector_holder;


/**
 * Returns a pointer to the bucket for that key.
 */
OBJC_INLINE unsigned int _selector_bucket_index_for_selector(_selector_holder table, const char *name){
	if (table->buckets == NULL){
		table->buckets = objc_zero_alloc(sizeof(_selector_bucket) * table->bucket_count);
	}
	return objc_hash_string(name) & (table->bucket_count - 1);
}

OBJC_INLINE unsigned int log2u(unsigned int x) { return (x<2) ? 0 : log2u (x>>1)+1; }

#define OBJC_CACHE_GOOD_CAPACITY(c) (c <= 1 ? 1 : 1 << (log2u(c-1)+1))

/**
 * Allocates the buckets.
 */
OBJC_INLINE void _selector_holder_initialize_buckets(_selector_holder table){
	table->buckets = objc_zero_alloc(table->bucket_count * sizeof(_selector_bucket));
}

OBJC_INLINE _selector_holder selector_holder_create_internal(){
	_selector_holder holder = (_selector_holder)(objc_alloc(sizeof(struct _selector_holder_str)));
	holder->buckets = NULL;
	holder->lock = objc_rw_lock_create();
	holder->bucket_count = OBJC_CACHE_GOOD_CAPACITY(64); // A usual class uses ~64 methods
	return holder;
}
OBJC_INLINE BOOL _selector_holder_contains_selector_in_bucket(_selector_bucket *bucket, SEL s){
	while (bucket != NULL) {
		if (bucket->selector == s){
			// Already there!
			return YES;
		}
		
		bucket = bucket->next;
	}
	return NO;
}

OBJC_INLINE void selector_holder_insert_selector_internal(_selector_holder holder, SEL s){
	unsigned int bucket_index;
	_selector_bucket *bucket;
	
	if (holder->buckets == NULL){
		_selector_holder_initialize_buckets(holder);
	}
	
	bucket_index = _selector_bucket_index_for_selector(holder, s->name);
	if (_selector_holder_contains_selector_in_bucket(holder->buckets[bucket_index], s)){
		// Already contains
		return;
	}
	
	// Prepare the bucket before locking in order to
	// keep the structure locked as little as possible
	bucket = (_selector_bucket*)objc_alloc(sizeof(struct _selector_bucket_struct));
	bucket->selector = s;
	
	// Lock the structure for insert
	objc_rw_lock_wlock(holder->lock);
	
	// Someone might have inserted it between searching and locking
	if (_selector_holder_contains_selector_in_bucket(holder->buckets[bucket_index], s)){
		objc_rw_lock_unlock(holder->lock);
		objc_dealloc(bucket);
		return;
	}
	
	bucket->next = holder->buckets[bucket_index];
	holder->buckets[bucket_index] = bucket;
	
	objc_rw_lock_unlock(holder->lock);
}

OBJC_INLINE SEL selector_holder_fetch_selector(_selector_holder holder, const char *name) {
	unsigned int bucket_index = _selector_bucket_index_for_selector(holder, name);
	_selector_bucket *bucket = holder->buckets[bucket_index];
	while (bucket != NULL) {
		if (objc_strings_equal(bucket->selector->name, name)){
			return bucket->selector;
		}
		bucket = bucket->next;
	}
	
	return NULL;
}


objc_selector_holder selector_holder_create(void){
	return selector_holder_create_internal();
}
void selector_holder_insert_selector(objc_selector_holder holder, SEL selector){
	selector_holder_insert_selector_internal((_selector_holder)holder, selector);
}
SEL selector_holder_lookup_selector(objc_selector_holder holder, const char *name){
	return selector_holder_fetch_selector((_selector_holder)holder, name);
}


#endif
