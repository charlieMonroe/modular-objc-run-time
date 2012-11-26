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
typedef void(*objc_abort)(const char*);
 
/**
 * A function pointer to an exit function. The exit function should terminate
 * the program peacefully, just like the POSIX function.
 */
typedef void(*objc_exit)(int);


/*********** Logging ***********/

/**
  * A function pointer to a logging function. These logs contain important
  * messages to the user and shouldn't be ignored even in a release
  * environment.
  *
  * printf is a compatible function.
  */
typedef void(*objc_log)(const char*, ...);
 

/*********** Memory management ***********/

/**
 * A function pointer to an allocator. The allocator should take just one  
 * parameter - size of the memory that should be allocated. Generally should
 * work just like malloc.
 */
// TODO: change to size_t or similar, though allocating more than 4GB of memory
// at once is quite unusual... Fixme?
typedef void*(*objc_allocator)(unsigned int);
 
/**
 * A function pointer to a deallocator. The deallocator should take just one  
 * parameter - a pointer to the memory that should be freed. Generally should
 * work just like free().
 */
typedef void(*objc_deallocator)(void*);
 
/**
 * A function pointer to a reallocator. The reallocator should try to change 
 * the size of the memory to the specific size. Generally should behave just
 * like realloc().
 */
typedef void*(*objc_reallocator)(void*, unsigned int);
 
/**
 * A function pointer to a zero-allocator. Unlike the regular calloc() function,
 * this function takes only one argument, just like objc_allocator indicating 
 * the size of memory requested. It should be equivalent to calloc(1, size).
 */
typedef void*(*objc_zero_allocator)(unsigned int);
 
 
/*********** objc_class_holder ***********/

/**
 * Creates a new class holder data structure. objc_class_holder is defined in
 * types.h as simply void* and can be a pointer to virtually any data structure.
 */
typedef objc_class_holder(*objc_class_holder_creator)(void);
 
/**
 * Destroys (and frees from memory) the whole class holder data structure.
 */
typedef void(*objc_class_holder_destroyer)(objc_class_holder);

/**
 * Adds the class to the data structure.
 */
typedef void(*objc_class_holder_inserter)(objc_class_holder, Class);

/**
 * Find a class in the objc_class_holder according to the class name. Return
 * NULL if the class isn't in the class holder.
 */
typedef Class(*objc_class_holder_lookup)(objc_class_holder, const char*);

/*********** Synchronization ***********/

/**
 * Creates a new RW lock and returns a pointer to it. If the current system
 * doesn't have RW locks, a mutex can be used instead.
 */
typedef objc_rw_lock(*objc_rw_lock_creator)(void);

/**
 * Deallocates RW lock.
 */
typedef void(*objc_rw_lock_destroyer)(objc_rw_lock);

/**
 * Locks the RW lock for reading.
 */
typedef void(*objc_rw_lock_read_lock)(objc_rw_lock);

/**
 * Locks the RW lock for writing.
 */
typedef void(*objc_rw_lock_write_lock)(objc_rw_lock);

/**
 * Unlocks the RW lock.
 */
typedef void(*objc_rw_lock_unlock)(objc_rw_lock);


/*********** objc_array ***********/

/**
 * Two creators for an array - one is lockable (i.e. a rw/lock is
 * initialized), the other not.
 */
typedef objc_array(*objc_array_creator)(void);
typedef objc_array(*objc_array_lockable_creator)(void);

/**
 * Function that frees the array memory.
 */
typedef void(*objc_array_destroyer)(objc_array);

/**
 * Returns a pointer at index.
 */
typedef void*(*objc_array_getter)(objc_array, unsigned int);

/**
 * Adds an item at the end of the array;
 */
typedef void(*objc_array_append)(objc_array, void*);

#endif //OBJC_FUNCTION_TYPES_H_
