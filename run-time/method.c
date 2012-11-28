//
//  method.c
//  modular_runtime
//
//  Created by Charlie Monroe on 11/27/12.
//  Copyright (c) 2012 Fuel Collective, LLC. All rights reserved.
//

#include "method.h"
#include "runtime-private.h"
#include "utilities.h"

struct objc_method {
	SEL selector;
	const char *types;
	IMP implementation;
};

Method objc_methodCreate(SEL selector, const char *types, IMP implementation){
	Method m = objc_setup.memory.allocator(sizeof(struct objc_method));
	m->selector = selector;
	m->implementation = implementation;
	m->types = objc_strcpy(types);
	return m;
}
