
#include "MRObjectMethods.h"

#include "os.h"
#include "class.h"
#include "selector.h"

#pragma mark MRObject

id _C_MRObject_alloc_(id self, SEL _cmd){
	MRObject_instance_t *instance = (MRObject_instance_t*)objc_class_create_instance((Class)self, 0);
	instance->retain_count = 1;
	return (id)instance;
}

id _C_MRObject_new_(id self, SEL _cmd){
	static SEL alloc_SEL;
	static SEL init_SEL;
	
	if (alloc_SEL == NULL){
		alloc_SEL = objc_selector_register("alloc");
	}
	
	if (init_SEL == NULL){
		init_SEL = objc_selector_register("init");
	}
	
	self = ((id(*)(id, SEL))objc_object_lookup_impl(self, alloc_SEL))(self, alloc_SEL);
	self = ((id(*)(id, SEL))objc_object_lookup_impl(self, init_SEL))(self, init_SEL);
	
	return self;
}

id _I_MRObject_init_(MRObject_instance_t *self, SEL _cmd){
	/** Retain count is set to 1 in +alloc. */
	return (id)self;
}

id _I_MRObject_retain_(MRObject_instance_t *self, SEL _cmd){
	/** TODO atomic */
	++self->retain_count;
	return (id)self;
}

void _I_MRObject_release_(MRObject_instance_t *self, SEL _cmd){
	/** TODO atomic */
	--self->retain_count;
	if (self->retain_count == 0){
		/** Dealloc */
		static SEL dealloc_selector;
		IMP dealloc_IMP;
		if (dealloc_selector == NULL){
			dealloc_selector = objc_selector_register("dealloc");
		}
		
		dealloc_IMP = objc_object_lookup_impl((id)self, dealloc_selector);
		if (dealloc_IMP == NULL){
			objc_log("%s doesn't implement the dealloc method.", objc_class_get_name(self->isa));
			objc_abort("Class doesn't implement the dealloc method.");
		}else{
			dealloc_IMP((id)self, dealloc_selector);
		}
	}
}

id _C_MRObject_retain_noop_(Class self, SEL _cmd){
	return (id)self;
}

void _C_MRObject_release_noop_(Class self, SEL _cmd){
	/** No-op. */
}

void _I_MRObject_dealloc_(MRObject_instance_t *self, SEL _cmd){
	objc_object_deallocate((id)self);
}

/** Forwarding "support". The default implementation simply returns NULL and NO. */

/** Same IMP for I and C. */
Method _IC_MRObject_forwardedMethodForSelector_(MRObject_instance_t *self, SEL _cmd, SEL selector){
	return NULL;
}

BOOL _IC_MRObject_dropsUnrecognizedMessage_(MRObject_instance_t *self, SEL _cmd, SEL selector){
	return NO;
}

#pragma mark -
#pragma mark __MRConstString

const char *_I___MRConstString_cString_(__MRConstString_instance_t *self, SEL _cmd){
	return self->c_string;
}
