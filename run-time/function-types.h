/*
 * This header defines function pointer types for functions used within
 * the run-time setup.
 */

#ifndef OBJC_FUNCTION_TYPES_H_
#define OBJC_FUNCTION_TYPES_H_

#include "types.h" // For objc_class, etc.

/*********** Execution ***********/

/**
 * A function pointer to an abort function. The abort function should terminate
 * the program immediately, or in case of a kernel panic. It is called in case 
 * a non-recoverable error has occurred.
 *
 * Unlike the POSIX function, this one takes one extra argument with a reason
 * why it has aborted.
 */
typedef void(*objc_abort_f)(const char*);
 
/**
 * A function pointer to an exit function. The exit function should terminate
 * the program peacefully, just like the POSIX function.
 */
typedef void(*objc_exit_f)(int);


/*********** Logging ***********/

/**
  * A function pointer to a logging function. These logs contain important
  * messages to the user and shouldn't be ignored even in a release
  * environment.
  *
  * printf is a compatible function.
  */
typedef int(*objc_log_f)(const char*, ...);
 

/*********** Memory management ***********/

/**
 * A function pointer to an allocator. The allocator should take just one  
 * parameter - size of the memory that should be allocated. Generally should
 * work just like malloc.
 */
// TODO: change to size_t or similar, though allocating more than 4GB of memory
// at once is quite unusual... Fixme?
typedef void*(*objc_allocator_f)(unsigned int);
 
/**
 * A function pointer to a deallocator. The deallocator should take just one  
 * parameter - a pointer to the memory that should be freed. Generally should
 * work just like free().
 */
typedef void(*objc_deallocator_f)(void*);
 
/**
 * A function pointer to a reallocator. The reallocator should try to change 
 * the size of the memory to the specific size. Generally should behave just
 * like realloc().
 */
typedef void*(*objc_reallocator_f)(void*, unsigned int);
 
/**
 * A function pointer to a zero-allocator. Unlike the regular calloc() function,
 * this function takes only one argument, just like objc_allocator indicating 
 * the size of memory requested. It should be equivalent to calloc(1, size).
 */
typedef void*(*objc_zero_allocator_f)(unsigned int);

/**
 * A function pointer to zero-out a chunk of memory. Functionality should
 * be the same as bzero.
 */
typedef void(*objc_memory_eraser_f)(void*, unsigned int);
 
 
/*********** objc_class_holder ***********/

/**
 * Creates a new class holder data structure. objc_class_holder is defined in
 * types.h as simply void* and can be a pointer to virtually any data structure.
 */
typedef objc_class_holder(*objc_class_holder_creator_f)(void);
 
/**
 * Destroys (and frees from memory) the whole class holder data structure.
 */
typedef void(*objc_class_holder_destroyer_f)(objc_class_holder);

/**
 * Adds the class to the data structure.
 */
typedef void(*objc_class_holder_inserter_f)(objc_class_holder, Class);

/**
 * Find a class in the objc_class_holder according to the class name. Return
 * NULL if the class isn't in the class holder.
 */
typedef Class(*objc_class_holder_lookup_f)(objc_class_holder, const char*);

/**
 * The class holder should be a lockable structure, allowing access from
 * multiple threads. The structure should use the locks provided by the
 * run-time so that in case the locks are set to no-op in single-threaded
 * environment, no locking is indeed performed.
 *
 * To speed things up, a rw lock should be used. If the structure used
 * is a lock-free structure, you can simply no-op these functions (they
 * still need to be non-NULL, though).
 */
typedef void(*objc_class_holder_rlock_f)(objc_class_holder);
typedef void(*objc_class_holder_wlock_f)(objc_class_holder);
typedef void(*objc_class_holder_unlock_f)(objc_class_holder);


/*********** objc_selector_holder ***********/

/**
 * Creates a new selector holder data structure. objc_selector_holder is defined in
 * types.h as simply void* and can be a pointer to virtually any data structure.
 */
typedef objc_selector_holder(*objc_selector_holder_creator_f)(void);

/**
 * Adds the class to the data structure.
 */
typedef void(*objc_selector_holder_inserter_f)(objc_selector_holder, SEL);

/**
 * Find a selector in the objc_selector_holder according to the class name. Return
 * NULL if the selector isn't in the selector holder.
 */
typedef SEL(*objc_selector_holder_lookup_f)(objc_selector_holder, const char*);

/**
 * The selector holder should be a lockable structure, allowing access from
 * multiple threads. The structure should use the locks provided by the
 * run-time so that in case the locks are set to no-op in single-threaded
 * environment, no locking is indeed performed.
 *
 * To speed things up, a rw lock should be used. If the structure used
 * is a lock-free structure, you can simply no-op these functions (they
 * still need to be non-NULL, though).
 */
typedef void(*objc_selector_holder_rlock_f)(objc_selector_holder);
typedef void(*objc_selector_holder_wlock_f)(objc_selector_holder);
typedef void(*objc_selector_holder_unlock_f)(objc_selector_holder);

/*********** Synchronization ***********/

/**
 * Creates a new RW lock and returns a pointer to it. If the current system
 * doesn't support RW locks, a mutex can be used instead.
 */
typedef objc_rw_lock(*objc_rw_lock_creator_f)(void);

/**
 * Deallocates RW lock.
 */
typedef void(*objc_rw_lock_destroyer_f)(objc_rw_lock);

/**
 * Locks the RW lock for reading. Returns 0 if successful,
 * non-zero for error. The error code is defined by the
 * lock implementation.
 */
typedef int(*objc_rw_lock_read_lock_f)(objc_rw_lock);

/**
 * Locks the RW lock for writing. Returns 0 if successful,
 * non-zero for error. The error code is defined by the
 * lock implementation.
 */
typedef int(*objc_rw_lock_write_lock_f)(objc_rw_lock);

/**
 * Unlocks the RW lock. Returns 0 if successful,
 * non-zero for error. The error code is defined by the
 * lock implementation.
 */
typedef int(*objc_rw_lock_unlock_f)(objc_rw_lock);


/*********** objc_array ***********/

/**
 * Two creators for an array - one is lockable (i.e. a rw/lock is
 * initialized), the other not. The argument is initial capacity of 
 * the array.
 */
typedef objc_array(*objc_array_creator_f)(unsigned int);
typedef objc_array(*objc_array_lockable_creator_f)(unsigned int);

/**
 * Function that frees the array memory.
 */
typedef void(*objc_array_destroyer_f)(objc_array);

/**
 * Returns a pointer at index.
 */
typedef void*(*objc_array_getter_f)(objc_array, unsigned int);

/**
 * Adds an item at the end of the array;
 */
typedef void(*objc_array_append_f)(objc_array, void*);

/**
 * Returns number of elements in the array;
 */
typedef unsigned int (*objc_array_count_f)(objc_array);

#endif //OBJC_FUNCTION_TYPES_H_
