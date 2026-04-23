test: logstore_test.c testing.h logstore.o
	gcc -Wall -O3 -g logstore_test.c testing.h logstore.o -o test -lpthread

logstore.o: logstore.c logstore.h
	gcc -Wall -O3 -c logstore.h logstore_internal.h logstore.c 

clean:
	rm -rf *.o test *.gch

