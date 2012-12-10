/*
  * Default objc_array implementation.
  */
 
 
 
 
#ifndef OBJC_ARRAY_H_
#define OBJC_ARRAY_H_

#include "types.h"

#define OBJC_ARRAY_NOT_FOUND ((unsigned int)(-1))

// Creates a new objc_array object
extern objc_array objc_array_create(unsigned int capacity);

// Destroys (i.e. frees the objc_array object). No action is
// performed on the pointers. It's up to the array owner
// to destroy them.
extern void objc_array_destroy(objc_array array);

// Add pointer to the array.
extern void objc_array_add(objc_array array, void *ptr);

// Returns YES if the array contains that specific pointer
extern BOOL objc_array_contains(objc_array array, void *ptr);

// Returns index of the ptr or OBJC_ARRAY_NOT_FOUND
extern unsigned int objc_array_index_of_pointer(objc_array array, void *ptr);

// Returns pointer at index. If index is out of range, aborts
extern void *objc_array_pointer_at_index(objc_array array, unsigned int index);

// Removes pointer from array. Does nothing if ptr isn't in the array
extern void objc_array_remove(objc_array array, void *ptr);

// Removes pointer at index
extern void objc_array_remove_at_index(objc_array array, unsigned int index);

// Size of the array (i.e. number of pointers stored)
extern unsigned int objc_array_size(objc_array array);


// LOCKABLE

// Creates a new objc_array with the ability to lock itself using a mutex
extern objc_array objc_array_create_lockable(unsigned int capacity);

// Locks the mutex associated with that particular array
extern void objc_array_lock_for_reading(objc_array array);
extern void objc_array_lock_for_writing(objc_array array);

// Unlocks the mutex associated with that particular array
extern void objc_array_unlock(objc_array array);

#endif // OBJC_ARRAY_H_
