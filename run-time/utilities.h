/*
  * This file contains functions that are usually in the C stdlib,
  * but aren't that hard to implement here for speed sake.
  */

#ifndef OBJ_UTILITIES_H_
#define OBJ_UTILITIES_H_

#include "runtime.h"

/*
  * Just as the regular strlen function, returns a number of non-zero characters.
  */
static inline unsigned int objc_strlen(const char *str){
	if (str == NULL){
		return 0;
	}
	
	unsigned int counter = 0;
	while (*str != '\0') {
		++counter;
		str++;
	}
	return counter;
}

/*
 * Unlike the POSIX function, this one handles allocating the new string itself.
 */
static inline char *objc_strcpy(const char *str){
	if (str == NULL){
		return NULL;
	}
	
	unsigned int len = objc_strlen(str);
	char *result = objc_setup.memory.allocator(len + 1); // +1 for zero-termination
	char *curr_char = result;
	
	while (*str != '\0') {
		*curr_char = *str;
		++curr_char;
		++str;
	}
	
	*curr_char = '\0';
	return result;
}

#endif // OBJ_UTILITIES_H_
