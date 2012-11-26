/**
 * Implementation of the class-related functions.
 */

#include "class-private.h"
#include "runtime-private.h"
#include "utilities.h"

// A class holder - all classes that get registered
// with the run-time get stored here.
objc_class_holder objc_classes;


// See header for documentation
Class objc_createClass(Class superclass, const char *name) {
	if (name == NULL || *name == '\0'){
		objc_setup.execution.abort("Trying to create a class with NULL or empty name.");
	}
	
	if (objc_setup.class_holder.lookup(objc_classes, name) != NULL){
		// i.e. a class with this name already exists
		objc_setup.logging.log("A class with this name already exists (%s).", name);
		return NULL;
	}
	
	Class newClass = (Class)(objc_setup.memory.allocator(sizeof(struct objc_class)));
	newClass->super_class = superclass;
	newClass->name = objc_strcpy(name);
	newClass->class_methods = NULL; // Lazy-loading
	newClass->instance_methods = NULL; // Lazy-loading
	
	return newClass;
}
