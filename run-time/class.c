/**
 * Implementation of the class-related functions.
 */

#include "class-private.h"
#include "class-extension.h"
#include "runtime-private.h"
#include "utilities.h"

// A class holder - all classes that get registered
// with the run-time get stored here.
objc_class_holder objc_classes;

// Class extension linked list
objc_class_extension *class_extensions;

static inline unsigned int _objc_extra_class_space_for_extensions(void){
	static unsigned int cached_result = 0;
	if (cached_result != 0 || class_extensions == NULL){
		// The result has already been cached, or no extensions are installed
		return cached_result;
	}
	
	objc_class_extension *ext = class_extensions;
	while (ext != NULL){
		cached_result += ext->extra_class_space;
		ext = ext->next_extension;
	}
	
	return cached_result;
}

// See header for documentation
Class objc_createClass(Class superclass, const char *name) {
	if (name == NULL || *name == '\0'){
		objc_setup.execution.abort("Trying to create a class with NULL or empty name.");
	}
	
	objc_setup.class_holder.wlock(objc_classes);
	if (objc_setup.class_holder.lookup(objc_classes, name) != NULL){
		// i.e. a class with this name already exists
		objc_setup.logging.log("A class with this name already exists (%s).\n", name);
		objc_setup.class_holder.unlock(objc_classes);
		return NULL;
	}
	
	Class newClass = (Class)(objc_setup.memory.allocator(sizeof(struct objc_class)));
	newClass->super_class = superclass;
	newClass->name = objc_strcpy(name);
	newClass->class_methods = NULL; // Lazy-loading
	newClass->instance_methods = NULL; // Lazy-loading
	newClass->flags.in_construction = YES;
	
	unsigned int extra_space = _objc_extra_class_space_for_extensions();
	if (extra_space != 0){
		newClass->extra_space = objc_setup.memory.zero_allocator(extra_space);
	}else{
		newClass->extra_space = NULL;
	}
	
	objc_setup.class_holder.inserter(objc_classes, newClass);
	objc_setup.class_holder.unlock(objc_classes);
	
	return newClass;
}

static inline void _objc_initializeMethodList(objc_array *methodList){
	if (*methodList != NULL){
		// Already allocated
		return;
	}
	
	*methodList = objc_setup.array.creator(1);
}

static inline void _objc_addMethods(objc_array method_list, Method *m, unsigned int count){
	objc_array new_list = objc_setup.array.creator(count);
	
	int i;
	for (i = 0; i < count; ++i){
		objc_setup.array.append(new_list, m[i]);
	}
	
	// TODO: check for duplicates, locking
	objc_setup.array.append(method_list, new_list);
}

void objc_addClassMethod(Class cl, Method m){
	if (cl == NULL || m == NULL){
		return;
	}
	
	objc_addClassMethods(cl, &m, 1);
}

void objc_addClassMethods(Class cl, Method *m, unsigned int count){
	if (cl == NULL || m == NULL){
		return;
	}
	
	_objc_initializeMethodList(&cl->class_methods);
	_objc_addMethods(cl->class_methods, m, count);
}

void objc_addMethod(Class cl, Method m){
	if (cl == NULL || m == NULL){
		return;
	}
	
	objc_addMethods(cl, &m, 1);
}

void objc_addMethods(Class cl, Method *m, unsigned int count){
	if (cl == NULL || m == NULL){
		return;
	}
	
	_objc_initializeMethodList(&cl->class_methods);
	_objc_addMethods(cl->instance_methods, m, count);
}

Class objc_getClass(const char *name){
	if (name == NULL){
		return Nil;
	}
	
	objc_setup.class_holder.rlock(objc_classes);
	Class c = objc_setup.class_holder.lookup(objc_classes, name);
	objc_setup.class_holder.unlock(objc_classes);
	
	if (c->flags.in_construction){
		// Still in construction
		return Nil;
	}
	
	return c;
}

void objc_finishClass(Class cl){
	// Pass the class through all extensions
	objc_class_extension *ext = class_extensions;
	void *extra_space = cl->extra_space;
	if (extra_space != NULL){
		while (ext != NULL) {
			if (ext->class_initializer != NULL){
				ext->class_initializer(cl, extra_space);
			}
			extra_space += ext->extra_class_space;
			ext = ext->next_extension;
		}
	}
	
	// That's it! Just mark it as not in construction
	cl->flags.in_construction = NO;
}

void objc_class_init(void){
	objc_classes = objc_setup.class_holder.creator();
	
	// Cache the extension offsets
	objc_class_extension *ext = class_extensions;
	unsigned int class_extra_space = 0;
	unsigned int object_extra_space = 0;
	while (ext != NULL) {
		ext->class_extra_space_offset = class_extra_space;
		ext->object_extra_space_offset = object_extra_space;
		class_extra_space += ext->extra_class_space;
		object_extra_space += ext->extra_object_space;
		ext = ext->next_extension;
	}
}

void objc_class_add_extension(objc_class_extension *extension){
	if (objc_classes != NULL){
		objc_setup.execution.abort("The run-time has already been initialized."
					   " No class extensions may be installed at this point anymore.");
	}
	
	if (class_extensions == NULL){
		class_extensions = extension;
	}else{
		// Prepend the extension
		extension->next_extension = class_extensions;
		class_extensions = extension;
	}
}
