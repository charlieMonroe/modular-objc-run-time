/**
 * A function pointer to an abort function. The abort function should terminate
 * the program immediately, or in case of a kernel panic.
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
 