/**
 * Implementation of the class-related functions.
 */

#include "class-private.h"
#include "class-extension.h"
#include "os.h"
#include "utilities.h"
#include "method-private.h"
#include "selector.h"

/**
 * A class holder - all classes that get registered
 * with the run-time get stored here.
 */
objc_class_holder objc_classes;

/**
 * Class extension linked list.
 */
objc_class_extension *class_extensions;

/**
 * A lock that is used for manipulating with classes - e.g. adding a class
 * to the run-time, etc.
 *
 * All actions performed with this lock locked, should be rarely performed,
 * or at least performed seldomly.
 */
objc_rw_lock objc_runtime_lock;

static id _objc_nil_receiver_function(id self, SEL _cmd, ...){
	return nil;
}

OBJC_INLINE unsigned int _objc_extra_class_space_for_extensions(void){
	static unsigned int cached_class_result = 0;
	objc_class_extension *ext;
	
	if (cached_class_result != 0 || class_extensions == NULL){
		/* The result has already been cached, or no extensions are installed */
		return cached_class_result;
	}
	
	ext = class_extensions;
	while (ext != NULL){
		cached_class_result += ext->extra_class_space;
		ext = ext->next_extension;
	}
	
	return cached_class_result;
}

OBJC_INLINE unsigned int _objc_extra_object_space_for_extensions(void){
	static unsigned int cached_object_result = 0;
	objc_class_extension *ext;
	
	if (cached_object_result != 0 || class_extensions == NULL){
		// The result has already been cached, or no extensions are installed
		return cached_object_result;
	}
	
	ext = class_extensions;
	while (ext != NULL){
		cached_object_result += ext->extra_object_space;
		ext = ext->next_extension;
	}
	
	return cached_object_result;
}

// See header for documentation
Class objc_class_create(Class superclass, const char *name) {
	Class newClass;
	unsigned int extra_space;
	
	if (name == NULL || *name == '\0'){
		objc_abort("Trying to create a class with NULL or empty name.");
	}
	
	objc_rw_lock_wlock(objc_runtime_lock);
	if (objc_class_holder_lookup(objc_classes, name) != NULL){
		// i.e. a class with this name already exists
		objc_log("A class with this name already exists (%s).\n", name);
		objc_rw_lock_unlock(objc_runtime_lock);
		return NULL;
	}
	
	newClass = (Class)(objc_alloc(sizeof(struct objc_class)));
	newClass->isa = newClass; // A loop to self to detect class method calls
	newClass->super_class = superclass;
	newClass->name = objc_strcpy(name);
	newClass->class_methods = NULL; // Lazy-loading
	newClass->instance_methods = NULL; // Lazy-loading
	
	// Right now sizeof(Class) as the object always includes pointer to its class.
	// Adding or removing ivars changes the value. Doesn't include extra space.
	newClass->instance_size = sizeof(Class);
	
	newClass->flags.in_construction = YES;
	
	extra_space = _objc_extra_class_space_for_extensions();
	if (extra_space != 0){
		newClass->extra_space = objc_zero_alloc(extra_space);
	}else{
		newClass->extra_space = NULL;
	}
	
	objc_class_holder_insert(objc_classes, newClass);
	objc_rw_lock_unlock(objc_runtime_lock);
	
	return newClass;
}

OBJC_INLINE void _objc_initialize_method_list(objc_array *method_list){
	if (*method_list != NULL){
		// Already allocated
		return;
	}
	
	*method_list = objc_array_create();
}

OBJC_INLINE void _objc_class_add_methods(objc_array method_list, Method *m, unsigned int count){
	/*
	 * +1 is for the NULL termination.
	 */
	Method *methods_copy = objc_alloc((count + 1) * sizeof(Method));
	int i;
	for (i = 0; i < count; ++i){
		methods_copy[i] = m[i];
	}
	methods_copy[i] = NULL;
	
	/*
	 * Just like Apple's run-time, we do not check for duplicates.
	 * As the method list gets appended at the end, a duplicated 
	 * method would simply be ignored by the run-time as the
	 * original would be found before that in the method lookup.
	 * This mechanism allows to override a function of a superclass,
	 * however.
	 */
	
	objc_array_append(method_list, methods_copy);
}

OBJC_INLINE void _objc_class_add_class_methods(Class cl, Method *m, unsigned int count){
	if (cl == NULL || m == NULL){
		return;
	}
	
	_objc_initialize_method_list(&cl->class_methods);
	_objc_class_add_methods(cl->class_methods, m, count);
}

void objc_class_add_class_method(Class cl, Method m){
	if (cl == NULL || m == NULL){
		return;
	}
	
	_objc_class_add_class_methods(cl, &m, 1);
}

void objc_class_add_class_methods(Class cl, Method *m, unsigned int count){
	_objc_class_add_class_methods(cl, m, count);
}

OBJC_INLINE void _objc_class_add_instance_methods(Class cl, Method *m, unsigned int count){
	if (cl == NULL || m == NULL){
		return;
	}
	
	_objc_initialize_method_list(&cl->instance_methods);
	_objc_class_add_methods(cl->instance_methods, m, count);
}

void objc_class_add_instance_method(Class cl, Method m){
	if (cl == NULL || m == NULL){
		return;
	}
	
	_objc_class_add_instance_methods(cl, &m, 1);
}

void objc_class_add_instance_methods(Class cl, Method *m, unsigned int count){
	_objc_class_add_instance_methods(cl, m, count);
}

