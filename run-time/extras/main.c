#include <stdio.h>
#include <time.h>

#include "runtime.h"
#include "class.h"
#include "method.h"
#include "selector.h"

#define DISPATCH_ITERATIONS 10000000
#define ALLOCATION_ITERATIONS 10000000

static int counter = 0;

id _MyClass_alloc(id self, SEL _cmd, ...){
	return objc_class_create_instance((Class)self, 0);
}

id _MyClass_log(id self, SEL _cmd, ...){
	counter++;
	return nil;
}

BOOL _MyClass_forward(id self, SEL _cmd, SEL selector){
	printf("Class %s supports forwarding - trying to forward selector %s.\n", objc_class_get_name(self->isa), objc_selector_get_name(selector));
	return YES;
}

static Class my_class;
static Class my_subclass;
static SEL alloc_selector;
static SEL log_selector;
static Method alloc_method;
static Method log_method;
static IMP alloc_impl;
static Method forwarding_method;
static SEL forwarding_selector;

static void create_classes(void){
	SEL second_alloc_selector;
	
	my_class = objc_class_create(Nil, "MyClass");
	objc_class_finish(my_class);
	(void)objc_class_create(Nil, "MyClass");
	
	my_subclass = objc_class_create(my_class, "MySubclass");
	objc_class_finish(my_subclass);
	
	alloc_selector = objc_selector_register("alloc");
	log_selector = objc_selector_register("log");
	forwarding_selector = objc_selector_register("forwardMessage:");
	
	second_alloc_selector = objc_selector_register("alloc");
	alloc_method = objc_method_create(alloc_selector, "^@:", &_MyClass_alloc);
	objc_class_add_class_method(my_class, alloc_method);
	log_method = objc_method_create(log_selector, "^@:", &_MyClass_log);
	
	forwarding_method = objc_method_create(forwarding_selector, "^@::", (IMP)_MyClass_forward);
	
	objc_class_add_class_method(my_class, alloc_method);
	objc_class_add_instance_method(my_class, log_method);
	objc_class_add_instance_method(my_class, forwarding_method);
	
	alloc_impl = objc_object_lookup_impl((id)my_subclass, alloc_selector);
	
}

static void print_method_list(Method *methods){
	while (*methods != NULL){
		printf("\t%s - %p\n", (*methods)->selector->name, (*methods)->implementation);
		++methods;
	}
}
static void print_ivar_list(Ivar *ivars){
	while (*ivars != NULL){
		printf("\t%s - %s - size: %d offset: %d\n", (*ivars)->name, (*ivars)->type, (*ivars)->size, (*ivars)->offset);
		++ivars;
	}
}
static void print_class(Class cl){
	printf("******** Class %s ********\n", objc_class_get_name(cl));
	printf("**** Class methods:\n");
	print_method_list(objc_class_get_class_method_list(cl));
	printf("**** Instance methods:\n");
	print_method_list(objc_class_get_instance_method_list(cl));
	printf("**** Ivars:\n");
	print_ivar_list(objc_class_get_ivar_list(cl));
	
	printf("\n\n");
}
static void list_classes(void){
	Class *classes = objc_class_get_list();
	while (*classes != NULL){
		print_class(*classes);
		++classes;
	}
}
static void method_dispatch_test(void){
	id instance;
	clock_t c1, c2;
	int i;
	
	instance = alloc_impl((id)my_subclass, alloc_selector);
	
	objc_object_lookup_impl(instance, objc_selector_register("some_selector"))(instance, objc_selector_register("some_selector"));
	
	c1 = clock();
	for (i = 0; i < DISPATCH_ITERATIONS; ++i){
		IMP log_impl = objc_object_lookup_impl(instance, log_selector);
		log_impl(instance, log_selector);
		/* _MyClass_log(instance, log_selector); */
	}
	
	c2 = clock();
	
	if (counter != DISPATCH_ITERATIONS){
		printf("counter != ITERATIONS (%d != %d)\n", counter, DISPATCH_ITERATIONS);
	}
	
	printf("Method dispatch test took %f seconds.\n", ((double)c2 - (double)c1)/ (double)CLOCKS_PER_SEC);
}
static void object_creation_test(void){
	clock_t c1, c2;
	int i;
	
	c1 = clock();
	for (i = 0; i < ALLOCATION_ITERATIONS; ++i){
		id instance = objc_class_create_instance(my_class, 0);
		objc_object_deallocate(instance);
	}
	
	c2 = clock();
		
	printf("Object allocation test took %f seconds.\n", ((double)c2 - (double)c1)/ (double)CLOCKS_PER_SEC);
}

int main(int argc, const char * argv[]){
	create_classes();
	list_classes();
	
	method_dispatch_test();
	object_creation_test();
	
	return 0;
}

