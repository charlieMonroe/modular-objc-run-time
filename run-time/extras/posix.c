
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "../runtime.h"

static void *_malloc(unsigned int size){
	return malloc(size);
}
static void *_realloc(void *mem, unsigned int size){
	return realloc(mem, size);
}
static void *_zero_alloc(unsigned int size){
	return calloc(1, size);
}
static void _bzero(void *s, unsigned int size){
	bzero(s, size);
}
static void _abort(const char *reason){
	printf("__OBJC_ABORT__ - %s", reason);
	abort();
}
static objc_rw_lock _rw_lock_creator(void){
	pthread_rwlock_t *lock = malloc(sizeof(pthread_rwlock_t));
	pthread_rwlock_init(lock, NULL);
	return lock;
}
static void _rw_lock_destroyer(objc_rw_lock lock){
	pthread_rwlock_destroy(lock);
	free(lock);
}

static void _objc_posix_init(void) __attribute__((constructor));
static void _objc_posix_init(void){
	
	objc_runtime_set_allocator(_malloc);
	objc_runtime_set_deallocator(free);
	objc_runtime_set_reallocator(_realloc);
	objc_runtime_set_zero_allocator(_zero_alloc);
	objc_runtime_set_memory_eraser(_bzero);
	
	objc_runtime_set_abort(_abort);
	objc_runtime_set_exit(exit);
	
	objc_runtime_set_log(printf);
	
	objc_runtime_set_rw_lock_creator(_rw_lock_creator);
	objc_runtime_set_rw_lock_destroyer(_rw_lock_destroyer);
	objc_runtime_set_rw_lock_rlock((objc_rw_lock_read_lock_f)pthread_rwlock_rdlock);
	objc_runtime_set_rw_lock_wlock((objc_rw_lock_write_lock_f)pthread_rwlock_wrlock);
	objc_runtime_set_rw_lock_unlock((objc_rw_lock_unlock_f)pthread_rwlock_unlock);
	
	objc_runtime_init();
}
