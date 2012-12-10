//
//  method.h
//  modular_runtime
//
//  Created by Charlie Monroe on 11/27/12.
//  Copyright (c) 2012 Fuel Collective, LLC. All rights reserved.
//

#ifndef OBJC_METHOD_H_
#define OBJC_METHOD_H_

#include "types.h"

extern Method objc_method_create(SEL selector, const char *types, IMP implementation);
extern IMP objc_method_get_implementation(Method method);

#endif //OBJC_METHOD_H_
