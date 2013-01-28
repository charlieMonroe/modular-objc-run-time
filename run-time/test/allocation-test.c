#include "testing.h"

static void allocation_test(void){
	clock_t c1, c2;
	int i;
	
	c1 = clock();
	for (i = 0; i < ALLOCATION_ITERATIONS; ++i){
		id instance = objc_class_create_instance(objc_class_for_name("MyClass"));
		objc_object_deallocate(instance);
	}
	c2 = clock();
	
	printf("%06lu\n", (c2 - c1));
}


int main(int argc, const char * argv[]){
	register_classes();
	allocation_test();
	return 0;
}
