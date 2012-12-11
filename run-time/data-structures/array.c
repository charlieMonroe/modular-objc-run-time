

#include "array.h"
#include "os.h"

#if !OBJC_USES_INLINE_FUNCTIONS

// Default capacity for the array
static const unsigned int _array_default_capacity = 4;

// Internal representation of objc_array
typedef struct _array {
	unsigned int _capacity;
	unsigned int _currentIndex; // Pointer to the last item in array
	objc_rw_lock _lock;
	void **_array;
} *_array;

// Grows the array dynamically
OBJC_INLINE void _array_grow(_array arr){
	int i;
	
	// Don't be too agressive growing - adding half of the original capacity
	// is quite enough
	arr->_capacity += (arr->_capacity) / 2;
	arr->_array = objc_realloc(arr->_array, sizeof(void*) * arr->_capacity);
	
	// Realloc doesn't zero the memory, need to do it manually
	for (i = arr->_currentIndex + 1; i < arr->_capacity; ++i){
		arr->_array[i] = NULL;
	}
}


objc_array array_create(unsigned int capacity){
	_array arr = objc_alloc(sizeof(struct _array));
	arr->_capacity = capacity == 0 ? _array_default_capacity : capacity;
	arr->_currentIndex = -1;
	arr->_lock = NULL;
	arr->_array = objc_zero_alloc(sizeof(void*) * arr->_capacity);
	
	return arr;
}

objc_array array_create_lockable(unsigned int capacity){
	_array arr = objc_array_create(capacity);
	arr->_lock = objc_rw_lock_create();
	return (objc_array)arr;
}

void array_lock_for_reading(objc_array array){
	objc_rw_lock_rlock(((_array)array)->_lock);
}
void array_lock_for_writing(objc_array array){
	objc_rw_lock_wlock(((_array)array)->_lock);
}

void array_unlock(objc_array array){
	objc_rw_lock_unlock(((_array)array)->_lock);
}

void array_destroy(objc_array array){
	_array arr = (_array)array;
	if (arr == NULL){
		return;
	}
	
	objc_dealloc(arr->_array);
	if (arr->_lock != NULL){
		objc_rw_lock_destroy(arr->_lock);
	}
	objc_dealloc(array);
}

void array_add(objc_array array, void *ptr){
	_array arr = (_array)array;
	if (arr == NULL){
		return;
	}
	
	arr->_currentIndex++;
	if (arr->_currentIndex >= arr->_capacity){
		_array_grow(arr);
	}
	
	arr->_array[arr->_currentIndex] = ptr;
}

BOOL array_contains(objc_array array, void *ptr){
	return (BOOL)(array_index_of_pointer(array, ptr) != OBJC_ARRAY_NOT_FOUND);
}

unsigned int array_index_of_pointer(objc_array array, void *ptr){
	_array arr = (_array)array;
	
	unsigned int index = 0;
	unsigned int size = array_size(array);
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

void *array_pointer_at_index(objc_array array, unsigned int index){
	_array arr = (_array)array;
	if (index > arr->_currentIndex){
		objc_log("%s - Index out of range (%i / %i)", __FUNCTION__, index, array_size(array));
		objc_abort("Index out of bounds.");
	}
	
	return arr->_array[index];
}

void array_remove(objc_array array, void *ptr){
	unsigned int index = array_index_of_pointer(array, ptr);
	if (index == OBJC_ARRAY_NOT_FOUND){
		// Not found
		return;
	}
	
	array_remove_at_index(array, index);
}

void array_remove_at_index(objc_array array, unsigned int index){
	_array arr = (_array)array;
	unsigned int size = array_size(array);
	int i;
	
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
	for (i = index; i < size - 1; ++i){
		arr->_array[i] = arr->_array[i + 1];
	}
	arr->_array[i] = NULL;
	--arr->_currentIndex;
}

unsigned int array_size(objc_array array){
	return ((_array)array)->_currentIndex + 1;
}

#endif // OBJC_USES_INLINE_FUNCTIONS
