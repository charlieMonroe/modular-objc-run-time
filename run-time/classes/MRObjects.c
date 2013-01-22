/**
 * A basic object class with R/R methods.
 */

#include "private.h"
#include "types.h"

#include "MRObjectMethods.h"

/*************** MRObject ***************/
/** MRObject class, ivars and methods. */

static struct objc_class MRObject_str = {
	NULL, /** isa pointer gets connected in the init function. */
	NULL, /** Root class */
	"MRObject",
	NULL, /** Class methods */
	NULL, /** Instance methods */
	NULL, /** Ivars */
	NULL, /** Class cache. */
	NULL, /** Instance cache. */
	0, /** Instance size - computed from ivars. */
	{
		YES /** In construction. */
	}
};

static struct objc_method_prototype _C_MRObject_alloc_mp = {
	"alloc",
	"@@:",
	(IMP)&_C_MRObject_alloc_,
	0
};

static struct objc_method_prototype _I_MRObject_retain_mp = {
	"alloc",
	"@@:",
	(IMP)&_C_MRObject_alloc_,
	0
};



static struct objc_method_prototype *MRObject_class_methods[] = {
	&_C_MRObject_alloc_mp
};


static struct objc_class MRString_str = {
	NULL, /** isa pointer gets connected in the init function. */
	&MRObject_str, /** Superclass */
	"MRString",
	NULL, /** Class methods */
	NULL, /** Instance methods */
	NULL, /** Ivars */
	NULL, /** Class cache. */
	NULL, /** Instance cache. */
	0, /** Instance size - computed from ivars. */
	{
		YES /** In construction. */
	}
};



Class MRObject = &MRObject_str;
Class MRString = &MRString_str;


void objc_install_base_classes(void){
	
}
