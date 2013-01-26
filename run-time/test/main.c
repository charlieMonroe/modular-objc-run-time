#include <stdio.h>
#include <time.h>

#include "runtime.h"
#include "class.h"
#include "method.h"
#include "selector.h"
#include "../classes/MRObjects.h"

#include "../extras/ao-ext.h"
#include "../extras/categs.h"


#define OBJC_HAS_AO_EXTENSION 0
#define OBJC_HAS_CATEGORIES_EXTENSION 0

#include "testing.h"

int main(int argc, const char * argv[]){
	register_classes();
	list_classes();
		
	return 0;
}

