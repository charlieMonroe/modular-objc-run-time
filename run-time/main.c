#include <stdio.h>
#include <time.h>

#include "runtime.h"
#include "class.h"
#include "method.h"
#include "selector.h"

#include "extras/ao-ext.h"

#define DISPATCH_ITERATIONS 10000000
#define ALLOCATION_ITERATIONS 10000000

typedef struct {
	Class isa;
	int i;
} MyClass;

id _MyClass_alloc(id self, SEL _cmd, ...){
	return objc_class_create_instance((Class)self, 0);
}

id _MyClass_log(MyClass *self, SEL _cmd, ...){
	++self->i;
	return nil;
}

id _MySubclass_log(MyClass *self, SEL _cmd, ...){
	/* i.e. [super _cmd]; */
	objc_super super;
	IMP super_imp;
	super.receiver = (id)self;
	super.class = objc_class_get_superclass(self->isa);
	super_imp = objc_object_lookup_impl_super(&super, _cmd);
	super_imp((id)self, _cmd);
	
	++self->i;
	
	return nil;
}

id _MyClass_increase_via_setters_and_getters(MyClass *self, SEL _cmd, ...){
	int *old_value_ptr;
	int new_value;
	Ivar i_ivar = objc_class_get_ivar(objc_object_get_class((id)self), "i");
	
	old_value_ptr = objc_object_get_variable((id)self, i_ivar);
	new_value = *old_value_ptr + 1;
	objc_object_set_variable((id)self, i_ivar, &new_value);
	return nil;
}

