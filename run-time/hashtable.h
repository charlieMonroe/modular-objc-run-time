//
//  hashtable.h
//  modular_runtime
//
//  Created by Charlie Monroe on 11/26/12.
//  Copyright (c) 2012 Fuel Collective, LLC. All rights reserved.
//

#ifndef OBJC_HASH_TABLE_H_
#define OBJC_HASH_TABLE_H_

#include "types.h"

typedef struct _objc_hash_table_str *_objc_hash_table;

extern _objc_hash_table objc_hash_table_create(unsigned int capacity, const char*(*keyGetter)(void*), BOOL(*equalityFunction)(void*, void*));
extern _objc_hash_table objc_hash_table_create_lockable(unsigned int capacity, const char*(*keyGetter)(void*), BOOL(*equalityFunction)(void*, void*));
extern void objc_hash_table_destroy(_objc_hash_table table);
extern void objc_hash_table_insert(_objc_hash_table table, void *obj);
extern void *objc_hash_table_get(_objc_hash_table table, const char *key);

extern void objc_hash_table_rlock(_objc_hash_table table);
extern void objc_hash_table_wlock(_objc_hash_table table);
extern void objc_hash_table_unlock(_objc_hash_table table);


#endif // OBJC_HASH_TABLE_H_
