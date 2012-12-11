/*
 * This file implements initialization and setup functions of the run-time.
 */

#include "runtime-private.h" // The public header is included here
#include "array.h" // For default array implementation
#include "class_holder.h" // For default class holder imp
#include "selector_holder.h" // For default selector holder imp
#include "cache.h" // Default cache imp
#include "class-private.h" // For class_init
#include "selector-private.h" // For selector_init
#include "os.h" // To figure out if the run-time is using inline functions or function pointers

// This is marked during objc_init() as YES. After that point, no modifications
// to the setup may be made.
static BOOL objc_runtime_has_been_initialized = NO;

// The exported runtime setup structure. Private for internal use only, though.
objc_runtime_setup_struct objc_setup;

// See header for documentation
void objc_runtime_set_setup(objc_runtime_setup_struct *setup){
	if (OBJC_USES_INLINE_FUNCTIONS){
		objc_log("The run-time uses inline functions. Setting the function pointers has no effect.\n");
		return;
	}
	
	// Check if either setup is NULL or the run-time has been already 
	// initialized - if so, we need to abort
	if (setup == NULL) {
		// At this point, the abort function doesn't have to be set.
		if (objc_abort != NULL){
			objc_abort("Cannot pass NULL setup!");
		}else{
			return;
		}
	}else if (objc_runtime_has_been_initialized == YES){
		// abort is a required function
		objc_abort("Cannot modify the run-time setup after it has "
			"been initialized");
	}
	
	// Copy over everything
	objc_setup = *setup;
}

// See header for documentation
void objc_runtime_get_setup(objc_runtime_setup_struct *setup){
	if (OBJC_USES_INLINE_FUNCTIONS){
		objc_log("The run-time uses inline functions. No function pointers have been returned.\n");
		return;
	}
	
	if (setup != NULL){
		*setup = objc_setup;
	}
}


static int _objc_runtime_default_log(const char *format, ...){
	return 0;
}

