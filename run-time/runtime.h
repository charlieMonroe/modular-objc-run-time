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
	objc_allocator_f allocator;
	objc_deallocator_f deallocator;
	objc_reallocator_f reallocator;
	objc_zero_allocator_f zero_allocator;
	objc_memory_eraser_f memory_eraser;
} objc_setup_memory;

typedef struct {
	objc_abort_f abort;
	objc_exit_f exit;
} objc_setup_execution;

typedef struct {
	objc_class_holder_creator_f creator;
	objc_class_holder_destroyer_f destroyer;
	objc_class_holder_inserter_f inserter;
	objc_class_holder_lookup_f lookup;
	objc_class_holder_rlock_f rlock;
	objc_class_holder_wlock_f wlock;
	objc_class_holder_unlock_f unlock;
} objc_setup_class_holder;

typedef struct {
	objc_log_f log;
} objc_setup_logging;

typedef struct {
	objc_rw_lock_creator_f creator;
	objc_rw_lock_destroyer_f destroyer;
	objc_rw_lock_read_lock_f rlock;
	objc_rw_lock_write_lock_f wlock;
	objc_rw_lock_unlock_f unlock;
} objc_setup_rw_lock;

typedef struct {
	objc_setup_rw_lock rwlock;
} objc_setup_sync;

typedef struct {
	objc_array_creator_f creator;
	objc_array_lockable_creator_f lockable_creator;
	objc_array_destroyer_f destroyer;
	objc_array_getter_f getter;
	objc_array_append_f append;
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

objc_runtime_create_getter_setter_function_decls(objc_abort_f, abort)
objc_runtime_create_getter_setter_function_decls(objc_exit_f, exit)
objc_runtime_create_getter_setter_function_decls(objc_allocator_f, allocator)
objc_runtime_create_getter_setter_function_decls(objc_deallocator_f, deallocator)
objc_runtime_create_getter_setter_function_decls(objc_reallocator_f, reallocator)
objc_runtime_create_getter_setter_function_decls(objc_zero_allocator_f, zero_allocator)
objc_runtime_create_getter_setter_function_decls(objc_memory_eraser_f, memory_eraser)
objc_runtime_create_getter_setter_function_decls(objc_class_holder_creator_f, class_holder_creator)
objc_runtime_create_getter_setter_function_decls(objc_class_holder_destroyer_f, class_holder_destroyer)
objc_runtime_create_getter_setter_function_decls(objc_class_holder_lookup_f, class_holder_lookup)
objc_runtime_create_getter_setter_function_decls(objc_class_holder_rlock_f, class_holder_rlock)
objc_runtime_create_getter_setter_function_decls(objc_class_holder_wlock_f, class_holder_wlock)
objc_runtime_create_getter_setter_function_decls(objc_class_holder_unlock_f, class_holder_unlock)
objc_runtime_create_getter_setter_function_decls(objc_log_f, log)
objc_runtime_create_getter_setter_function_decls(objc_rw_lock_creator_f, rw_lock_creator)
objc_runtime_create_getter_setter_function_decls(objc_rw_lock_destroyer_f, rw_lock_destroyer)
objc_runtime_create_getter_setter_function_decls(objc_rw_lock_read_lock_f, rw_lock_rlock)
objc_runtime_create_getter_setter_function_decls(objc_rw_lock_write_lock_f, rw_lock_wlock)
objc_runtime_create_getter_setter_function_decls(objc_rw_lock_unlock_f, rw_lock_unlock)
objc_runtime_create_getter_setter_function_decls(objc_array_creator_f, array_creator)
objc_runtime_create_getter_setter_function_decls(objc_array_lockable_creator_f, array_lockable_creator)
objc_runtime_create_getter_setter_function_decls(objc_array_destroyer_f, array_destroyer)
objc_runtime_create_getter_setter_function_decls(objc_array_getter_f, array_getter)
objc_runtime_create_getter_setter_function_decls(objc_array_append_f, array_append)

#endif //OBJC_RUNTIME_H_
