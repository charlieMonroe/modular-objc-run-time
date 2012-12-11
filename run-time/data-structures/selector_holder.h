//
//  selector_holder.h
//  modular_runtime
//
//  Created by Charlie Monroe on 12/9/12.
//  Copyright (c) 2012 Fuel Collective, LLC. All rights reserved.
//

#ifndef OBJC_SELECTOR_HOLDER_H_
#define OBJC_SELECTOR_HOLDER_H_

#include "os.h"
#if !OBJC_USES_INLINE_FUNCTIONS

#include "types.h"

/**
 * Functions compatible with declares in function-types.h, section objc_selector_holder.
 */
extern objc_selector_holder _objc_selector_holder_create(void);
extern void _objc_selector_holder_insert_selector(objc_selector_holder holder, SEL selector);
extern SEL _objc_selector_holder_lookup_selector(objc_selector_holder holder, const char *name);
extern void _objc_selector_holder_rlock(objc_selector_holder holder);
extern void _objc_selector_holder_wlock(objc_selector_holder holder);
extern void _objc_selector_holder_unlock(objc_selector_holder holder);


#endif // OBJC_USES_INLINE_FUNCTIONS
#endif // OBJC_SELECTOR_HOLDER_H_
