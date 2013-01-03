#include "method.h"
#include "os.h" /* For objc_alloc. */
#include "utilities.h" /* For objc_strcpy */

Method objc_method_create(SEL selector, const char *types, IMP implementation){
	Method m = objc_alloc(sizeof(struct objc_method));
	m->selector = selector;
	m->implementation = implementation;
	m->types = objc_strcpy(types);
	return m;
}

IMP objc_method_get_implementation(Method method){
	return method->implementation;
}
