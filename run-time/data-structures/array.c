

#include "array.h"
#include "os.h"

// Default capacity for the array
static const unsigned int _objc_array_default_capacity = 4;

// Internal representation of objc_array
typedef struct _objc_array {
	unsigned int _capacity;
	unsigned int _currentIndex; // Pointer to the last item in array
	objc_rw_lock _lock;
	void **_array;
} *_objc_array;

// Grows the array dynamically
static inline void _objc_array_grow(_objc_array arr){
	// Don't be too agressive growing - adding half of the original capacity
	// is quite enough
	arr->_capacity += (arr->_capacity) / 2;
	arr->_array = objc_realloc(arr->_array, sizeof(void*) * arr->_capacity);
	
	// Realloc doesn't zero the memory, need to do it manually
	for (int i = arr->_currentIndex + 1; i < arr->_capacity; ++i){
		arr->_array[i] = NULL;
	}
}


objc_array objc_array_create(unsigned int capacity){
	_objc_array arr = objc_alloc(sizeof(struct _objc_array));
	arr->_capacity = capacity == 0 ? _objc_array_default_capacity : capacity;
	arr->_currentIndex = -1;
	arr->_lock = NULL;
	arr->_array = objc_zero_alloc(sizeof(void*) * arr->_capacity);
	
	return arr;
}

objc_array objc_array_create_lockable(unsigned int capacity){
	_objc_array arr = objc_array_create(capacity);
	arr->_lock = objc_rw_lock_create();
	return (objc_array)arr;
}

void objc_array_lock_for_reading(objc_array array){
	objc_rw_lock_rlock(((_objc_array)array)->_lock);
}
void objc_array_lock_for_writing(objc_array array){
	objc_rw_lock_wlock(((_objc_array)array)->_lock);
}

void objc_array_unlock(objc_array array){
	objc_rw_lock_unlock(((_objc_array)array)->_lock);
}

void objc_array_destroy(objc_array array){
	if (array == NULL){
		return;
	}
	_objc_array arr = (_objc_array)array;
	objc_dealloc(arr->_array);
	if (arr->_lock != NULL){
		objc_rw_lock_destroy(arr->_lock);
	}
	objc_dealloc(array);
}

void objc_array_add(objc_array array, void *ptr){
	_objc_array arr = (_objc_array)array;
	if (arr == NULL){
		return;
	}
	
	arr->_currentIndex++;
	if (arr->_currentIndex >= arr->_capacity){
		_objc_array_grow(arr);
	}
	
	arr->_array[arr->_currentIndex] = ptr;
}

BOOL objc_array_contains(objc_array array, void *ptr){
	return (BOOL)(objc_array_index_of_pointer(array, ptr) != OBJC_ARRAY_NOT_FOUND);
}

unsigned int objc_array_index_of_pointer(objc_array array, void *ptr){
	_objc_array arr = (_objc_array)array;
	
	unsigned int index = 0;
	unsigned int size = objc_array_size(array);
	for (index = 0; index < size; ++index){
		if (arr->_array[index] == ptr){
			break;
		}
	}
	
	if (index == size){
		// Not found
		return OBJC_ARRAY_NOT_FOUND;
	}
	return index;
}

void *objc_array_pointer_at_index(objc_array array, unsigned int index){
	_objc_array arr = (_objc_array)array;
	if (index > arr->_currentIndex){
		objc_log("%s - Index out of range (%i / %i)", __FUNCTION__, index, objc_array_size(array));
		objc_abort("Index out of bounds.");
	}
	
	return arr->_array[index];
}

void objc_array_remove(objc_array array, void *ptr){
	unsigned int index = objc_array_index_of_pointer(array, ptr);
	if (index == OBJC_ARRAY_NOT_FOUND){
		// Not found
		return;
	}
	
	objc_array_remove_at_index(array, index);
}

void objc_array_remove_at_index(objc_array array, unsigned int index){
	_objc_array arr = (_objc_array)array;
	unsigned int size = objc_array_size(array);
	if (index >= size){
		objc_log("%s - Index out of range (%i / %i)", __FUNCTION__, index, size);
		objc_abort("Index out of bounds.");
	}
	
	if (index == size - 1){
		// Easy case, it's the last item, just wipe it and move the pointer
		arr->_array[index] = NULL;
		--arr->_currentIndex;
		return;
	}
	
	// Otherwise - shift everything after
	int i;
	for (i = index; i < size - 1; ++i){
		arr->_array[i] = arr->_array[i + 1];
	}
	arr->_array[i] = NULL;
	--arr->_currentIndex;
}

unsigned int objc_array_size(objc_array array){
	return ((_objc_array)array)->_currentIndex + 1;
}

