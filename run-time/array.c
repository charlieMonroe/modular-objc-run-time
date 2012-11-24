

#include "array.h"
#include "runtime-private.h"

// Default capacity for the array
static const unsigned int _objc_array_default_capacity = 4;

// Internal representation of objc_array
typedef struct _objc_array {
	unsigned int _capacity;
	unsigned int _currentIndex; // Pointer to the last item in array
	void **_array;
} *_objc_array;

// Grows the array dynamically
static inline void _objc_array_grow(_objc_array arr){
	// Don't be too agressive growing - adding half of the original capacity
	// is quite enough
	arr->_capacity += (arr->_capacity) / 2;
	arr->_array = objc_setup.memory.reallocator(arr->_array, sizeof(void*) * arr->_capacity);
	
	// Realloc doesn't zero the memory, need to do it manually
	for (int i = arr->_currentIndex + 1; i < arr->_capacity; ++i){
		arr->_array[i] = NULL;
	}
}


objc_array objc_array_create(void){
	_objc_array arr = objc_setup.memory.allocator(sizeof(struct _objc_array));
	arr->_capacity = _objc_array_default_capacity;
	arr->_currentIndex = -1;
	arr->_array = objc_setup.memory.zero_allocator(sizeof(void*) * arr->_capacity);
	
	return arr;
}

void objc_array_destroy(objc_array array){
	if (array == NULL){
		return;
	}
	
	objc_setup.memory.deallocator(((_objc_array)array)->_array);
	objc_setup.memory.deallocator(array);
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
	if (index >= arr->_currentIndex){
		objc_setup.logging.log("%s - Index out of range (%i / %i)", __FUNCTION__, index, arr->_currentIndex);
		objc_setup.execution.abort("Index out of bounds.");
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
		objc_setup.logging.log("%s - Index out of range (%i / %i)", __FUNCTION__, index, arr->_currentIndex);
		objc_setup.execution.abort("Index out of bounds.");
	}
	
	if (index == size - 1){
		// Easy case, it's the last item, just wipe it and move the pointer
		arr->_array[index] = NULL;
		--arr->_currentIndex;
		return;
	}
	
	// Otherwise - take the last item and move it to the index slot
	arr->_array[index] = arr->_array[arr->_currentIndex];
	arr->_array[arr->_currentIndex] = NULL;
	--arr->_currentIndex;
}

unsigned int objc_array_size(objc_array array){
	return ((_objc_array)array)->_currentIndex + 1;
}

