
#ifndef OBJC_CLASS_PRIVATE_H_
#define OBJC_CLASS_PRIVATE_H_

#include "class.h"

/* A pointer to a structure containing all classes */
extern objc_class_holder objc_classes;

/* Actual private structure of Class. */
struct objc_class {
	Class isa; /* Points to self - this way the lookup mechanism detects class method calls */
	Class super_class;
	char *name;
	
	/*
	 * Both class and instance methods are actually
	 * arrays of arrays - each item of this array is an array
	 * representing a set of methods, implemented by e.g.
	 * a category.
	 *
	 * WARNING: Both are lazily created -> may be NULL!
	 */
	objc_array class_methods;
	objc_array instance_methods;
	objc_array ivars;
	
	/* Cache */
	objc_cache class_cache;
	objc_cache instance_cache;
	
	
	unsigned int instance_size; /* Doesn't include class extensions */
	struct {
		BOOL in_construction : 1;
	} flags;
	
	void *extra_space;
};

void objc_class_init(void);

#endif /* OBJC_CLASS_PRIVATE_H_ */