Class objc_class_for_name(const char *name){
	Class c;
	
	if (name == NULL){
		return Nil;
	}
	
	c = objc_class_holder_lookup(objc_classes, name);
	if (c == NULL || c->flags.in_construction){
		/* NULL, or still in construction */
		return Nil;
	}
	
	return c;
}

void objc_class_finish(Class cl){
	objc_class_extension *ext;
	char *extra_space;
	
	if (cl == Nil){
		objc_abort("Cannot finish a NULL class!\n");
		return;
	}
	
	// Pass the class through all extensions
	ext = class_extensions;
	extra_space = (char*)cl->extra_space;
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

id objc_class_create_instance(Class cl, unsigned int extra_bytes){
	unsigned int size;
	id obj;
	objc_class_extension *ext;
	
	if (cl->flags.in_construction){
		objc_log("Trying to create an instance of unfinished class (%s).", cl->name);
		return nil;
	}
	
	size = cl->instance_size + _objc_extra_object_space_for_extensions() + extra_bytes;
	obj = (id)objc_zero_alloc(size);
	obj->isa = cl;
	
	ext = class_extensions;
	while (ext != NULL){
		ext->object_initializer(obj, (char*)obj + ext->object_extra_space_offset);
		ext = ext->next_extension;
	}
	
	return obj;
}

void objc_class_deallocate_instance(id obj){
	objc_class_extension *ext;
	
	ext = class_extensions;
	while (ext != NULL){
		ext->object_deallocator(obj, (char*)obj + ext->object_extra_space_offset);
		ext = ext->next_extension;
	}
	
	objc_dealloc(obj);
}

OBJC_INLINE Method _objc_lookup_method_in_method_list(objc_array method_list, SEL selector){
	objc_array_enumerator en;
	
	if (method_list == NULL){
		return NULL;
	}
	
	en = objc_array_get_enumerator(method_list);
	do {
		Method *methods = en->item;
		while (*methods != NULL){
			if (objc_selectors_equal(selector, (*methods)->selector)){
				return *methods;
			}
			++methods;
		}
	} while (en->next != NULL);
		
	return NULL;
}

OBJC_INLINE Method _objc_lookup_class_method(Class cl, SEL selector){
	if (cl == Nil || selector == NULL){
		return NULL;
	}
	
	while (cl != NULL){
		Method m = _objc_lookup_method_in_method_list(cl->class_methods, selector);
		if (m != NULL){
			return m;
		}
		cl = cl->super_class;
	}
	return NULL;
}

OBJC_INLINE Method _objc_lookup_cached_method(objc_cache cache, SEL selector){
	if (cache == NULL){
		return NULL;
	}
	return objc_cache_fetch(cache, selector);
}
OBJC_INLINE void _objc_cache_method(objc_cache *cache, Method m){
	if (*cache == NULL){
		*cache = objc_cache_create();
	}
	objc_cache_insert(*cache, m);
}

OBJC_INLINE IMP _objc_lookup_class_method_impl(Class cl, SEL selector){
	Method m = _objc_lookup_cached_method(cl->class_cache, selector);
	if (m != NULL){
		return m->implementation;
	}
	
	m = _objc_lookup_class_method(cl, selector);
	if (m == NULL){
		return _objc_nil_receiver_function;
	}
	
	_objc_cache_method(&cl->class_cache, m);
	return m->implementation;
}

Method objc_lookup_class_method(Class cl, SEL selector){
	return _objc_lookup_class_method(cl, selector);
}

IMP objc_lookup_class_method_impl(Class cl, SEL selector){
	return _objc_lookup_class_method_impl(cl, selector);
}

OBJC_INLINE Method _objc_lookup_instance_method(id obj, SEL selector){
	Class class;
	
	if (obj == nil || selector == NULL){
		return NULL;
	}
	
	class = obj->isa;
	while (class != NULL){
		Method m = _objc_lookup_method_in_method_list(class->instance_methods, selector);
		if (m != NULL){
			return m;
		}
		class = class->super_class;
	}
	return NULL;
}

OBJC_INLINE IMP _objc_lookup_instance_method_impl(id obj, SEL selector){
	Method m = _objc_lookup_cached_method(obj->isa->instance_cache, selector);
	if (m != NULL){
		return m->implementation;
	}
	
	m = _objc_lookup_instance_method(obj, selector);
	if (m == NULL){
		return _objc_nil_receiver_function;
	}
	
	_objc_cache_method(&obj->isa->instance_cache, m);
	return m->implementation;
}

Method objc_lookup_instance_method(id obj, SEL selector){
	return _objc_lookup_instance_method(obj, selector);
}

IMP objc_lookup_instance_method_impl(id obj, SEL selector){
	return _objc_lookup_instance_method_impl(obj, selector);
}

IMP objc_object_lookup_impl(id obj, SEL selector){
	if (obj == nil){
		return _objc_nil_receiver_function;
	}
	
	if ((Class)obj == obj->isa){
		// Class method
		return _objc_lookup_class_method_impl((Class)obj, selector);
	}
	return _objc_lookup_instance_method_impl(obj, selector);
}

void objc_class_init(void){
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
	
	objc_runtime_lock = objc_rw_lock_create();
	
	objc_classes = objc_class_holder_create();
}

BOOL objc_class_in_construction(Class cl){
	return cl->flags.in_construction;
}

void objc_class_add_extension(objc_class_extension *extension){
	if (objc_classes != NULL){
		objc_abort("The run-time has already been initialized."
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
