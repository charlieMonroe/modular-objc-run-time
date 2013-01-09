
#ifndef OBJC_CLASS_EXTENSION_H_
#define OBJC_CLASS_EXTENSION_H_

#include "types.h"

typedef struct _objc_class_extension {
	/**
	 * A pointer to the next extension. Extensions
	 * are stored in a linked list. Do not modify this
	 * field.
	 */
	struct _objc_class_extension *next_extension;
	
	/**
	 * NOTE: any of the following functions *may* be NULL if no
	 * action is required.
	 */
	
	/**
	 * This function is responsible for initializing
	 * the extra space in the class. The second parameter
	 * is a pointer to the extra space within the class
	 * structure.
	 */
	void(*class_initializer)(Class, void*);
	
	/**
	 * This function is responsible for initializing
	 * the extra space in an object. The second parameter
	 * is a pointer to the extra space within the object
	 * structure.
	 */
	void(*object_initializer)(id, void*);
	
	/**
	 * This function is responsible for deallocating
	 * any dynamically allocated space within the object
	 * extra space.
	 */
	void(*object_deallocator)(id, void*);
	
	
	/**
	 * These are functions that allows to modify the method
	 * implementation lookup. 
	 */
	Method(*instance_lookup_function)(Class, SEL);
	Method(*class_lookup_function)(Class, SEL);
	
	/**
	 * Extra space in the class and object structures requested.
	 * If the target architecture requires any alignments,
	 * the size must be so that a pointer following
	 * the extra space is valid.
	 */
	unsigned int extra_class_space;
	unsigned int extra_object_space;
	
	/**
	 * When the run-time is initialized, each extension's
	 * structure gets a pre-computed offsets from the extra
	 * space part of the object or class. This is merely a convenience
	 * so that the class extension list doens't need to be iterated
	 * every time an extension needs to access the object's extra
	 * space.
	 *
	 * Note though, that this is the offset *after* the end of the
	 * object variables.
	 */
	unsigned int class_extra_space_offset;
	unsigned int object_extra_space_offset;
} objc_class_extension;

/* Caller is responsible for keeping the structure in memory. */
extern void objc_class_add_extension(objc_class_extension *extension);

extern void *objc_class_extensions_beginning(Class cl);
extern void *objc_object_extensions_beginning(id obj);


#endif /* OBJC_CLASS_EXTENSION_H_ */
