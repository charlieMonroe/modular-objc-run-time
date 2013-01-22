
#ifndef _MRObjectMethods_H_
#define _MRObjectMethods_H_

#include "types.h"
#include "MRObjects.h"

extern id _C_MRObject_alloc_(id self, SEL _cmd);

extern id _I_MRObject_retain_(MRObject_instance_t *self, SEL _cmd);

#endif /** _MRObjectMethods_H_ */
