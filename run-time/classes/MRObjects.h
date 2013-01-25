
#ifndef _MRObject_H_
#define _MRObject_H_

#include "../private.h"

typedef struct {
	Class isa;
	int retainCount;
} MRObject_instance_t;

typedef struct {
	Class isa;
	int retainCount;
	const char *cString;
} __MRConstString_instance_t;

extern struct objc_class_prototype MRObject_class;
extern struct objc_class_prototype __MRConstString_class;

#define OBJC_STRING(VAR_NAME, STR) static __MRConstString_instance_t VAR_NAME##_stat_str = { (Class)(&__MRConstString_class), 1, STR };\
							     VAR_NAME = (id)&VAR_NAME##_stat_str;


#endif /** _MRObject_H_ */
