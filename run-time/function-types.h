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
typedef void*(*objc_abort)(const char*);

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
 * Adds the class to the data structure. For easier manipulation, the name
 * is passed as well, so that you don't need to call class_getName().
 */
typedef void(*objc_class_holder_inserter)(objc_class_holder, Class, const char*);

/**
 * Find a class in the objc_class_holder according to the class name. Return
 * NULL if the class isn't in the class holder.
 */
typedef Class(*objc_class_holder_lookup)(objc_class_holder, const char*);
 
#endif //OBJC_FUNCTION_TYPES_H_
