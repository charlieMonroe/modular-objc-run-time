#include "testing.h"

GENERATE_TEST(dispatch, "MySubclass", {}, DISPATCH_ITERATIONS, {
	SEL selector = objc_selector_register("increment");
	IMP impl = objc_object_lookup_impl((id)instance, selector);
	impl((id)instance, selector);
}, (*((int*)(objc_object_get_variable((id)instance, objc_class_get_ivar(objc_class_for_name("MySubclass"), "i")))) == DISPATCH_ITERATIONS))

int main(int argc, const char * argv[]){
	register_classes();
	dispatch_test();
	return 0;
}
