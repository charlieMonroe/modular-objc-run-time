//
//  inline-sample.h
//  modular_runtime
//
//  Created by Charlie Monroe on 12/11/12.
//  Copyright (c) 2012 Fuel Collective, LLC. All rights reserved.
//

#ifndef _INLINE_FUNCTIONS_SAMPLE_H_
#define _INLINE_FUNCTIONS_SAMPLE_H_

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "../types.h"

#define objc_log(format, ...) printf(format, ## __VA_ARGS__)

OBJC_INLINE void objc_abort(const char *reason){
	printf("Aborting because of %s.", reason);
	abort();
}

OBJC_INLINE void *objc_alloc(unsigned int size){
	return malloc(size);
}

OBJC_INLINE void *objc_realloc(void *memory, unsigned int size){
	return realloc(memory, size);
}

OBJC_INLINE void *objc_zero_alloc(unsigned int size){
	return calloc(1, size);
}
OBJC_INLINE void objc_dealloc(void *memory){
	free(memory);
}


OBJC_INLINE objc_rw_lock objc_rw_lock_create(void){
	pthread_rwlock_t *lock = malloc(sizeof(pthread_rwlock_t));
	pthread_rwlock_init(lock, NULL);
	return lock;
}
OBJC_INLINE void objc_rw_lock_destroy(objc_rw_lock lock){
	pthread_rwlock_destroy(lock);
	free(lock);
}
OBJC_INLINE void objc_rw_lock_unlock(objc_rw_lock lock){
	pthread_rwlock_unlock(lock);
}
OBJC_INLINE void objc_rw_lock_rlock(objc_rw_lock lock){
	pthread_rwlock_rdlock(lock);
}
OBJC_INLINE void objc_rw_lock_wlock(objc_rw_lock lock){
	pthread_rwlock_wrlock(lock);
}

#include "cache-inline.h"
#include "array-inline.h"
#include "class_holder-inline.h"
#include "selector_holder-inline.h"


#endif