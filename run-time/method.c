//
//  method.c
//  modular_runtime
//
//  Created by Charlie Monroe on 11/27/12.
//  Copyright (c) 2012 Fuel Collective, LLC. All rights reserved.
//

#include "method.h"
#include "method-private.h"
#include "runtime-private.h"
#include "utilities.h"

Method objc_method_create(SEL selector, const char *types, IMP implementation){
	Method m = objc_alloc(sizeof(struct objc_method));
	m->selector = selector;
	m->implementation = implementation;
	m->types = objc_strcpy(types);
	return m;
}

IMP objc_method_get_implementation(Method method){
	return method->implementation;
}
