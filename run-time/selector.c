//
//  selector.c
//  modular_runtime
//
//  Created by Charlie Monroe on 12/9/12.
//  Copyright (c) 2012 Fuel Collective, LLC. All rights reserved.
//

#include "selector.h"
#include "os.h" // For run-time functions
#include "utilities.h" // For strcpy

static objc_selector_holder selector_cache;

static inline SEL _objc_selector_insert_into_cache_no_lock(const char* name){
	SEL selector = objc_alloc(sizeof(struct objc_selector));
	selector->name = objc_strcpy(name);
	objc_selector_holder_insert(selector_cache, selector);
	return selector;
}

SEL objc_selector_register(const char *name){
	objc_selector_holder_rlock(selector_cache);
	SEL selector = objc_selector_holder_lookup(selector_cache, name);
	objc_selector_holder_unlock(selector_cache);
	
	if (selector == NULL){
		objc_selector_holder_wlock(selector_cache);
		
		// Check if the selector hasn't been added yet
		selector = (SEL)objc_selector_holder_lookup(selector_cache, name);
		if (selector == NULL){
			// Still nothing, insert
			selector = _objc_selector_insert_into_cache_no_lock(name);
		}
	}
	return selector;
}

BOOL objc_selectors_equal(SEL selector1, SEL selector2){
	if (selector1 == NULL && selector2 == NULL){
		return YES;
	}
	if (selector1 == NULL || selector2 == NULL){
		return NO;
	}
	
	return selector1 == selector2 || objc_strings_equal(selector1->name, selector2->name);
}

void objc_selector_init(void){
	selector_cache = objc_selector_holder_create();
}

