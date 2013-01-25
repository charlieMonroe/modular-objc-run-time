#include <stdio.h>
#include <time.h>

#include "runtime.h"
#include "class.h"
#include "method.h"
#include "selector.h"
#include "classes/MRObjects.h"

#include "extras/ao-ext.h"
#include "extras/categs.h"

#define DISPATCH_ITERATIONS 10000000
#define ALLOCATION_ITERATIONS 10000000

#define OBJC_HAS_AO_EXTENSION 0
#define OBJC_HAS_CATEGORIES_EXTENSION 0

#include "testing.h"

static void register_classes(void){
	objc_class_register_prototype(&MyClass_class);
	objc_class_register_prototype(&MySubclass_class);
	
	
#if OBJC_HAS_CATEGORIES_EXTENSION
	objc_class_register_category_prototype(&_MyClass_Privates_category_prototype_);
#endif
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

#if OBJC_HAS_CATEGORIES_EXTENSION
static void print_categories(Class cl){
	Category *categories = objc_class_get_category_list(cl);
	Category *orig_ptr = categories;
	while (*categories != NULL){
		Method *class_methods = objc_category_get_class_methods(*categories);
		Method *instance_methods = objc_category_get_instance_methods(*categories);
		
		printf("** %s - Class category methods:\n", objc_category_get_name(*categories));
		print_method_list(class_methods);
		
		printf("** %s - Instance category methods:\n", objc_category_get_name(*categories));
		print_method_list(instance_methods);
		
		objc_dealloc(class_methods);
		objc_dealloc(instance_methods);
		
		++categories;
	}
	objc_dealloc(orig_ptr);
}
#endif

static void print_class(Class cl){
	printf("******** Class %s ********\n", objc_class_get_name(cl));
	printf("**** Class methods:\n");
	print_method_list(objc_class_get_class_method_list(cl));
	printf("**** Instance methods:\n");
	print_method_list(objc_class_get_instance_method_list(cl));
	
#if OBJC_HAS_CATEGORIES_EXTENSION
	printf("**** Categories:\n");
	print_categories(cl);
#endif
	
	printf("**** Ivars:\n");
	print_ivar_list(objc_class_get_ivar_list(cl));
	
	printf("\n\n");
}
static void list_classes(void){
	Class *classes = objc_class_get_list();
	Class *orig_ptr = classes;
	while (*classes != NULL){
		print_class(*classes);
		++classes;
	}
	objc_dealloc(orig_ptr);
}

static void method_dispatch_test(void){
	MyClass *instance;
	clock_t c1, c2;
	int i;
	int *result;
	Ivar i_ivar;
	
	instance = (MyClass*)objc_object_lookup_impl((id)my_subclass, alloc_selector)((id)my_subclass, alloc_selector);
	
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
	
	instance = (MyClass*)objc_object_lookup_impl((id)my_subclass, alloc_selector)((id)my_subclass, alloc_selector);
	
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
	
	instance = (MyClass*)objc_object_lookup_impl((id)my_subclass, alloc_selector)((id)my_subclass, alloc_selector);
	
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
	
	instance = (MyClass*)objc_object_lookup_impl((id)my_subclass, alloc_selector)((id)my_subclass, alloc_selector);
	
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
		id instance = objc_class_create_instance(my_class);
		objc_object_deallocate(instance);
	}
	c2 = clock();
		
	printf("Object allocation test took %f seconds.\n", ((double)c2 - (double)c1)/ (double)CLOCKS_PER_SEC);
}

static void _I_NewClass_unknownSelector(id self, SEL _cmd, int i){
	/* No-op */
}

static void forwarding_test(void){
	id new_class_instance;
	MyClass *my_class_instance;
	clock_t c1, c2;
	int i;
	
	Class completely_new_class = objc_class_create(Nil, "NewClass");
	SEL unknown_selector = objc_selector_register("unknownSelector:");
	objc_class_add_ivar(completely_new_class, "isa", sizeof(Class), __alignof(Class), "#");
	objc_class_add_instance_method(completely_new_class, objc_method_create(unknown_selector, "v@:i", (IMP)_I_NewClass_unknownSelector));
	objc_class_finish(completely_new_class);
	
	
	new_class_instance = objc_class_create_instance(completely_new_class);
	c1 = clock();
	for (i = 0; i < DISPATCH_ITERATIONS; ++i){
		((void(*)(id,SEL,int))objc_object_lookup_impl(new_class_instance, unknown_selector))(new_class_instance, unknown_selector, i);
	}
	c2 = clock();
	printf("Non-proxy calls took %f seconds.\n", ((double)c2 - (double)c1)/ (double)CLOCKS_PER_SEC);
	
	
	my_class_instance = (MyClass*)objc_class_create_instance(my_class);
	my_class_instance->proxy_object = new_class_instance;
	
	c1 = clock();
	for (i = 0; i < DISPATCH_ITERATIONS; ++i){
		((void(*)(id,SEL,int))objc_object_lookup_impl((id)my_class_instance, unknown_selector))((id)my_class_instance, unknown_selector, i);
	}
	c2 = clock();
	printf("Proxy calls took %f seconds.\n", ((double)c2 - (double)c1)/ (double)CLOCKS_PER_SEC);	
	
}

int main(int argc, const char * argv[]){
	create_classes();
	//list_classes();
	
	method_dispatch_test();
	//method_dispatch_test_via_ivar_setters();
	//method_dispatch_test_via_ao();
	object_creation_test();
	//super_method_dispatch_test();
	forwarding_test();
	
	return 0;
}

