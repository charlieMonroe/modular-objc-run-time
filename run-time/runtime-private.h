/*
 * This file contains declarations of symbols private to the run-time.
 */

#ifndef OBJC_RUNTIME_PRIVATE_H_
#define OBJC_RUNTIME_PRIVATE_H_
	 
/* Need to include the public header for the objc_runtime_setup struct def. */
#include "runtime.h"

/**
 * An external run-time setup structure. This structure shouldn't be modified
 * from anywhere after the run-time is started.
 */
extern objc_runtime_setup_struct objc_setup;
 
#endif /* OBJC_RUNTIME_PRIVATE_H_ */
 