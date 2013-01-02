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

/**
 * A cached forwarding selector.
 */
static SEL objc_forwarding_selector = NULL;

/**
 * A function that is returned when the receiver is nil.
 */
static id _objc_nil_receiver_function(id self, SEL _cmd, ...){
	return nil;
}

/***** PRIVATE FUNCTIONS *****/

/**
 * Returns the extra space needed in the class structure by
 * the class extensions.
 */
OBJC_INLINE unsigned int _extra_class_space_for_extensions(void){
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

/**
 * Returns the extra space needed in each object structure by
 * the class extensions.
 */
OBJC_INLINE unsigned int _extra_object_space_for_extensions(void){
	static unsigned int cached_object_result = 0;
	objc_class_extension *ext;
	
	if (cached_object_result != 0 || class_extensions == NULL){
		/* The result has already been cached, or no extensions are installed. */
		return cached_object_result;
	}
	
	ext = class_extensions;
	while (ext != NULL){
		cached_object_result += ext->extra_object_space;
		ext = ext->next_extension;
	}
	
	return cached_object_result;
}

/**
 * Returns the size required for instances of class 'cl'. This
 * includes the extra space required by extensions.
 */
OBJC_INLINE unsigned int _instance_size(Class cl){
	return cl->instance_size + _extra_object_space_for_extensions();
}

/**
 * Creates a new objc_array in place of *method_list unless it's already 
 * created. In that case, it is a no-op function.
 */
OBJC_INLINE void _initialize_method_list(objc_array *method_list){
	if (*method_list != NULL){
		/* Already allocated */
		return;
	}
	
	*method_list = objc_array_create();
}

/**
 * Adds methods from the array 'm' into the method_list. The m array doesn't
 * have to be NULL-terminated, and has to contain 'count' methods.
 *
 * The m array is copied over to be NULL-terminated, but the Method 'objects'
 * are not.
 */
OBJC_INLINE void _add_methods_to_method_list(objc_array method_list, Method *m, unsigned int count){
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

/**
 * Adds class methods to class cl.
 */
OBJC_INLINE void _add_class_methods(Class cl, Method *m, unsigned int count){
	if (cl == NULL || m == NULL){
		return;
	}
	
	_initialize_method_list(&cl->class_methods);
	_add_methods_to_method_list(cl->class_methods, m, count);
}

/**
 * Adds instance methods to class cl.
 */
OBJC_INLINE void _add_instance_methods(Class cl, Method *m, unsigned int count){
	if (cl == NULL || m == NULL){
		return;
	}
	
	_initialize_method_list(&cl->instance_methods);
	_add_methods_to_method_list(cl->instance_methods, m, count);
}

/**
 * Searches for a method in a method list and returns it, or NULL 
 * if it hasn't been found.
 */
OBJC_INLINE Method _lookup_method_in_method_list(objc_array method_list, SEL selector){
	objc_array_enumerator en;
	
	if (method_list == NULL){
		return NULL;
	}
	
	en = objc_array_get_enumerator(method_list);
	while (en != NULL) {
		Method *methods = en->item;
		while (*methods != NULL){
			if (objc_selectors_equal(selector, (*methods)->selector)){
				return *methods;
			}
			++methods;
		}
		en = en->next;
	}
		
	return NULL;
}

/**
 * Searches for a method among class extensions.
 */
OBJC_INLINE Method _lookup_extension_method(id obj, SEL selector){
	Method m = NULL;
	objc_class_extension *ext;
	
	ext = class_extensions;
	while (ext != NULL){
		if (ext->lookup_function != NULL){
			m = ext->lookup_function(obj, selector);
			if (m != NULL){
				return m;
			}
		}
		
		ext = ext->next_extension;
	}
	
	return m;
}

/**
 * Looks up a method for a selector in a cache.
 */
OBJC_INLINE Method _lookup_cached_method(objc_cache cache, SEL selector){
	if (cache == NULL){
		return NULL;
	}
	return objc_cache_fetch(cache, selector);
}

/**
 * Adds a method to a cache. If *cache == NULL, it gets created.
 */
OBJC_INLINE void _cache_method(objc_cache *cache, Method m){
	if (*cache == NULL){
		*cache = objc_cache_create();
	}
	objc_cache_insert(*cache, m);
}

/**
 * Searches for a class method for a selector.
 */
OBJC_INLINE Method _lookup_class_method(Class cl, SEL selector){
	Method m;
	
	if (cl == Nil || selector == NULL){
		return NULL;
	}
	
	m = _lookup_extension_method((id)cl, selector);
	if (m != NULL){
		/* An extension returned a valid method. */
		return m;
	}
	
	while (cl != NULL){
		m = _lookup_method_in_method_list(cl->class_methods, selector);
		if (m != NULL){
			return m;
		}
		cl = cl->super_class;
	}
	return NULL;
}

/**
 * Looks up a class method implementation within a class. 
 * NULL if it hasn't been found, yet no-op function in case
 * the receiver is nil.
 */
OBJC_INLINE IMP _lookup_class_method_impl(Class cl, SEL selector){
	Method m = _lookup_cached_method(cl->class_cache, selector);
	if (m != NULL){
		return m->implementation;
	}
	
	m = _lookup_class_method(cl, selector);
	if (m == NULL){
		return NULL;
	}
	
	_cache_method(&cl->class_cache, m);
	return m->implementation;
}

/**
 * Searches for an instance method for a selector.
 */
OBJC_INLINE Method _lookup_instance_method(id obj, SEL selector){
	Class class;
	Method m;
	
	if (obj == nil || selector == NULL){
		return NULL;
	}
	
	m = _lookup_extension_method(obj, selector);
	if (m != NULL){
		/* An extension returned a valid method. */
		return m;
	}
	
	class = obj->isa;
	while (class != NULL){
		m = _lookup_method_in_method_list(class->instance_methods, selector);
		if (m != NULL){
			return m;
		}
		class = class->super_class;
	}
	return NULL;
}

/**
 * Looks up an instance method implementation within a class.
 * NULL if it hasn't been found, yet no-op function in case
 * the receiver is nil.
 */
OBJC_INLINE IMP _lookup_instance_method_impl(id obj, SEL selector){
	Method m = _lookup_cached_method(obj->isa->instance_cache, selector);
	if (m != NULL){
		return m->implementation;
	}
	
	m = _lookup_instance_method(obj, selector);
	if (m == NULL){
		return NULL;
	}
	
	_cache_method(&obj->isa->instance_cache, m);
	return m->implementation;
}

/**
 * Crashes the program because forwarding isn't supported by the class of the object.
 */
OBJC_INLINE void _forwarding_not_supported_abort(id obj, SEL selector){
	/* i.e. the object doesn't respond to the
	 forwarding selector either. */
	objc_log("%s doesn't support forwarding and doesn't respond to selector %s!\n", obj->isa->name, selector->name);
	objc_abort("Class doesn't support forwarding.");
}

/**
 * Forwards the method invocation by calling forwardMessage: method on obj
 * be it a class or an instance.
 */
OBJC_INLINE BOOL _forward_method_invocation(id obj, SEL selector){
	if (objc_selectors_equal(selector, objc_forwarding_selector)){
		/* Make sure the app really crashes. */
		return NO;
	}else{
		/* Forwarding. */
		IMP forwarding_imp;
		
		if (objc_forwarding_selector == NULL){
			objc_forwarding_selector = objc_selector_register("forwardMessage:");
		}
		
		if (OBJC_OBJ_IS_CLASS(obj)){
			forwarding_imp = _lookup_class_method_impl((Class)obj, objc_forwarding_selector);
		}else{
			forwarding_imp = _lookup_instance_method_impl(obj, objc_forwarding_selector);
		}
		
		if (forwarding_imp == NULL){
			return NO;
		}
		
		return ((BOOL(*)(id,SEL,SEL))forwarding_imp)(obj, selector, selector);
	}
}

/**
 * Calls class extension object_initializer functions.
 */
OBJC_INLINE void _complete_object(id obj){
	objc_class_extension *ext = class_extensions;
	while (ext != NULL){
		ext->object_initializer(obj, (char*)obj + ext->object_extra_space_offset);
		ext = ext->next_extension;
	}
}

/**
 * Calls class extension object_deallocator functions.
 */
OBJC_INLINE void _finalize_object(id obj){
	objc_class_extension *ext;
	
	ext = class_extensions;
	while (ext != NULL){
		ext->object_deallocator(obj, (char*)obj + ext->object_extra_space_offset);
		ext = ext->next_extension;
	}
}

OBJC_INLINE void _flush_cache(objc_cache *cache){
	if (cache == NULL){
		/* This is wrong. *cache may be NULL, cache no. */
		objc_abort("Cache == NULL in _flush_cache!");
	}
	
	if (*cache != NULL){
		objc_cache old_cache = *cache;
		*cache = NULL;
		objc_cache_destroy(old_cache);
	}
}


/***** PUBLIC FUNCTIONS *****/

void objc_class_add_class_method(Class cl, Method m){
	if (cl == NULL || m == NULL){
		return;
	}
	
	_add_class_methods(cl, &m, 1);
}
void objc_class_add_class_methods(Class cl, Method *m, unsigned int count){
	_add_class_methods(cl, m, count);
}
void objc_class_add_instance_method(Class cl, Method m){
	if (cl == NULL || m == NULL){
		return;
	}
	
	_add_instance_methods(cl, &m, 1);
}
void objc_class_add_instance_methods(Class cl, Method *m, unsigned int count){
	_add_instance_methods(cl, m, count);
}
Class objc_class_create(Class superclass, const char *name) {
	Class newClass;
	unsigned int extra_space;
	
	if (name == NULL || *name == '\0'){
		objc_abort("Trying to create a class with NULL or empty name.");
	}
	
	objc_rw_lock_wlock(objc_runtime_lock);
	if (objc_class_holder_lookup(objc_classes, name) != NULL){
		/* i.e. a class with this name already exists */
		objc_log("A class with this name already exists (%s).\n", name);
		objc_rw_lock_unlock(objc_runtime_lock);
		return NULL;
	}
	
	newClass = (Class)(objc_alloc(sizeof(struct objc_class)));
	newClass->isa = newClass; /* A loop to self to detect class method calls. */
	newClass->super_class = superclass;
	newClass->name = objc_strcpy(name);
	newClass->class_methods = NULL; /* Lazy-loading */
	newClass->instance_methods = NULL; /* Lazy-loading */
	
	/*
	 * Right now sizeof(id) as the object always includes pointer to its class.
	 * Adding or removing ivars changes the value. Doesn't include extra space.
	 */
	newClass->instance_size = sizeof(id);
	
	newClass->flags.in_construction = YES;
	
	extra_space = _extra_class_space_for_extensions();
	if (extra_space != 0){
		newClass->extra_space = objc_zero_alloc(extra_space);
	}else{
		newClass->extra_space = NULL;
	}
	
	objc_class_holder_insert(objc_classes, newClass);
	objc_rw_lock_unlock(objc_runtime_lock);
	
	return newClass;
}
void objc_complete_object(id instance){
	_complete_object(instance);
}
id objc_class_create_instance(Class cl, unsigned int extra_bytes){
	id obj;
	
	if (cl->flags.in_construction){
		objc_log("Trying to create an instance of unfinished class (%s).", cl->name);
		return nil;
	}
	
	obj = (id)objc_zero_alloc(_instance_size(cl) + extra_bytes);
	obj->isa = cl;
	
	_complete_object(obj);
	
	return obj;
}
void objc_class_finish(Class cl){
	objc_class_extension *ext;
	char *extra_space;
	
	if (cl == Nil){
		objc_abort("Cannot finish a NULL class!\n");
		return;
	}
	
	/* Pass the class through all extensions */
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
	
	/* That's it! Just mark it as not in construction */
	cl->flags.in_construction = NO;
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
void objc_finalize_object(id instance){
	_finalize_object(instance);
}
unsigned int objc_class_instance_size(Class cl){
	return _instance_size(cl);
}
Method objc_lookup_class_method(Class cl, SEL selector){
	return _lookup_class_method(cl, selector);
}
IMP objc_lookup_class_method_impl(Class cl, SEL selector){
	IMP implementation = _lookup_class_method_impl(cl, selector);
	if (implementation == NULL){
		/* Not found! Prepare for forwarding. */
		if (_forward_method_invocation((id)cl, selector)){
			return _objc_nil_receiver_function;
		}else{
			_forwarding_not_supported_abort((id)cl, selector);
			return NULL;
		}
	}
	
	return implementation;
}
Method objc_lookup_instance_method(id obj, SEL selector){
	return _lookup_instance_method(obj, selector);
}
IMP objc_lookup_instance_method_impl(id obj, SEL selector){
	IMP implementation = _lookup_instance_method_impl(obj, selector);
	if (implementation == NULL){
		/* Not found! Prepare for forwarding. */
		if (_forward_method_invocation(obj, selector)){
			return _objc_nil_receiver_function;
		}else{
			_forwarding_not_supported_abort(obj, selector);
			return NULL;
		}
	}
	
	return implementation;
}
void objc_object_deallocate(id obj){
	_finalize_object(obj);
	objc_dealloc(obj);
}
IMP objc_object_lookup_impl(id obj, SEL selector){
	IMP implementation = NULL;
	
	if (obj == nil){
		return _objc_nil_receiver_function;
	}
	
	if (OBJC_OBJ_IS_CLASS(obj)){
		/* Class method */
		implementation = _lookup_class_method_impl((Class)obj, selector);
	}else{
		/* Instance method. */
		implementation = _lookup_instance_method_impl(obj, selector);
	}
	
	if (implementation == NULL){
		/* Not found! Prepare for forwarding. */
		if (_forward_method_invocation(obj, selector)){
			return _objc_nil_receiver_function;
		}else{
			_forwarding_not_supported_abort(obj, selector);
			return NULL;
		}
	}
	
	return implementation;
}

/***** INFORMATION GETTERS *****/
BOOL objc_class_in_construction(Class cl){
	return cl->flags.in_construction;
}
const char *objc_class_get_name(Class cl){
	return cl->name;
}
Class objc_class_get_superclass(Class cl){
	return cl->super_class;
}

/***** EXTENSION-RELATED *****/
void objc_class_add_extension(objc_class_extension *extension){
	if (objc_classes != NULL){
		objc_abort("The run-time has already been initialized."
				" No class extensions may be installed at this point anymore.");
	}
	
	if (class_extensions == NULL){
		class_extensions = extension;
	}else{
		/* Prepend the extension */
		extension->next_extension = class_extensions;
		class_extensions = extension;
	}
}

/**** CACHE-RELATED ****/

/**
 * Flushing caches is a little tricky. As the structure is
 * read-lock-free, there can be multiple readers present.
 *
 * To solve this, the structure must handle this. The default
 * implementation solves this by simply marking the structure
 * as 'to be deallocated'. At the beginning of each read from
 * the cache, reader count is increased, at the end decreased.
 * 
 * Once the reader count is zero and the structure is marked
 * as to be deallocated, it is actually deallocated. This can be
 * done simply because the cache structure is marked to be
 * deleted *after* a new one has been created and placed instead.
 */

void objc_class_flush_caches(Class cl){
	if (cl == Nil){
		return;
	}
	
	_flush_cache(&cl->class_cache);
	_flush_cache(&cl->instance_cache);
}
void objc_class_flush_instance_cache(Class cl){
	if (cl == Nil){
		return;
	}
	_flush_cache(&cl->instance_cache);
}
void objc_class_flush_class_cache(Class cl){
	if (cl == Nil){
		return;
	}
	_flush_cache(&cl->class_cache);
}

/***** INITIALIZATION *****/
void objc_class_init(void){
	/* Cache the extension offsets */
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
