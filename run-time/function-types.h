/*
 * This header defines function pointer types for functions used within
 * the run-time setup.
 */

#ifndef OBJC_FUNCTION_TYPES_H_
#define OBJC_FUNCTION_TYPES_H_

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
 
 
#endif //OBJC_FUNCTION_TYPES_H_
