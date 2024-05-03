all: library test.o test1.o test2.o
	gcc -o test test.o -L. -l _mem_alloc
	gcc -o test1 test1.o -L. -l _mem_alloc
	gcc -o test2 test2.o -L. -l _mem_alloc

library: _mem_alloc.o
	gcc _mem_alloc.o -shared -o lib_mem_alloc.so

_mem_alloc.o: mem_alloc.h
	gcc -c mem_alloc.c -fPIC -o _mem_alloc.o

test.o test1.o test2.o:
	gcc -c test*.c -fPIC

clean:
	rm test *.o  test test1 test2 lib_mem_alloc.so