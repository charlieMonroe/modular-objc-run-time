/**
 * WARNING: This test doesn't return times,
 * but a proxy / normal call ratio.
 */

#include "testing.h"

static void _I_NewClass_unknownSelector(id self, SEL _cmd, int i){
	/* No-op */
}

static void forwarding_test(void){
	id new_class_instance;
	MyClass *my_class_instance;
	clock_t c1, c2;
	double regular_call_time, proxy_call_time;
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
	regular_call_time = ((double)c2 - (double)c1)/ (double)CLOCKS_PER_SEC;
	
	
	my_class_instance = (MyClass*)objc_class_create_instance(objc_class_for_name("MyClass"));
	my_class_instance->proxyObject = new_class_instance;
	
	c1 = clock();
	for (i = 0; i < DISPATCH_ITERATIONS; ++i){
		((void(*)(id,SEL,int))objc_object_lookup_impl((id)my_class_instance, unknown_selector))((id)my_class_instance, unknown_selector, i);
	}
	c2 = clock();
	
	proxy_call_time = ((double)c2 - (double)c1)/ (double)CLOCKS_PER_SEC;
	printf("%f\n", proxy_call_time / regular_call_time);
}

int main(int argc, const char * argv[]){
	register_classes();
	forwarding_test();
	return 0;
}

