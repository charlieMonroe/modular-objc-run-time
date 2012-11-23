#include "function-types.h"

typedef signed char BOOL;
#define YES ((BOOL)1)
#define NO ((BOOL)0)

#ifndef NULL
	#define NULL ((void*)0)
#endif

/**
 * A structure that defines the run-time setup. This means all functions that
 * the run-time needs for running and that can be invoked.
 *
 * Note that some function pointers that are marked as optional may be NULL.
 */
typedef struct {
	objc_abort abort;
	objc_allocator allocator;
} objc_runtime_setup_struct;


/**
 * This function copies over all the function pointers. If you want to set just
 * a few of the pointers, consider using one of the functions below instead.
 *
 * If the run-time has already been initialized, calling this function aborts
 * execution of the program.
 */
extern void objc_runtime_set_setup(objc_runtime_setup_struct *setup);

/**
 * Copies over all the current function pointers into the setup structure.
 */
extern void objc_runtime_get_setup(objc_runtime_setup_struct *setup);



/**
 * All the following functions serve as getters and setters for the functions
 * within the setup structure.
 */

// A macro for creating declares
#define objc_runtime_create_getter_setter_function_decls(type, name)\
	 extern void objc_runtime_set_##name(type *name);\
	 extern type objc_runtime_get_##name(void);

objc_runtime_create_getter_setter_function_decls(objc_allocator, allocator);
objc_runtime_create_getter_setter_function_decls(objc_abort, abort);

