/*
 * This header file contains declarations
 * of functions that deal with the class structure.
 */

#ifndef OBJC_CLASS_H_	 
#define OBJC_CLASS_H_

#include "types.h" /* For Class, BOOL, Method, ... definitions. */

/**
 * Two simple macros that determine whether the object is
 * a class or not. As the class->isa pointer creates a loop to
 * itself, the detection is very easy.
 */
#define OBJC_OBJ_IS_CLASS(obj) ((BOOL)(obj->isa == (Class)obj))
#define OBJC_OBJ_IS_INSTANCE(obj) ((BOOL)(obj->isa != (Class)obj))

/**
  * Creates a new class with name that is a subclass of superclass.
  * 
  * The returned class isn't registered with the run-time yet,
  * you need to use the objc_registerClass function.
  *
  * Memory management note: the name is copied.
  */
extern Class objc_class_create(Class superclass, const char *name);

/**
 * The following functions add methods or a single method to the class.
 *
 * If a method with the same selector is already attached to the class,
 * this doesn't override it. This is due to the method lists being a linked
 * list and the new methods being attached to the end of the list
 * as well as maintaining behavior of other run-times.
 *
 * Caller is responsible for the method 'objects' to be kept in memory,
 * however, the method array does get copied. In particular, when you
 * call the add_*_methods function, you are responsible for each *m,
 * however, you may free(m) when done.
 */
extern void objc_class_add_class_method(Class cl, Method m);
extern void objc_class_add_class_methods(Class cl, Method *m, unsigned int count);
extern void objc_class_add_instance_method(Class cl, Method m);
extern void objc_class_add_instance_methods(Class cl, Method *m, unsigned int count);

/**
 * This function allocates enough space to contain an instance
 * of the class, including the space required by class extensions,
 * plus extra_bytes, which is generally just for compatibility reasons,
 * as you should use class extensions instead.
 *
 * Each class extension gets called, if it implements an object
 * initializer function.
 *
 * If you do not use this function for object creation, you need to
 * call the complete_object function (see below) on each created object.
 */
extern id objc_class_create_instance(Class cl, unsigned int extra_bytes);

/**
 * If, for whatever reason, you do not use the run-time function
 * to create an instance (see above), always *do* call the finalize function
 * in order for class extensions to work. This function just calls
 * the object initializers of class extensions.
 */
extern void objc_complete_object(id instance);

/**
 * Deallocates an instance and calls all object_deallocator
 * functions of class extensions.
 *
 * If, again, for whatever reason, you are not using
 * the run-time functions, to create and deallocate
 * objects, use the function below so that the class
 * extension object_deallocator functions are called.
 */
extern void objc_object_deallocate(id obj);

/**
 * If, for whatever reason, you do not use the run-time function
 * to deallocate the object, you must call this function so that
 * all class extensions can deallocate anything they might have 
 * allocated.
 */
extern void objc_finalize_object(id instance);

/**
 * Returns size of an instance of a class. This size does include
 * the space required by class extensions.
 */
extern unsigned int objc_class_instance_size(Class cl);

/**
 * Finds a class registered with the run-time and returns it,
 * or Nil, if no such class is registered.
 *
 * Note that if the class is currently being in construction,
 * Nil is returned anyway.
 */
extern Class objc_class_for_name(const char *name);

/**
 * This function marks the class as finished (i.e. not in construction)
 * and calls class initializers of all class extensions on that class.
 * You need to call this before creating any instances.
 */
extern void objc_class_finish(Class cl);

/**
 * The following functions lookup a method on a class, or an object.
 *
 * Each class has two caches - one for instances, one for classes. 
 * The run-time first looks into the appropriate cache - if a Method
 * object is found, it is returned (or the IMP).
 *
 * If the method is not found, the class extension lookup functions
 * are called (this is so that the extensions may override methods,
 * allowing categories, proxies, etc.).
 *
 * If the class extensions do not find the method, the class hierarchy
 * is climbed and searched for such a method.
 *
 * Once a method is found, at any point of the lookup, it is cached for
 * later use.
 *
 * If, for example, a class extension changes in a way that the lookup would
 * result in a different method implementation, it is responsible to clear
 * the class caches (see the cache-related functions below).
 *
 * NOTE: The functions looking up method *implementation* NEVER
 * return NULL! If cl or obj are Nil (or nil), a no-op function is returned.
 * If the method is not found, the class is then searched for a selector
 * -(BOOL)forwardMessage:(SEL)selector; and the class or the object is 
 * then called with this method. If the class doesn't implement 
 * the forwardMessage: method, or the method returns NO, 
 * the program is aborted. If it returns YES, a no-op function pointer
 * is returned.
 *
 * This is a simplified forwarding mechanism with less overhead. In case
 * the forwarding mechanism used by Apple and others is needed for
 * compatibility reasons, it can be easily implemented within this method.
 */
extern Method objc_lookup_class_method(Class cl, SEL selector);
extern IMP objc_lookup_class_method_impl(Class cl, SEL selector);
extern Method objc_lookup_instance_method(id obj, SEL selector);
extern IMP objc_lookup_instance_method_impl(id obj, SEL selector);

/**
 * This function differs from the previous ones that it is preferred to be
 * used by the compiler. It detects whether obj is a class or an instance
 * and handles the situation depending on that.
 */
extern IMP objc_object_lookup_impl(id obj, SEL selector);


/**** INFORMATION GETTERS ****/

/**
 * Returns YES if the class is currently in construction.
 * This generally means that objc_class_finish hasn't been
 * called with this class yet.
 */
extern BOOL objc_class_in_construction(Class cl);

/**
 * Returns the name of the class.
 */
extern const char *objc_class_get_name(Class cl);

/**
 * Returns the superclass of cl, or Nil if it's a root class.
 */
extern Class objc_class_get_superclass(Class cl);


/**** CACHE-RELATED ****/

/**
 * These cache-related functions flush the caches of a class,
 * which then requires the lookup function to search for the
 * method implementation once again.
 *
 * These functions should be used by class extensions in case
 * they have loaded new methods, or similar, so that the old cached
 * method implementations aren't returned.
 */
extern void objc_class_flush_caches(Class cl);
extern void objc_class_flush_instance_cache(Class cl);
extern void objc_class_flush_class_cache(Class cl);

#endif /* OBJC_CLASS_H_ */
