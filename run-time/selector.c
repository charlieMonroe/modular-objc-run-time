#include "selector.h"
#include "os.h" /* For run-time functions */
#include "utils.h" /* For strcpy */

static objc_selector_holder selector_cache;

SEL objc_selector_register(const char *name){
	SEL selector = objc_selector_holder_lookup(selector_cache, name);
	if (selector == NULL){
		/* Check if the selector hasn't been added yet */
		selector = (SEL)objc_selector_holder_lookup(selector_cache, name);
		if (selector == NULL){
			/* Still nothing, insert */
			selector = objc_alloc(sizeof(struct objc_selector));
			selector->name = objc_strcpy(name);
			objc_selector_holder_insert(selector_cache, selector);
		}
	}
	return selector;
}

BOOL objc_selectors_equal(SEL selector1, SEL selector2){
	if (selector1 == NULL && selector2 == NULL){
		return YES;
	}
	if (selector1 == NULL || selector2 == NULL){
		return NO;
	}
	
	return selector1 == selector2 || objc_strings_equal(selector1->name, selector2->name);
}

const char *objc_selector_get_name(SEL selector){
	if (selector == NULL){
		return "((null))";
	}
	return selector->name;
}

void objc_selector_init(void){
	selector_cache = objc_selector_holder_create();
}

