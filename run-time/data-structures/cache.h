//
//  cache.h
//  modular_runtime
//
//  Created by Charlie Monroe on 12/9/12.
//  Copyright (c) 2012 Fuel Collective, LLC. All rights reserved.
//

#ifndef cache_H_
#define cache_H_

#include "os.h"
#if !OBJC_USES_INLINE_FUNCTIONS

#include "types.h"
extern objc_cache cache_create(void);
extern Method cache_fetch(objc_cache cache, SEL selector);
extern void cache_insert(objc_cache cache, Method method);

#endif // OBJC_USES_INLINE_FUNCTIONS

#endif // cache_H_
