/*
 * This file implements initialization and setup functions of the run-time.
 */

#include "runtime-private.h" // The public header is included here
#include "types.h" // For BOOL, YES and NO

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
		if (objc_setup.abort != NULL){
			objc_setup.abort("Cannot pass NULL setup!");
		}else{
			return;
		}
	}else if (objc_runtime_has_been_initialized == YES){
		// abort is a required function
		objc_setup.abort("Cannot modify the run-time setup after it has "
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

// A macro that creates the getter and setter function bodies.
#define objc_runtime_create_getter_setter_function_body(type, name)\
	void objc_runtime_set_##name(type name){\
		if (objc_runtime_has_been_initialized){\
			objc_setup.abort("Cannot modify the run-time " #name " after the "\
				 				"run-time has been initialized");\
		}\
	    objc_setup.name = name;\
	}\
	type objc_runtime_get_##name(void){ \
		return objc_setup.name; \
	}

objc_runtime_create_getter_setter_function_body(objc_allocator, allocator);
objc_runtime_create_getter_setter_function_body(objc_abort, abort);

