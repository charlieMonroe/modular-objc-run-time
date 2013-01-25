
CFLAGS=-std=c99 -O3 -DUSES_C89=0 -DHAS_ALWAYS_INLINE_ATTRIBUTE=1 -DOBJC_USES_INLINE_FUNCTIONS=0
LINKER=`x=\`uname\`; \
		if [ $${x} = Darwin ]; then \
		  	echo ld; \
		fi`
LFLAGS=`x=\`uname\`; \
		if [ $${x} = Darwin ]; then \
		  	echo '-r -macosx_version_min 10.5'; \
		fi`


all : test-no-lto test-with-lto test-cocoa
	echo "Done."

test-cocoa : main.m
	cc -lobjc main.m -o objc_test_cocoa

test-no-lto : static
	cc -L/usr/lib/ -L. -lobjc-runtime main.c -o objc_test_no_lto

test-with-lto : static
	cc -L/usr/lib/ -L. -lobjc-runtime -flto main.c -o objc_test_with_lto

main.o : main.c
	cc -c main.c

static: class.o method.o runtime.o selector.o array.o holder.o ao.o posix.o MRObjects.o MRObjectMethods.o
	$(LINKER) class.o method.o runtime.o selector.o array.o holder.o ao.o posix.o MRObjects.o MRObjectMethods.o $(LFLAGS) -o libobjc-runtime.a

MRObjectMethods.o : classes/MRObjectMethods.c
	cc $(CFLAGS) -c classes/MRObjectMethods.c -o MRObjectMethods.o
MRObjects.o : classes/MRObjects.c
	cc $(CFLAGS) -c classes/MRObjects.c -o MRObjects.o
class.o : class.c
	cc $(CFLAGS) -c class.c
method.o : method.c
	cc $(CFLAGS) -c method.c
runtime.o : runtime.c private.h
	cc $(CFLAGS) -c runtime.c
selector.o : selector.c
	cc $(CFLAGS) -c selector.c
array.o : structs/array.c
	cc $(CFLAGS) -c structs/array.c -o array.o
holder.o : structs/holder.c
	cc $(CFLAGS) -c structs/holder.c -o holder.o
ao.o : extras/ao-ext.c
	cc $(CFLAGS) -c extras/ao-ext.c -o ao.o
posix.o : extras/posix.c
	cc $(CFLAGS) -c extras/posix.c -o posix.o

clean: 
	rm *.o *.a objc_test_no_lto objc_test_with_lto objc_test_cocoa

