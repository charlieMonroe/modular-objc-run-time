/*
 * This file contains functions and structures that are meant for the run-time
 * setup and initialization.
 */

#ifndef OBJC_RUNTIME_H_
#define OBJC_RUNTIME_H_

#include "function-types.h" // For function types in the struct

/**
 * Structures that defines the run-time setup. This means all functions that
 * the run-time needs for running and that can be invoked.
 *
 * Note that some function pointers that are marked as optional may be NULL.
 */

typedef struct {
	objc_allocator allocator;
	objc_deallocator deallocator;
	objc_reallocator reallocator;
	objc_zero_allocator zero_allocator;
} objc_setup_memory;

typedef struct {
	objc_abort abort;
	objc_exit exit;
} objc_setup_execution;

typedef struct {
	objc_class_holder_creator creator;
	objc_class_holder_destroyer destroyer;
	objc_class_holder_inserter inserter;
	objc_class_holder_lookup lookup;
} objc_setup_class_holder;

typedef struct {
	objc_log log;
} objc_setup_logging;

typedef struct {
	objc_rw_lock_creator creator;
	objc_rw_lock_destroyer destroyer;
	objc_rw_lock_read_lock rlock;
	objc_rw_lock_write_lock wlock;
	objc_rw_lock_unlock unlock;
} objc_setup_rw_lock;

typedef struct {
	objc_setup_rw_lock rwlock;
} objc_setup_sync;

typedef struct {
	objc_array_creator creator;
	objc_array_lockable_creator lockable_creator;
	objc_array_destroyer destroyer;
	objc_array_getter getter;
	objc_array_append append;
} objc_setup_array;

typedef struct {
	objc_setup_memory memory;
	objc_setup_execution execution;
	objc_setup_sync sync;
	
	// Logging is optional. If the logging function
	// is not available, the run-time will simply not log anything
	objc_setup_logging logging;
	
	// The following pointers are to be set optionally
	// as this run-time provides a default implementation.
	// If, however, you modify one pointer of the set,
	// all other pointers need to be modified as well...
	objc_setup_class_holder class_holder;
	objc_setup_array array;
	
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
 * This function initializes the run-time, checks for any missing function pointers,
 * fills in the built-in function pointers and seals the run-time setup.
 */
extern void objc_runtime_init(void);


/**
 * All the following functions serve as getters and setters for the functions
 * within the setup structure.
 */

// A macro for creating function declarations
#define objc_runtime_create_getter_setter_function_decls(type, name)\
	 extern void objc_runtime_set_##name(type name);\
	 extern type objc_runtime_get_##name(void);

objc_runtime_create_getter_setter_function_decls(objc_abort, abort)
objc_runtime_create_getter_setter_function_decls(objc_exit, exit)
objc_runtime_create_getter_setter_function_decls(objc_allocator, allocator)
objc_runtime_create_getter_setter_function_decls(objc_deallocator, deallocator)
objc_runtime_create_getter_setter_function_decls(objc_reallocator, reallocator)
objc_runtime_create_getter_setter_function_decls(objc_zero_allocator, zero_allocator)
objc_runtime_create_getter_setter_function_decls(objc_class_holder_creator, class_holder_creator)
objc_runtime_create_getter_setter_function_decls(objc_class_holder_destroyer, class_holder_destroyer)
objc_runtime_create_getter_setter_function_decls(objc_class_holder_lookup, class_holder_lookup)
objc_runtime_create_getter_setter_function_decls(objc_log, log)
objc_runtime_create_getter_setter_function_decls(objc_rw_lock_creator, rw_lock_creator)
objc_runtime_create_getter_setter_function_decls(objc_rw_lock_destroyer, rw_lock_destroyer)
objc_runtime_create_getter_setter_function_decls(objc_rw_lock_read_lock, rw_lock_rlock)
objc_runtime_create_getter_setter_function_decls(objc_rw_lock_write_lock, rw_lock_wlock)
objc_runtime_create_getter_setter_function_decls(objc_rw_lock_unlock, rw_lock_unlock)
objc_runtime_create_getter_setter_function_decls(objc_array_creator, array_creator)
objc_runtime_create_getter_setter_function_decls(objc_array_lockable_creator, array_lockable_creator)
objc_runtime_create_getter_setter_function_decls(objc_array_destroyer, array_destroyer)
objc_runtime_create_getter_setter_function_decls(objc_array_getter, array_getter)
objc_runtime_create_getter_setter_function_decls(objc_array_append, array_append)

#endif //OBJC_RUNTIME_H_
