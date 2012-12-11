//
//  selector_holder-inline.h
//  modular_runtime
//
//  Created by Charlie Monroe on 12/11/12.
//  Copyright (c) 2012 Fuel Collective, LLC. All rights reserved.
//

#ifndef modular_runtime_selector_holder_inline_h
#define modular_runtime_selector_holder_inline_h

#include "hashtable-inline.h"
#include "../selector.h"

static const char *_objc_selector_holder_key_getter(void *selector){
	return ((SEL)selector)->name;
}

// Just passing all the functions to a hash table
OBJC_INLINE objc_selector_holder objc_selector_holder_create(void){
	// There are likely to be many classes. Start with 64 buckets
	_objc_hash_table table = objc_hash_table_create_lockable(64, _objc_selector_holder_key_getter, (BOOL(*)(void*,void*))objc_selectors_equal);
	return (objc_class_holder)table;
}
OBJC_INLINE void objc_selector_holder_insert(objc_selector_holder holder, SEL selector){
	objc_hash_table_insert((_objc_hash_table)holder, (void*)selector);
}
OBJC_INLINE SEL objc_selector_holder_lookup(objc_selector_holder holder, const char *name){
	return (SEL)objc_hash_table_get((_objc_hash_table)holder, name);
}
OBJC_INLINE void objc_selector_holder_rlock(objc_selector_holder holder){
	objc_hash_table_rlock((_objc_hash_table)holder);
}
OBJC_INLINE void objc_selector_holder_wlock(objc_selector_holder holder){
	objc_hash_table_wlock((_objc_hash_table)holder);
}
OBJC_INLINE void objc_selector_holder_unlock(objc_selector_holder holder){
	objc_hash_table_unlock((_objc_hash_table)holder);
}


#endif