id _MyClass_increase_via_ao(MyClass *self, SEL _cmd, ...){
	unsigned int old_value = (unsigned int)objc_object_get_associated_object((id)self, (void*)_cmd);
	objc_object_set_associated_object((id)self, (void*)_cmd, (id)(old_value + 1));
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
static SEL increase_via_gs_selector;
static SEL increase_via_ao_selector;
static SEL forwarding_selector;

static Method alloc_method;
static Method log_method;
static Method increase_via_gs_method;
static Method increase_via_ao_method;
static Method forwarding_method;

static IMP alloc_impl;


static void create_classes(void){
	SEL second_alloc_selector;
	
	my_class = objc_class_create(Nil, "MyClass");
	
	alloc_selector = objc_selector_register("alloc");
	log_selector = objc_selector_register("log");
	increase_via_gs_selector = objc_selector_register("increaseViaGS");
	increase_via_ao_selector = objc_selector_register("increaseViaAO");
	forwarding_selector = objc_selector_register("forwardMessage:");
	second_alloc_selector = objc_selector_register("alloc");
	if (second_alloc_selector != alloc_selector){
		printf("alloc selector (%p) != second_alloc_selector (%p)!", alloc_selector, second_alloc_selector);
		
		/* Force-crash */
		((IMP)(NULL))(nil, NULL);
	}
	
	objc_class_add_ivar(my_class, "i", sizeof(int), __alignof(int), "i");
	objc_class_finish(my_class);
	(void)objc_class_create(Nil, "MyClass");
	
	my_subclass = objc_class_create(my_class, "MySubclass");
	objc_class_finish(my_subclass);
	
	
	alloc_method = objc_method_create(alloc_selector, "^@:", &_MyClass_alloc);
	objc_class_add_class_method(my_class, alloc_method);
	
	log_method = objc_method_create(log_selector, "^@:", (IMP)&_MyClass_log);
	objc_class_add_instance_method(my_class, log_method);
	
	increase_via_gs_method = objc_method_create(increase_via_gs_selector, "^@:", (IMP)&_MyClass_increase_via_setters_and_getters);
	objc_class_add_instance_method(my_class, increase_via_gs_method);
	
	increase_via_ao_method = objc_method_create(increase_via_ao_selector, "^@:", (IMP)&_MyClass_increase_via_ao);
	objc_class_add_instance_method(my_class, increase_via_ao_method);
	
	forwarding_method = objc_method_create(forwarding_selector, "^@::", (IMP)_MyClass_forward);
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
	MyClass *instance;
	clock_t c1, c2;
	int i;
	int *result;
	Ivar i_ivar;
	
	instance = (MyClass*)alloc_impl((id)my_subclass, alloc_selector);
	
	objc_object_lookup_impl((id)instance, objc_selector_register("some_selector"))((id)instance, objc_selector_register("some_selector"));
	
	c1 = clock();
	for (i = 0; i < DISPATCH_ITERATIONS; ++i){
		IMP log_impl = objc_object_lookup_impl((id)instance, log_selector);
		log_impl((id)instance, log_selector);
		/* _MyClass_log(instance, log_selector); */
	}
	
	c2 = clock();
	
	i_ivar = objc_class_get_ivar(my_class, "i");
	result = (int*)objc_object_get_variable((id)instance, i_ivar);
	if (*result != DISPATCH_ITERATIONS){
		printf("counter != ITERATIONS (%d != %d)\n", *result, DISPATCH_ITERATIONS);
	}
	
	printf("Method dispatch test took %f seconds.\n", ((double)c2 - (double)c1)/ (double)CLOCKS_PER_SEC);
}
static void super_method_dispatch_test(void){
	MyClass *instance;
	clock_t c1, c2;
	int i;
	int *result;
	Ivar i_ivar;
	
	/** Adds a new method to the subclass. */
	objc_class_replace_instance_method_implementation(my_subclass, log_selector, (IMP)&_MySubclass_log, "^@:");
	
	instance = (MyClass*)alloc_impl((id)my_subclass, alloc_selector);
	
	c1 = clock();
	for (i = 0; i < DISPATCH_ITERATIONS; ++i){
		IMP log_impl = objc_object_lookup_impl((id)instance, log_selector);
		log_impl((id)instance, log_selector);
		/* _MyClass_log(instance, log_selector); */
	}
	
	c2 = clock();
	
	i_ivar = objc_class_get_ivar(my_class, "i");
	result = (int*)objc_object_get_variable((id)instance, i_ivar);
	if (*result != 2 * DISPATCH_ITERATIONS){
		printf("counter != ITERATIONS (%d != %d)\n", *result, 2 * DISPATCH_ITERATIONS);
	}
	
	printf("Super method dispatch test took %f seconds.\n", ((double)c2 - (double)c1)/ (double)CLOCKS_PER_SEC);
}
static void method_dispatch_test_via_ivar_setters(void){
	MyClass *instance;
	clock_t c1, c2;
	int i;
	int *result;
	Ivar i_ivar;
	
	instance = (MyClass*)alloc_impl((id)my_subclass, alloc_selector);
	
	c1 = clock();
	for (i = 0; i < DISPATCH_ITERATIONS; ++i){
		IMP log_impl = objc_object_lookup_impl((id)instance, increase_via_gs_selector);
		log_impl((id)instance, increase_via_gs_selector);
	}
	
	c2 = clock();
	
	i_ivar = objc_class_get_ivar(my_class, "i");
	result = (int*)objc_object_get_variable((id)instance, i_ivar);
	if (*result != DISPATCH_ITERATIONS){
		printf("counter != ITERATIONS (%d != %d)\n", *result, DISPATCH_ITERATIONS);
	}
	
	printf("Method dispatch test via ivar setters took %f seconds.\n", ((double)c2 - (double)c1)/ (double)CLOCKS_PER_SEC);
}
static void method_dispatch_test_via_ao(void){
	MyClass *instance;
	clock_t c1, c2;
	int i;
	int result;
	
	instance = (MyClass*)alloc_impl((id)my_subclass, alloc_selector);
	
	c1 = clock();
	for (i = 0; i < DISPATCH_ITERATIONS; ++i){
		IMP log_impl = objc_object_lookup_impl((id)instance, increase_via_ao_selector);
		log_impl((id)instance, increase_via_ao_selector);
	}
	
	c2 = clock();
	
	result = (unsigned int)objc_object_get_associated_object((id)instance, (void*)increase_via_ao_selector);
	if (result != DISPATCH_ITERATIONS){
		printf("counter != ITERATIONS (%d != %d)\n", result, DISPATCH_ITERATIONS);
	}
	
	printf("Method dispatch test via AO took %f seconds.\n", ((double)c2 - (double)c1)/ (double)CLOCKS_PER_SEC);
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
	method_dispatch_test_via_ivar_setters();
	method_dispatch_test_via_ao();
	object_creation_test();
	super_method_dispatch_test();
	
	return 0;
}

