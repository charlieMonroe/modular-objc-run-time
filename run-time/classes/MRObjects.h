
#ifndef _MRObject_H_
#define _MRObject_H_

#include "private.h"

typedef struct {
	Class isa;
	unsigned int retain_count;
} MRObject_instance_t;

typedef struct {
	Class isa;
	unsigned int retain_count;
	const char *string;
} MRString_instance_t;

extern struct objc_class_prototype MRObject_class;
extern struct objc_class_prototype MRString_class;

#define OBJC_STRING(VAR_NAME, STR) static MRString_instance_t VAR_NAME##_stat_str = { (Class)(&MRString_class), 1, STR };\
							     VAR_NAME = (id)&VAR_NAME##_stat_str;


#endif /** _MRObject_H_ */
