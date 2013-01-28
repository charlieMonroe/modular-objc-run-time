
#include "testing-cocoa.h"


void allocation_test(void){
	clock_t c1, c2;
	c1 = clock();
	for (int i = 0; i < 10000000; ++i){
		id instance = class_createInstance(objc_getClass("MyClass"), 0);
		object_dispose(instance);
	}
	
	c2 = clock();
	
	printf("%06lu\n", (c2 - c1));
}

int main(int argc, const char *argv[]){
	allocation_test();
	return 0;
}
