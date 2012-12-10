//
//  method-private.h
//  modular_runtime
//
//  Created by Charlie Monroe on 12/9/12.
//  Copyright (c) 2012 Fuel Collective, LLC. All rights reserved.
//

#ifndef OBJC_METHOD_PRIVATE_H_
#define OBJC_METHOD_PRIVATE_H_

struct objc_method {
	SEL selector;
	const char *types;
	IMP implementation;
};

#endif // OBJC_METHOD_PRIVATE_H_
