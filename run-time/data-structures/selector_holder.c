//
//  selector_holder.c
//  modular_runtime
//
//  Created by Charlie Monroe on 12/9/12.
//  Copyright (c) 2012 Fuel Collective, LLC. All rights reserved.
//

#include "selector_holder.h"
#include "hashtable.h"
#include "selector.h"



static const char *_objc_selector_holder_key_getter(void *selector){
	return ((SEL)selector)->name;
}

// Just passing all the functions to a hash table
objc_selector_holder _objc_selector_holder_create(void){
	// There are likely to be many classes. Start with 64 buckets
	_objc_hash_table table = objc_hash_table_create_lockable(64, _objc_selector_holder_key_getter, (BOOL(*)(void*,void*))objc_selectors_equal);
	return (objc_class_holder)table;
}
void _objc_selector_holder_insert_selector(objc_selector_holder holder, SEL selector){
	objc_hash_table_insert((_objc_hash_table)holder, (void*)selector);
}
SEL _objc_selector_holder_lookup_selector(objc_selector_holder holder, const char *name){
	return (SEL)objc_hash_table_get((_objc_hash_table)holder, name);
}
void _objc_selector_holder_rlock(objc_selector_holder holder){
	objc_hash_table_rlock((_objc_hash_table)holder);
}
void _objc_selector_holder_wlock(objc_selector_holder holder){
	objc_hash_table_wlock((_objc_hash_table)holder);
}
void _objc_selector_holder_unlock(objc_selector_holder holder){
	objc_hash_table_unlock((_objc_hash_table)holder);
}


