
#define OBJC_HAS_AO_EXTENSION 1

#include "testing.h"

GENERATE_TEST(ao, "MySubclass", {}, DISPATCH_ITERATIONS, {
	SEL selector = objc_selector_register("incrementViaAO");
	IMP impl = objc_object_lookup_impl((id)instance, selector);
	impl((id)instance, selector);
}, (*((int*)(objc_object_get_variable((id)instance, objc_class_get_ivar(objc_class_for_name("MySubclass"), "i")))) == DISPATCH_ITERATIONS))

int main(int argc, const char * argv[]){
	register_classes();
	ao_test();
	return 0;
}
