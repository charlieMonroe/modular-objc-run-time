
#include "MRObjectMethods.h"

#include "os.h"
#include "class.h"
#include "selector.h"

id _C_MRObject_alloc_(id self, SEL _cmd){
	return objc_class_create_instance((Class)self, 0);
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


