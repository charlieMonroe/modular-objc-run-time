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

#if !OBJC_USES_INLINE_FUNCTIONS

typedef struct _cache_bucket_struct {
	struct _cache_bucket_struct *next;
	Method method;
} _cache_bucket;

typedef struct _cache_str {
	objc_rw_lock lock;
	unsigned int bucket_count;
	_cache_bucket **buckets; // Lazily allocated!
} *_cache;


/**
 * Returns a pointer to the bucket for that key.
 */
static inline unsigned int _cache_bucket_index_for_selector(_cache table, const SEL selector){
	if (table->buckets == NULL){
		table->buckets = objc_zero_alloc(sizeof(_cache_bucket) * table->bucket_count);
	}
	return (unsigned int)selector & (table->bucket_count - 1);
}

static inline unsigned int log2u(unsigned int x) { return (x<2) ? 0 : log2u (x>>1)+1; };

#define OBJC_CACHE_GOOD_CAPACITY(c) (c <= 1 ? 1 : 1 << (log2u(c-1)+1))


/**
 * Allocates the buckets.
 */
static inline void _cache_initialize_buckets(_cache table){
	table->buckets = objc_zero_alloc(table->bucket_count * sizeof(_cache_bucket));
}

static inline _cache cache_create_internal(){
	_cache cache = (_cache)(objc_alloc(sizeof(struct _cache_str)));
	cache->buckets = NULL;
	cache->lock = objc_rw_lock_create();
	cache->bucket_count = OBJC_CACHE_GOOD_CAPACITY(64); // A usual class uses ~64 methods
	return cache;
}
static inline BOOL _cache_contains_method_in_bucket(_cache_bucket *bucket, Method m){
	while (bucket != NULL) {
		if (bucket->method == m){
			// Already there!
			return YES;
		}
		
		bucket = bucket->next;
	}
	return NO;
}

static inline void cache_insert_method(_cache cache, Method m){
	if (cache->buckets == NULL){
		_cache_initialize_buckets(cache);
	}
	
	unsigned int bucket_index = _cache_bucket_index_for_selector(cache, m->selector);
	if (_cache_contains_method_in_bucket(cache->buckets[bucket_index], m)){
		// Already contains
		return;
	}
	
	// Prepare the bucket before locking in order to
	// keep the structure locked as little as possible
	_cache_bucket *bucket = (_cache_bucket*)objc_alloc(sizeof(struct _cache_bucket_struct));
	bucket->method = m;
	
	// Lock the structure for insert
	objc_rw_lock_wlock(cache->lock);
	
	// Someone might have inserted it between searching and locking
	if (_cache_contains_method_in_bucket(cache->buckets[bucket_index], m)){
		objc_rw_lock_unlock(cache->lock);
		objc_dealloc(bucket);
		return;
	}
	
	bucket->next = cache->buckets[bucket_index];
	cache->buckets[bucket_index] = bucket;
	
	objc_rw_lock_unlock(cache->lock);
}

static Method cache_fetch_method(_cache cache, SEL key) {
	unsigned int bucket_index = _cache_bucket_index_for_selector(cache, key);
	_cache_bucket *bucket = cache->buckets[bucket_index];
	while (bucket != NULL) {
		if (bucket->method->selector == key){
			return bucket->method;
		}
		bucket = bucket->next;
	}
	
	return NULL;
}


objc_cache cache_create(void){
	return (objc_cache)cache_create_internal();
}
Method cache_fetch(objc_cache cache, SEL selector){
	return cache_fetch_method((_cache)cache, selector);
}
void cache_insert(objc_cache cache, Method method){
	cache_insert_method((_cache)cache, method);
}


#endif // OBJC_USES_INLINE_FUNCTIONS
