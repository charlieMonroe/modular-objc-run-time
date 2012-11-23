/*
 * This file contains basic types, like BOOL and OBJC_NULL.
 */

#ifndef OBJC_TYPES_H_
#define OBJC_TYPES_H_

// A boolean type.
typedef signed char BOOL;
#define YES ((BOOL)1)
#define NO ((BOOL)0)

// NULL definition.
#ifndef NULL
	#define NULL ((void*)0)
#endif
 
/********** Class ***********/
// Opaque declaration of a Class. The actual structure is private to 
// the run-time, declared in class.c.
typedef struct objc_class *Class;
 
// Definition of id - a pointer to an object - a struct, where the first field
// is so-called isa, in GCC run-time called 'class_pointer'. I've chosen to keep
// the 'isa' name.
typedef struct objc_object {
	Class isa;
} *id;

// Definitions of nil and Nil.
// nil is used for objects, Nil for classes. It doesn't really matter,
// but all traditional run-times use this as "type checking".
#define nil ((id)0)
#define Nil ((Class)0)
 
 
// A definition for a class holder - a data structure that is responsible for
// keeping a list of classes registered with the run-time, looking them up, etc.
typedef void* objc_class_holder;



#endif // OBJC_TYPES_H_
