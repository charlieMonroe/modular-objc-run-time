/*
 * This file implements initialization and setup functions of the run-time.
 */

#include "runtime-private.h" // The public header is included here

// This is marked during objc_init() as YES. After that point, no modifications
// to the setup may be made.
static BOOL objc_runtime_has_been_initialized = NO;

// The exported runtime setup structure. Private for internal use only, though.
objc_runtime_setup_struct objc_setup;

// See header for documentation
void objc_runtime_set_setup(objc_runtime_setup_struct *setup){
	// Check if either setup is NULL or the run-time has been already 
	// initialized - if so, we need to abort
	if (setup == NULL) {
		// At this point, the abort function doesn't have to be set.
		if (objc_setup.execution.abort != NULL){
			objc_setup.execution.abort("Cannot pass NULL setup!");
		}else{
			return;
		}
	}else if (objc_runtime_has_been_initialized == YES){
		// abort is a required function
		objc_setup.execution.abort("Cannot modify the run-time setup after it has "
			"been initialized");
	}
	
	// Copy over everything
	objc_setup = *setup;
}

// See header for documentation
void objc_runtime_get_setup(objc_runtime_setup_struct *setup){
	if (setup != NULL){
		*setup = objc_setup;
	}
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
		if (objc_runtime_has_been_initialized){\
			objc_setup.execution.abort("Cannot modify the run-time " #name " after the "\
				 				"run-time has been initialized");\
		}\
	    objc_setup.struct_path = name;\
	}\
	type objc_runtime_get_##name(void){ \
		return objc_setup.struct_path; \
	}

objc_runtime_create_getter_setter_function_body(objc_abort, abort, execution.abort)
objc_runtime_create_getter_setter_function_body(objc_allocator, allocator, memory.allocator)
objc_runtime_create_getter_setter_function_body(objc_deallocator, deallocator, memory.deallocator)
objc_runtime_create_getter_setter_function_body(objc_reallocator, reallocator, memory.reallocator)
objc_runtime_create_getter_setter_function_body(objc_zero_allocator, zero_allocator, memory.zero_allocator)
objc_runtime_create_getter_setter_function_body(objc_class_holder_creator, class_holder_creator, class_holder.creator)
objc_runtime_create_getter_setter_function_body(objc_class_holder_destroyer, class_holder_destroyer, class_holder.destroyer)
objc_runtime_create_getter_setter_function_body(objc_class_holder_inserter, class_holder_inserter, class_holder.inserter)
objc_runtime_create_getter_setter_function_body(objc_class_holder_lookup, class_holder_lookup, class_holder.lookup)

