//
//  cache.h
//  modular_runtime
//
//  Created by Charlie Monroe on 12/9/12.
//  Copyright (c) 2012 Fuel Collective, LLC. All rights reserved.
//

#ifndef OBJC_CACHE_H_
#define OBJC_CACHE_H_

#include "types.h"

extern objc_cache objc_cache_create(void);
extern Method objc_cache_fetch(objc_cache cache, SEL selector);
extern void objc_cache_insert(objc_cache cache, Method method);


#endif // OBJC_CACHE_H_
