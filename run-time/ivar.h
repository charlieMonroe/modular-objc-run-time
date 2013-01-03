
#ifndef OBJC_IVAR_H_
#define OBJC_IVAR_H_

typedef struct _objc_ivar_t {
	const char *name;
	const char *type;
	unsigned int offset;
} *Ivar;

#endif /* OBJC_IVAR_H_ */
