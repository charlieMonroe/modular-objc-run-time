/**
 * This header file contains various declarations of functions, depending
 * on whether the run-time is using inline functions, or function pointers.
 *
 * If function pointers are used, the user, or dynamic loader, or other deity
 * is responsible for initializing the run-time's setup.
 *
 * If inline functions are used, the person compiling this run-time must
 * supply a header file for his OS, that implements inline functions with
 * such names as are the defines in the function-pointer section, and
 * with the same signature as the function types defined in function-types.h
 */

#ifndef OBJC_OS_H_
#define OBJC_OS_H_

#define OBJC_USES_INLINE_FUNCTIONS 0

#if OBJC_USES_INLINE_FUNCTIONS

/********* INLINE FUNCTIONS *********/
//#if TARGET_MY_OS
	//#include "os-my-os.h"
//#else
	#error "This OS isn't supported at the moment."
//#endif

#else

/********* FUNCTION POINTERS *********/

#include "runtime-private.h"

// Memory
#define objc_alloc objc_setup.memory.allocator
#define objc_realloc objc_setup.memory.reallocator
#define objc_zero_alloc objc_setup.memory.zero_allocator
#define objc_dealloc objc_setup.memory.deallocator

// Execution
#define objc_abort objc_setup.execution.abort

// Logging
#define objc_log objc_setup.logging.log
#ifdef DEBUG
	#define objc_debug_log(format, ...) objc_log(format, __VA_ARGS__)
#else
	#define objc_debug_log(format, ...)
#endif

// RW lock
#define objc_rw_lock_create objc_setup.sync.rwlock.creator
#define objc_rw_lock_rlock objc_setup.sync.rwlock.rlock
#define objc_rw_lock_wlock objc_setup.sync.rwlock.wlock
#define objc_rw_lock_unlock objc_setup.sync.rwlock.unlock
#define objc_rw_lock_destroy objc_setup.sync.rwlock.destroyer

// Selector holder
#define objc_selector_holder_create objc_setup.selector_holder.creator
#define objc_selector_holder_insert objc_setup.selector_holder.inserter
#define objc_selector_holder_lookup objc_setup.selector_holder.lookup
#define objc_selector_holder_rlock objc_setup.selector_holder.rlock
#define objc_selector_holder_wlock objc_setup.selector_holder.wlock
#define objc_selector_holder_unlock objc_setup.selector_holder.unlock

// Array
#define objc_array_count objc_setup.array.count
#define objc_array_get objc_setup.array.getter

#endif

#endif // OBJC_OS_H_
