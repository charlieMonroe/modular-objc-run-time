/*
 * This file contains declarations of symbols private to the run-time.
 */

#ifndef OBJC_PRIVATE_H_
#define OBJC_PRIVATE_H_

#include "runtime.h" /* Needed for objc_runtime_setup_struct. */
#include "class.h" /* Needed for Class. */

extern BOOL objc_runtime_has_been_initialized;

/**
 * Initializes structures necessary for selector registration.
 */
extern void objc_selector_init(void);

/* A pointer to a structure containing all classes */
extern objc_class_holder objc_classes;

/* Actual private structure of Class. */
struct objc_class {
	Class isa; /* Points to self - this way the lookup mechanism detects class method calls */
	Class super_class;
	char *name;
	
	/*
	 * Both class and instance methods and ivars are actually
	 * arrays of arrays - each item of this objc_array is a pointer
	 * to a C array of methods/ivars.
	 *
	 * WARNING: All of them are lazily created -> may be NULL!
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

/**
 * An external run-time setup structure. This structure shouldn't be modified
 * from anywhere after the run-time is started.
 */
extern objc_runtime_setup_t objc_setup;

#endif /* OBJC_PRIVATE_H_ */
