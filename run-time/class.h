/*
 * This header file contains the definition of a class as well as declarations
 * of functions that deal with the class structure.
 */

#ifndef OBJC_CLASS_H_	 
#define OBJC_CLASS_H_

#include "types.h"

/**
  * Creates a new class with name that is a subclass of superclass.
  * 
  * The returned class isn't registered with the run-time yet,
  * you need to use the objc_registerClass function.
  *
  * Memory management note: the name is copied.
  */
extern Class objc_class_create(Class superclass, const char *name);

extern void objc_class_add_class_method(Class cl, Method m);
extern void objc_class_add_class_methods(Class cl, Method *m, unsigned int count);
extern void objc_class_add_instance_method(Class cl, Method m);
extern void objc_class_add_instance_methods(Class cl, Method *m, unsigned int count);

extern id objc_class_create_instance(Class cl, unsigned int extra_bytes);

extern Class objc_class_for_name(const char *name);
extern BOOL objc_class_in_construction(Class cl);
extern void objc_class_finish(Class cl);

extern Method objc_lookup_class_method(Class cl, SEL selector);
extern IMP objc_lookup_class_method_impl(Class cl, SEL selector);
extern Method objc_lookup_instance_method(id obj, SEL selector);
extern IMP objc_lookup_instance_method_impl(id obj, SEL selector);

extern IMP objc_object_lookup_impl(id obj, SEL selector);

#endif //OBJC_CLASS_H_
