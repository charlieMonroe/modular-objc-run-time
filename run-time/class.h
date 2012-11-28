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
extern Class objc_createClass(Class superclass, const char *name);

extern void objc_addClassMethod(Class cl, Method m);
extern void objc_addClassMethods(Class cl, Method *m, unsigned int count);
extern void objc_addMethod(Class cl, Method m);
extern void objc_addMethods(Class cl, Method *m, unsigned int count);
extern Class objc_getClass(const char *name);
extern void objc_finishClass(Class cl);

#endif //OBJC_CLASS_H_
