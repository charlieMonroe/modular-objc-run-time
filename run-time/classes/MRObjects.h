
#ifndef _MRObject_H_
#define _MRObject_H_

#include "private.h"

typedef struct {
	Class isa;
	unsigned int retain_count;
} MRObject_instance_t;

#endif /** _MRObject_H_ */
