
#include "os.h"
#include "utilities.h"

#include "selector-private.h"
#include "class-private.h"
#include "method-private.h"

#if !OBJC_USES_INLINE_FUNCTIONS

typedef enum {
	holder_type_selector,
	holder_type_class,
	holder_type_cache
} holder_type;

typedef struct _bucket_struct {
	struct _bucket_struct *next;
	void *obj;
} _bucket;

/**
 * To make things easier (as this is just a sample structure
 * for the run-time), as well as include less dependencies
 * on the run-time itself, and most importantly, because
 * it's enough, there is a fixed number of buckets in the
 * structure.
 */
#define HOLDER_BUCKET_COUNT 64

#define OFFSETOF(type, field) ((unsigned int)&(((type *)0)->field))

/**
 * Structure of the actual holder.
 *
 * lock - RW lock, used only for inserting items.
 * type - one of the types above. Used to determine which key to use.
 * buckets - lazily allocated buckets (i.e. NULL at the beginning). This
 *		  can save memory for classes that aren't used by the program
 *		  so that their cache doesn't get allocated unless needed.
 * key_offset_in_object - offset of the key within the obj structure.
 * pointer_equality - whether to simply compare pointers to determine
 *				equality. Implies pointer hash, instead of key hash
 *				as well.
 */
typedef struct _holder_str {
	objc_rw_lock lock;
	unsigned int key_offset_in_object;
	BOOL pointer_equality;
	_bucket **buckets;
} *_holder;


OBJC_INLINE void _holder_initialize_buckets(_holder holder){
	holder->buckets = objc_zero_alloc(HOLDER_BUCKET_COUNT * sizeof(_bucket*));
}

OBJC_INLINE unsigned int _bucket_index_for_key(_holder holder, const void *key){
	unsigned int hash = holder->pointer_equality ? (unsigned int)key : objc_hash_string((const char*)key);
	return hash & (HOLDER_BUCKET_COUNT - 1);
}

OBJC_INLINE void *_holder_object_in_bucket(_holder holder, _bucket *bucket, void *obj){
	while (bucket != NULL) {
		if (bucket->obj == obj){
			/* Already there! */
			return bucket->obj;
		}
		
		bucket = bucket->next;
	}
	return NULL;
}

OBJC_INLINE BOOL _holder_contains_object_in_bucket(_holder holder, _bucket *bucket, void *obj){
	return (BOOL)(_holder_object_in_bucket(holder, bucket, obj) != NULL);
}

OBJC_INLINE void *_holder_object_in_bucket_for_key(_holder holder, _bucket *bucket, const void *key){
	while (bucket != NULL) {
		void *obj_key = *(void**)(((char*)bucket->obj) + holder->key_offset_in_object);
		BOOL equals = holder->pointer_equality ? (obj_key == key) : objc_strings_equal(key, obj_key);
		if (equals){
			/* Already there! */
			return bucket->obj;
		}
		
		bucket = bucket->next;
	}
	return NULL;
}

OBJC_INLINE void holder_insert_object_internal(_holder holder, void *obj){
	unsigned int bucket_index;
	_bucket *bucket;
	const void *key;
	
	if (holder->buckets == NULL){
		_holder_initialize_buckets(holder);
	}
	
	key = *(void**)((char*)obj + holder->key_offset_in_object);
	bucket_index = _bucket_index_for_key(holder, key);
	if (_holder_contains_object_in_bucket(holder, holder->buckets[bucket_index], obj)){
		/* Already contains */
		return;
	}
	
	/*
	 * Prepare the bucket before locking in order to
	 * keep the structure locked as little as possible
	 */
	bucket = (_bucket *)objc_alloc(sizeof(_bucket));
	bucket->obj = obj;
	
	/* Lock the structure for insert */
	objc_rw_lock_wlock(holder->lock);
	
	/* Someone might have inserted it between searching and locking */
	if (_holder_contains_object_in_bucket(holder, holder->buckets[bucket_index], obj)){
		objc_rw_lock_unlock(holder->lock);
		objc_dealloc(bucket);
		return;
	}
	
	bucket->next = holder->buckets[bucket_index];
	holder->buckets[bucket_index] = bucket;
	
	objc_rw_lock_unlock(holder->lock);
}

OBJC_INLINE void *holder_fetch_object(_holder holder, const void *key) {
	unsigned int bucket_index;
	_bucket *bucket;
	
	if (holder->buckets == NULL){
		/* No buckets */
		return NULL;
	}
	
	bucket_index = _bucket_index_for_key(holder, key);
	bucket = holder->buckets[bucket_index];
	return _holder_object_in_bucket_for_key(holder, bucket, key);
}

OBJC_INLINE _holder holder_create_internal(holder_type type){
	_holder holder = (_holder)(objc_alloc(sizeof(struct _holder_str)));
	holder->buckets = NULL;
	holder->lock = objc_rw_lock_create();
	holder->pointer_equality = NO;
	switch (type) {
		case holder_type_class:
			holder->key_offset_in_object = OFFSETOF(struct objc_class, name);
			break;
		case holder_type_selector:
			holder->key_offset_in_object = OFFSETOF(struct objc_selector, name);
			break;
		case holder_type_cache:
			holder->key_offset_in_object = OFFSETOF(struct objc_method, selector);
			holder->pointer_equality = YES;
			break;
		default:
			objc_abort("Unknown holder type!");
			break;
	}
	
	return holder;
}



objc_selector_holder selector_holder_create(void){
	return (objc_selector_holder)holder_create_internal(holder_type_selector);
}
void selector_holder_insert_selector(objc_selector_holder holder, SEL selector){
	holder_insert_object_internal((_holder)holder, selector);
}
SEL selector_holder_lookup_selector(objc_selector_holder holder, const char *name){
	return (SEL)holder_fetch_object((_holder)holder, name);
}


objc_class_holder class_holder_create(void){
	return (objc_class_holder)holder_create_internal(holder_type_class);
}
void class_holder_insert_class(objc_class_holder holder, Class cl){
	holder_insert_object_internal((_holder)holder, cl);
}
Class class_holder_lookup_class(objc_class_holder holder, const char *name){
	return (Class)holder_fetch_object((_holder)holder, name);
}

objc_cache cache_create(void){
	return (objc_cache)holder_create_internal(holder_type_cache);
}
Method cache_fetch(objc_cache cache, SEL selector){
	return holder_fetch_object((_holder)cache, selector);
}
void cache_insert(objc_cache cache, Method method){
	holder_insert_object_internal((_holder)cache, method);
}

#endif