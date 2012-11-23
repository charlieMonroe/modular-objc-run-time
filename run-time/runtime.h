/*
 * This file contains functions and structures that are meant for the run-time
 * setup and initialization.
 */

#ifndef OBJC_RUNTIME_H_
#define OBJC_RUNTIME_H_

#include "function-types.h" // For function types in the struct

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

// A macro for creating function declarations
#define objc_runtime_create_getter_setter_function_decls(type, name)\
	 extern void objc_runtime_set_##name(type name);\
	 extern type objc_runtime_get_##name(void);

objc_runtime_create_getter_setter_function_decls(objc_allocator, allocator);
objc_runtime_create_getter_setter_function_decls(objc_abort, abort);

#endif //OBJC_RUNTIME_H_
