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
	struct {
		// Keep the allocator at the top. It is likely the most often used
		// function (called with every object creation), keeping it first
		// requires no additional computation to determine the address of the
		// pointer, when called from within the structure
		objc_allocator allocator;
		objc_deallocator deallocator;
		objc_reallocator reallocator;
		objc_zero_allocator zero_allocator;
	} memory;
	struct {
		objc_abort abort;
	} execution;
	struct {
		objc_class_holder_creator creator;
		objc_class_holder_destroyer destroyer;
		objc_class_holder_inserter inserter;
		objc_class_holder_lookup lookup;
	} class_holder;
	
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

objc_runtime_create_getter_setter_function_decls(objc_abort, abort);
objc_runtime_create_getter_setter_function_decls(objc_allocator, allocator);
objc_runtime_create_getter_setter_function_decls(objc_deallocator, deallocator);
objc_runtime_create_getter_setter_function_decls(objc_reallocator, reallocator);
objc_runtime_create_getter_setter_function_decls(objc_zero_allocator, zero_allocator);
objc_runtime_create_getter_setter_function_decls(objc_class_holder_creator, class_holder_creator);
objc_runtime_create_getter_setter_function_decls(objc_class_holder_destroyer, class_holder_destroyer);
objc_runtime_create_getter_setter_function_decls(objc_class_holder_lookup, class_holder_lookup);

#endif //OBJC_RUNTIME_H_
