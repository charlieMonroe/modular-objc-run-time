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

// Opaque declaration of a Method. The actual structure is private to
// the run-time, declared in method.c.
typedef struct objc_method *Method;

// A definition of a SEL.
typedef struct objc_selector {
	const char *name;
} *SEL;

// Definition of id - a pointer to an object - a struct, where the first field
// is so-called isa, in GCC run-time called 'class_pointer'. I've chosen to keep
// the 'isa' name.
typedef struct objc_object {
	Class isa;
} *id;

// A definition of a method implementation function pointer
typedef id(*IMP)(id target, SEL _cmd, ...);

// Definitions of nil and Nil.
// nil is used for objects, Nil for classes. It doesn't really matter,
// but all traditional run-times use this as "type checking".
#define nil ((id)0)
#define Nil ((Class)0)
 
 
// A definition for a class holder - a data structure that is responsible for
// keeping a list of classes registered with the run-time, looking them up, etc.
// Priority of this structure should be lookup speed.
typedef void *objc_class_holder;

// A definition for a selector holder - a data structure that is responsible for
// keeping a list of selectors registered with the run-time, looking them up, etc.
// Priority of this structure should be both lookup and insertion speed.
typedef void *objc_selector_holder;

// A definition for a cache - a data structure that is responsible for
// keeping a list of IMPs to be found by SEL.
// Priority of this structure should be both lookup and insertion speed.
typedef void *objc_cache;

// A definition for a dynamically growing array structure. The easiest 
// implementation is to create a structure which includes a counter of objects,
// size of the array, and a dynamically allocated C-array of inserted pointers.
// It is used to keep arrays of methods, protocols, etc. on a class.
// This run-time provides a default implementation of such an array.
typedef void *objc_array;

// A definition for a read/write lock.
typedef void *objc_rw_lock;


#endif // OBJC_TYPES_H_
