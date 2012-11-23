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
 
// A definition for a class holder - a data structure that is responsible for
// keeping a list of classes registered with the run-time, looking them up, etc.
typedef void* objc_class_holder;

#endif // OBJC_TYPES_H_
