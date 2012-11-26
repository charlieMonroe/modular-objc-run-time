//
//  class_holder.c
//  modular_runtime
//
//  Created by Charlie Monroe on 11/25/12.
//  Copyright (c) 2012 Fuel Collective, LLC. All rights reserved.
//

#include "class_holder.h"
#include "class-private.h"
#include "hashtable.h"

static const char *_objc_class_holder_key_getter(void *cl){
	return ((Class)cl)->name;
}

static BOOL _objc_class_equality_function(void *cl1, void *cl2){
	// The equality among classes is just pointer-wise as we don't allow
	// two classes with the same name
	return cl1 == cl2;
}

// Just passing all the functions to a hash table
objc_class_holder objc_class_holder_create(void){
	// There are likely to be many classes. Start with 64 buckets
	_objc_hash_table table = objc_hash_table_create(64, _objc_class_holder_key_getter, _objc_class_equality_function);
	return (objc_class_holder)table;
}
void objc_class_holder_destroy(objc_class_holder holder){
	objc_hash_table_destroy((_objc_hash_table)holder);
}
void objc_class_holder_insert_class(objc_class_holder holder, Class cl){
	objc_hash_table_insert((_objc_hash_table)holder, (void*)cl);
}
Class objc_class_holder_lookup_class(objc_class_holder holder, const char *name){
	return (Class)objc_hash_table_get((_objc_hash_table)holder, name);
}