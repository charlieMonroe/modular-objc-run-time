/*
 * This file contains basic types, like BOOL and OBJC_NULL.
 */

#ifndef OBJC_TYPES_H_
#define OBJC_TYPES_H_

typedef signed char BOOL;
#define YES ((BOOL)1)
#define NO ((BOOL)0)

#ifndef OBJC_NULL
	#define OBJC_NULL ((void*)0)
#endif

#endif // OBJC_TYPES_H_
