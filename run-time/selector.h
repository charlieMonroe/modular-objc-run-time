
#ifndef OBJC_SELECTOR_H_
#define OBJC_SELECTOR_H_

#include "types.h"

/* The selector name is copied over */
extern SEL objc_selector_register(const char *name);

/* Pointer and name comparison */
extern BOOL objc_selectors_equal(SEL selector1, SEL selector2);

/* Returns the selector name */
extern const char *objc_selector_get_name(SEL selector);

#endif /* OBJC_SELECTOR_H_ */