#define objc_runtime_init_check_function_pointer(struct_path)\
	if (objc_setup.struct_path == NULL){\
		objc_setup.execution.abort("No function pointer set for " #struct_path "!\n");\
	}

#define objc_runtime_init_check_function_pointer_with_default_imp(struct_path, imp)\
	if (objc_setup.struct_path == NULL){\
		objc_setup.struct_path = imp;\
	}


/**
 * Validates all function pointers. Aborts the program if some of the required pointers isn't set.
 */
static void _objc_runtime_validate_function_pointers(void){
#if !OBJC_USES_INLINE_FUNCTIONS
	
	objc_runtime_init_check_function_pointer(execution.abort)
	objc_runtime_init_check_function_pointer(execution.exit)
	
	objc_runtime_init_check_function_pointer(memory.allocator)
	objc_runtime_init_check_function_pointer(memory.deallocator)
	objc_runtime_init_check_function_pointer(memory.reallocator)
	objc_runtime_init_check_function_pointer(memory.zero_allocator)
	objc_runtime_init_check_function_pointer(memory.memory_eraser)
	
	objc_runtime_init_check_function_pointer(sync.rwlock.creator)
	objc_runtime_init_check_function_pointer(sync.rwlock.destroyer)
	objc_runtime_init_check_function_pointer(sync.rwlock.rlock)
	objc_runtime_init_check_function_pointer(sync.rwlock.unlock)
	objc_runtime_init_check_function_pointer(sync.rwlock.wlock)
	
	objc_runtime_init_check_function_pointer_with_default_imp(logging.log, _objc_runtime_default_log)
	
	objc_runtime_init_check_function_pointer_with_default_imp(array.creator, array_create)
	objc_runtime_init_check_function_pointer_with_default_imp(array.lockable_creator, array_create_lockable)
	objc_runtime_init_check_function_pointer_with_default_imp(array.append, array_add)
	objc_runtime_init_check_function_pointer_with_default_imp(array.destroyer, array_destroy)
	objc_runtime_init_check_function_pointer_with_default_imp(array.getter, array_pointer_at_index)
	objc_runtime_init_check_function_pointer_with_default_imp(array.count, array_size)
	
	objc_runtime_init_check_function_pointer_with_default_imp(class_holder.creator, class_holder_create)
	objc_runtime_init_check_function_pointer_with_default_imp(class_holder.destroyer, class_holder_destroy)
	objc_runtime_init_check_function_pointer_with_default_imp(class_holder.inserter, class_holder_insert_class)
	objc_runtime_init_check_function_pointer_with_default_imp(class_holder.lookup, class_holder_lookup_class)
	objc_runtime_init_check_function_pointer_with_default_imp(class_holder.rlock, class_holder_rlock)
	objc_runtime_init_check_function_pointer_with_default_imp(class_holder.wlock, class_holder_wlock)
	objc_runtime_init_check_function_pointer_with_default_imp(class_holder.unlock, class_holder_unlock)
	
	objc_runtime_init_check_function_pointer_with_default_imp(selector_holder.creator, _objc_selector_holder_create)
	objc_runtime_init_check_function_pointer_with_default_imp(selector_holder.inserter, _objc_selector_holder_insert_selector)
	objc_runtime_init_check_function_pointer_with_default_imp(selector_holder.lookup, _objc_selector_holder_lookup_selector)
	objc_runtime_init_check_function_pointer_with_default_imp(selector_holder.rlock, _objc_selector_holder_rlock)
	objc_runtime_init_check_function_pointer_with_default_imp(selector_holder.wlock, _objc_selector_holder_wlock)
	objc_runtime_init_check_function_pointer_with_default_imp(selector_holder.unlock, _objc_selector_holder_unlock)

	objc_runtime_init_check_function_pointer_with_default_imp(cache.creator, cache_create)
	objc_runtime_init_check_function_pointer_with_default_imp(cache.fetcher, cache_fetch)
	objc_runtime_init_check_function_pointer_with_default_imp(cache.inserter, cache_insert)
	
#endif // OBJC_USES_INLINE_FUNCTIONS
}

// See header for documentation
void objc_runtime_init(void){
	// Run-time has been initialized
	objc_runtime_has_been_initialized = YES;
	
	// If inline functions aren't in use, check the function pointers
	if (!OBJC_USES_INLINE_FUNCTIONS){
		_objc_runtime_validate_function_pointers();
	}
	
	// Initialize classes
	objc_class_init();
	objc_selector_init();
}

/********** Getters and setters. ***********/
/*
 * A macro that creates the getter and setter function bodies. The type argument
 * takes in the function type (e.g. objc_allocator), the name argument is used
 * in the function name (both type and name arguments need to match the decls
 * in the header!), struct_path is the path in the objc_runtime_setup_struct
 * structure - for functions that are directly part of the structure, pass their
 * name, otherwise you need to include the sub-structure they're in as well,
 * e.g. 'class_holder.creator'.
 */
#define objc_runtime_create_getter_setter_function_body(type, name, struct_path)\
	void objc_runtime_set_##name(type name){\
		if (OBJC_USES_INLINE_FUNCTIONS){\
			objc_log("The run-time uses inline functions. Setting the function pointer " #struct_path " has no effect.\n");\
			return;\
		}\
		if (objc_runtime_has_been_initialized){\
			objc_abort("Cannot modify the run-time " #name " after the "\
				 				"run-time has been initialized");\
		}\
		objc_setup.struct_path = name;\
	}\
	type objc_runtime_get_##name(void){ \
		if (OBJC_USES_INLINE_FUNCTIONS){\
			objc_log("The run-time uses inline functions. NULL pointer has been returned.\n");\
			return NULL;\
		}\
		return objc_setup.struct_path; \
	}

