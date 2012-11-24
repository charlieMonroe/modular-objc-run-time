/**
 * Implementation of the class-related functions.
 */

#include "class.h"
#include "runtime-private.h"
#include "utilities.h"

// A class holder - all classes that get registered
// with the run-time get stored here.
objc_class_holder objc_classes;

// Actual private structure of Class.
struct objc_class {
	Class super_class;
	char *name;
};


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
	newClass->name = objc_strcpy(name);
	
	return newClass;
}
