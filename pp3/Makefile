current: trylib

lib.o: lib.c lib.h
	gcc -c lib.c

trylib.o: lib.c trylib.c
	gcc -c trylib.c

trylib: lib.o trylib.o
	gcc -o trylib trylib.o lib.o
