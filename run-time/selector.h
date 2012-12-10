//
//  selector.h
//  modular_runtime
//
//  Created by Charlie Monroe on 12/9/12.
//  Copyright (c) 2012 Fuel Collective, LLC. All rights reserved.
//

#ifndef OBJC_SELECTOR_H_
#define OBJC_SELECTOR_H_

#include "types.h"

// The selector name is copied over
extern SEL objc_selector_register(const char *name);

// Pointer and name comparison
extern BOOL objc_selectors_equal(SEL selector1, SEL selector2);

#endif // OBJC_SELECTOR_H_
