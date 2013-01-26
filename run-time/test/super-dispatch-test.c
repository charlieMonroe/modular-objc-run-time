#include "testing.h"

GENERATE_TEST(super_dispatch, "MySubclass", {
	Method m = (Method)&_I_MySubclass_increment_mp_;
	m->selector = objc_selector_register(_I_MySubclass_increment_mp_.selector_name);
	objc_class_add_instance_method(objc_class_for_name("MySubclass"), m);
}, DISPATCH_ITERATIONS, {
	SEL selector = objc_selector_register("increment");
	IMP impl = objc_object_lookup_impl((id)instance, selector);
	impl((id)instance, selector);
}, (*((int*)(objc_object_get_variable((id)instance, objc_class_get_ivar(objc_class_for_name("MySubclass"), "i")))) == 2 * DISPATCH_ITERATIONS))

int main(int argc, const char * argv[]){
	register_classes();
	super_dispatch_test();
	return 0;
}
