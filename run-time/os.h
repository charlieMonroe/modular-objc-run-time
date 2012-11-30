//
//  os.h
//  modular_runtime
//
//  Created by Charlie Monroe on 11/29/12.
//  Copyright (c) 2012 Fuel Collective, LLC. All rights reserved.
//

#ifndef OBJC_OS_H_
#define OBJC_OS_H_

#define OBJC_USES_INLINE_FUNCTIONS 0

#if OBJC_USES_INLINE_FUNCTIONS

/********* INLINE FUNCTIONS *********/
//#if TARGET_MY_OS
	//#include "os-my-os.h"
//#else
	#error "This OS isn't supported at the moment."
//#endif

#else

/********* FUNCTION POINTERS *********/

#include "runtime-private.h"

#define objc_alloc objc_setup.memory.allocator



#endif

#endif // OBJC_OS_H_