objc_runtime_create_getter_setter_function_body(objc_abort_f, abort, execution.abort)
objc_runtime_create_getter_setter_function_body(objc_exit_f, exit, execution.exit)
objc_runtime_create_getter_setter_function_body(objc_allocator_f, allocator, memory.allocator)
objc_runtime_create_getter_setter_function_body(objc_deallocator_f, deallocator, memory.deallocator)
objc_runtime_create_getter_setter_function_body(objc_reallocator_f, reallocator, memory.reallocator)
objc_runtime_create_getter_setter_function_body(objc_zero_allocator_f, zero_allocator, memory.zero_allocator)
objc_runtime_create_getter_setter_function_body(objc_memory_eraser_f, memory_eraser, memory.memory_eraser)
objc_runtime_create_getter_setter_function_body(objc_class_holder_creator_f, class_holder_creator, class_holder.creator)
objc_runtime_create_getter_setter_function_body(objc_class_holder_destroyer_f, class_holder_destroyer, class_holder.destroyer)
objc_runtime_create_getter_setter_function_body(objc_class_holder_inserter_f, class_holder_inserter, class_holder.inserter)
objc_runtime_create_getter_setter_function_body(objc_class_holder_lookup_f, class_holder_lookup, class_holder.lookup)
objc_runtime_create_getter_setter_function_body(objc_class_holder_rlock_f, class_holder_rlock, class_holder.rlock)
objc_runtime_create_getter_setter_function_body(objc_class_holder_wlock_f, class_holder_wlock, class_holder.wlock)
objc_runtime_create_getter_setter_function_body(objc_class_holder_unlock_f, class_holder_unlock, class_holder.unlock)

objc_runtime_create_getter_setter_function_body(objc_selector_holder_creator_f, selector_holder_creator, selector_holder.creator)
objc_runtime_create_getter_setter_function_body(objc_selector_holder_inserter_f, selector_holder_inserter, selector_holder.inserter)
objc_runtime_create_getter_setter_function_body(objc_selector_holder_lookup_f, selector_holder_lookup, selector_holder.lookup)
objc_runtime_create_getter_setter_function_body(objc_selector_holder_rlock_f, selector_holder_rlock, selector_holder.rlock)
objc_runtime_create_getter_setter_function_body(objc_selector_holder_wlock_f, selector_holder_wlock, selector_holder.wlock)
objc_runtime_create_getter_setter_function_body(objc_selector_holder_unlock_f, selector_holder_unlock, selector_holder.unlock)

objc_runtime_create_getter_setter_function_body(objc_log_f, log, logging.log)

objc_runtime_create_getter_setter_function_body(objc_rw_lock_creator_f, rw_lock_creator, sync.rwlock.creator)
objc_runtime_create_getter_setter_function_body(objc_rw_lock_destroyer_f, rw_lock_destroyer, sync.rwlock.destroyer)
objc_runtime_create_getter_setter_function_body(objc_rw_lock_read_lock_f, rw_lock_rlock, sync.rwlock.rlock)
objc_runtime_create_getter_setter_function_body(objc_rw_lock_write_lock_f, rw_lock_wlock, sync.rwlock.wlock)
objc_runtime_create_getter_setter_function_body(objc_rw_lock_unlock_f, rw_lock_unlock, sync.rwlock.unlock)

objc_runtime_create_getter_setter_function_body(objc_array_creator_f, array_creator, array.creator)
objc_runtime_create_getter_setter_function_body(objc_array_lockable_creator_f, array_lockable_creator, array.lockable_creator)
objc_runtime_create_getter_setter_function_body(objc_array_destroyer_f, array_destroyer, array.destroyer)
objc_runtime_create_getter_setter_function_body(objc_array_getter_f, array_getter, array.getter)
objc_runtime_create_getter_setter_function_body(objc_array_append_f, array_append, array.append)
objc_runtime_create_getter_setter_function_body(objc_array_count_f, array_count, array.count)

objc_runtime_create_getter_setter_function_body(objc_cache_creator_f, cache_creator, cache.creator)
objc_runtime_create_getter_setter_function_body(objc_cache_fetcher_f, cache_fetcher, cache.fetcher)
objc_runtime_create_getter_setter_function_body(objc_cache_inserter_f, cache_inserter, cache.inserter)

