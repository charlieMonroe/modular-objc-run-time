
#ifndef RUNTIME_TEST_CLASSES_H_
#define RUNTIME_TEST_CLASSES_H_

#include "types.h"

typedef struct {
	Class isa;
	id proxyObject;
	int i;
} MyClass;

static id _C_MyClass_alloc_(id self, SEL _cmd, ...){
#if OBJC_HAS_CATEGORIES_EXTENSION
	printf("Shouldn't be here!\n");
	objc_abort("Category should override this method.");
#endif
	return objc_class_create_instance((Class)self);
}

static void _I_MyClass_increment_(MyClass *self, SEL _cmd, ...){
	++self->i;
}

static void _I_MyClass_incrementViaSettersAndGetters_(MyClass *self, SEL _cmd, ...){
	int *old_value_ptr;
	int new_value;
	Ivar i_ivar = objc_class_get_ivar(objc_object_get_class((id)self), "i");
	
	old_value_ptr = objc_object_get_variable((id)self, i_ivar);
	new_value = *old_value_ptr + 1;
	objc_object_set_variable((id)self, i_ivar, &new_value);
}

#if OBJC_HAS_AO_EXTENSION
static void _I_MyClass_incrementViaAO_(MyClass *self, SEL _cmd, ...){
	unsigned int old_value = (unsigned int)objc_object_get_associated_object((id)self, (void*)_cmd);
	objc_object_set_associated_object((id)self, (void*)_cmd, (id)(old_value + 1));
}
#endif

static Method _I_MyClass_forwardedMethod_(MyClass *self, SEL _cmd, SEL selector){
	return objc_object_lookup_method(self->proxyObject, selector);
}

static BOOL _I_MyClass_dropMessageForSelector_(id self, SEL _cmd, SEL selector){
	printf("Class %s supports message dropping - dropped call with selector %s.\n", objc_class_get_name(self->isa), objc_selector_get_name(selector));
	return YES;
}

static struct objc_method_prototype _C_MyClass_alloc_mp_ = {
	"alloc",
	"@@:",
	(IMP)_C_MyClass_alloc_,
	0
};

static struct objc_method_prototype _I_MyClass_increment_mp_ = {
	"increment",
	"v@:",
	(IMP)_I_MyClass_increment_,
	0
};

static struct objc_method_prototype _I_MyClass_incrementViaSettersAndGetters_mp_ = {
	"incrementViaSettersAndGetters",
	"v@:",
	(IMP)_I_MyClass_incrementViaSettersAndGetters_,
	0
};

#if OBJC_HAS_AO_EXTENSION
static struct objc_method_prototype _I_MyClass_incrementViaAO_mp_ = {
	"incrementViaAO",
	"v@:",
	(IMP)_I_MyClass_incrementViaAO_,
	0
};
#endif

static struct objc_method_prototype _I_MyClass_forwardedMethod_mp_ = {
	"forwardedMethod:",
	"^@::",
	(IMP)_I_MyClass_forwardedMethod_,
	0
};

static struct objc_method_prototype _I_MyClass_dropMessageForSelector_mp_ = {
	"dropMessageForSelector:",
	"B@::",
	(IMP)_I_MyClass_dropMessageForSelector_,
	0
};

static struct objc_ivar _MyClass_isa_proxyObject_ivar_ = {
	"proxyObject",
	"@",
	sizeof(id),
	sizeof(id)
};

static struct objc_ivar _MyClass_isa_i_ivar_ = {
	"i",
	"i",
	sizeof(int),
	sizeof(id) + sizeof(id)
};


static struct objc_method_prototype *_MyClass_class_methods[] = {
	&_C_MyClass_alloc_mp_,
	NULL
};

static struct objc_method_prototype *_MyClass_instance_methods[] = {
	&_I_MyClass_increment_mp_,
	&_I_MyClass_incrementViaSettersAndGetters_mp_,
#if OBJC_HAS_AO_EXTENSION
	&_I_MyClass_incrementViaAO_mp_,
#endif
	&_I_MyClass_forwardedMethod_mp_,
	&_I_MyClass_dropMessageForSelector_mp_,
	NULL
};

static Ivar _MyClass_ivars_[] = {
	&_MyClass_isa_proxyObject_ivar_,
	&_MyClass_isa_i_ivar_
};

static struct objc_class_prototype MyClass_class = {
	NULL, /** isa pointer gets connected when registering. */
	"MRObject", /** Superclass */
	"MyClass",
	_MyClass_class_methods, /** Class methods */
	_MyClass_instance_methods, /** Instance methods */
	_MyClass_ivars_, /** Ivars */
	NULL, /** Class cache. */
	NULL, /** Instance cache. */
	0, /** Instance size - computed from ivars. */
	0, /** Version. */
	{
		YES /** In construction. */
	},
	NULL /** Extra space. */
};

static void _I_MySubclass_increment_(MyClass *self, SEL _cmd, ...){
	/* i.e. [super _cmd]; */
	objc_super super;
	IMP super_imp;
	super.receiver = (id)self;
	super.class = objc_class_get_superclass(self->isa);
	super_imp = objc_object_lookup_impl_super(&super, _cmd);
	super_imp((id)self, _cmd);
	
	++self->i;
}

static struct objc_method_prototype _I_MySubclass_increment_mp_ = {
	"increment",
	"v@:",
	(IMP)_I_MySubclass_increment_,
	0
};

static struct objc_class_prototype MySubclass_class = {
	NULL, /** isa pointer gets connected when registering. */
	"MyClass", /** Superclass */
	"MySubclass",
	NULL, /** Class methods */
	NULL, /** Instance methods */
	NULL, /** Ivars */
	NULL, /** Class cache. */
	NULL, /** Instance cache. */
	0, /** Instance size - computed from ivars. */
	0, /** Version. */
	{
		YES /** In construction. */
	},
	NULL /** Extra space. */
};

#if OBJC_HAS_CATEGORIES_EXTENSION

static id _C_MyClassCategoryPrivate_alloc_(id self, SEL _cmd){
	static BOOL firstCallPassed;
	if (!firstCallPassed){
		printf("Allocating via class category!\n");
		firstCallPassed = YES;
	}
	return objc_class_create_instance((Class)self);
}

static struct objc_method_prototype _C_MyClassCategoryPrivate_alloc_mp_ = {
	"alloc",
	"@@:",
	(IMP)_C_MyClassCategoryPrivate_alloc_,
	0
};

static struct objc_method_prototype *MyClass_Private_category_class_methods[] = {
	&_C_MyClassCategoryPrivate_alloc_mp_,
	NULL
};

static struct objc_category_prototype _MyClass_Privates_category_prototype_ = {
	"MyClass",
	"Privates",
	MyClass_Private_category_class_methods,
	NULL
};

#endif


#endif /* RUNTIME_TEST_CLASSES_H_ */
