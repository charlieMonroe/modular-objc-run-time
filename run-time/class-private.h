//
//  class-private.h
//  modular_runtime
//
//  Created by Charlie Monroe on 11/24/12.
//  Copyright (c) 2012 Fuel Collective, LLC. All rights reserved.
//

#ifndef OBJC_CLASS_PRIVATE_H_
#define OBJC_CLASS_PRIVATE_H_

#include "class.h"

// A pointer to a structure containing all classes
extern objc_class_holder objc_classes;

// Actual private structure of Class.
struct objc_class {
	Class super_class;
	char *name;
	
	// Both class and instance methods are actually
	// arrays of arrays - each item of this array is an array
	// representing a set of methods, implemented by e.g.
	// a category
	//
	// WARNING: Both are lazily created!
	objc_array class_methods;
	objc_array instance_methods;
};

#endif // OBJC_CLASS_PRIVATE_H_
