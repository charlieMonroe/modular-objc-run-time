
#define OBJC_HAS_CATEGORIES_EXTENSION 1
#include "testing-cocoa.h"


void category_test(void){
	clock_t c1, c2;
	id instance = class_createInstance(objc_getClass("MySubclass"), 0);
	c1 = clock();
	for (int i = 0; i < 10000000; ++i){
		OBJC_MSG_SEND_DEBUG(instance, incrementViaCategoryMethod);
	}
	
	c2 = clock();
	
	printf("%06lu\n", (c2 - c1));
}

int main(int argc, const char *argv[]){
	category_test();
	return 0;
}

