/*
  * This file contains functions that are usually in the C stdlib,
  * but aren't that hard to implement here for speed sake.
  */

#ifndef OBJC_UTILITIES_H_
#define OBJC_UTILITIES_H_

#include "types.h" // For BOOL, NULL, ...
#include "os.h" // For objc_alloc

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
		++str;
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
	char *result = objc_alloc(len + 1); // +1 for zero-termination
	char *curr_char = result;
	
	while (*str != '\0') {
		*curr_char = *str;
		++curr_char;
		++str;
	}
	
	*curr_char = '\0';
	return result;
}

/*
 * Returns YES if the strings are equal.
 */
static inline BOOL objc_strings_equal(const char *str1, const char *str2){
	if (str1 == NULL && str2 == NULL){
		return YES;
	}
	
	if (str1 == NULL || str2 == NULL){
		// Just one of them NULL
		return NO;
	}
	
	unsigned int index = 0;
	while (YES) {
		if (str1[index] == str2[index]){
			if (str1[index] == '\0'){
				// Equal
				return YES;
			}
			++index;
			continue;
		}
		return NO;
	}
	
	return NO;
}

#endif // OBJC_UTILITIES_H_
