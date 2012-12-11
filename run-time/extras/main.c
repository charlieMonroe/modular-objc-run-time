//
//  main.c
//  modular_runtime
//
//  Created by Charlie Monroe on 11/24/12.
//  Copyright (c) 2012 Fuel Collective, LLC. All rights reserved.
//

#include <stdio.h>
#include <time.h>

#include "runtime.h"
#include "class.h"
#include "method.h"
#include "selector.h"

#define ITERATIONS 10000000
static int counter = 0;

id _MyClass_alloc(id self, SEL _cmd, ...){
	return objc_class_create_instance((Class)self, 0);
}

id _MyClass_log(id self, SEL _cmd, ...){
	counter++;
	//printf("Instance method, hallelujah - self %p, _cmd %p", self, _cmd);
	return nil;
}

int main(int argc, const char * argv[]){
	
	Class my_class = objc_class_create(Nil, "MyClass");
	printf("Created class: %p\n", my_class);
	printf("Getting class: %p\n", objc_class_for_name("MyClass"));
	objc_class_finish(my_class);
	printf("Getting class after finishing: %p\n", objc_class_for_name("MyClass"));
	
	(void)objc_class_create(Nil, "MyClass");
	
	Class my_subclass = objc_class_create(my_class, "MySubclass");
	objc_class_finish(my_subclass);
	
	SEL alloc_selector = objc_selector_register("alloc");
	printf("Registering alloc selector - %p.\n", alloc_selector);
	
	SEL log_selector = objc_selector_register("log");
	printf("Registering log selector - %p.\n", log_selector);
	
	SEL second_alloc_selector = objc_selector_register("alloc");
	printf("Registering second alloc selector should yield in the same pointers: %s (%p x %p).\n", second_alloc_selector == alloc_selector ? "YES" : "NO", alloc_selector, second_alloc_selector);
	
	Method alloc_method = objc_method_create(alloc_selector, "^@:", &_MyClass_alloc);
	printf("Created alloc method %p\n", alloc_method);
	
	objc_class_add_class_method(my_class, alloc_method);
	
	Method log_method = objc_method_create(log_selector, "^@:", &_MyClass_log);
	printf("Created log method %p\n", log_method);
	
	objc_class_add_class_method(my_class, alloc_method);
	objc_class_add_instance_method(my_class, log_method);
	
	IMP alloc_impl = objc_object_lookup_impl((id)my_subclass, alloc_selector);
	printf("Got alloc implementation: %p - equals to %p: %s\n", alloc_impl, &_MyClass_alloc, alloc_impl == &_MyClass_alloc ? "YES" : "NO");
	
	id instance = alloc_impl((id)my_subclass, alloc_selector);
	printf("Created an object instance: %p\n", instance);
	
	clock_t c1, c2;
	c1 = clock();
	for (int i = 0; i < ITERATIONS; ++i){
		IMP log_impl = objc_object_lookup_impl(instance, objc_selector_register("log"));
		log_impl(instance, log_selector);
		//_MyClass_log(instance, log_selector);
	}
	
	c2 = clock();
	
	if (counter != ITERATIONS){
		printf("counter != ITERATIONS (%d != %d)\n", counter, ITERATIONS);
	}
	
	printf("Modular run-time call took %f seconds.\n", ((double)c2 - (double)c1)/ (double)CLOCKS_PER_SEC);
	
	return 0;
}

