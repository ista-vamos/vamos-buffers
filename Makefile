CFLAGS=-fPIC -Wall -Werror -Wextra -g

all: libshamon.so libshamon.a drfun

libshamon.so: shm.o client.o monitor.o
	$(CC) -shared $^ -o $@

libshamon.a: shm.o client.o monitor.o
	ar rcs $@ $^

shm.o: shm.c shm.h

client.o: client.c client.h

monitor.o: monitor.c monitor.h

drfun: libshamon.so
	make -C drfun

clean:
	-rm *.o *.a *.so
	make -C drfun clean

.PHONY: clean all drfun

