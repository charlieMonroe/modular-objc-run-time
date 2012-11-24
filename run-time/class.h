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

#endif //OBJC_CLASS_H_
