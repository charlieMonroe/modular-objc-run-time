/*
 * This header defines function pointer types for functions used within
 * the run-time setup.
 */

#ifndef OBJC_FUNCTION_TYPES_H_
#define OBJC_FUNCTION_TYPES_H_

#include "types.h" // For objc_class, etc.

/**
 * A function pointer to an abort function. The abort function should terminate
 * the program immediately, or in case of a kernel panic. It is called in case 
 * a non-recoverable error has occurred.
 *
 * Unlike the POSIX function, this one takes one extra argument with a reason
 * why it has aborted.
 */
typedef void*(*objc_abort)(const char*);

/**
 * A function pointer to an allocator. The allocator should take just one  
 * parameter - size of the memory that should be allocated. Generally should
 * work just like malloc.
 */
typedef void*(*objc_allocator)(unsigned int);

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
 * Find a class in the objc_class_holder according to the class name. Return
 * NULL if the class isn't in the class holder.
 */
typedef Class(*objc_class_holder_lookup)(objc_class_holder, const char*);
 
#endif //OBJC_FUNCTION_TYPES_H_
