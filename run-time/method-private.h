
#ifndef OBJC_METHOD_PRIVATE_H_
#define OBJC_METHOD_PRIVATE_H_

struct objc_method {
	SEL selector;
	const char *types;
	IMP implementation;
};

#endif /* OBJC_METHOD_PRIVATE_H_ */
