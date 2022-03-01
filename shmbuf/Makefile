CFLAGS=-fPIC -Wall -Werror -Wextra -g

all: libshamon.so libshamon.a

libshamon.so: shm.o client.o monitor.o
	$(CC) -shared $^ -o $@

libshamon.a: shm.o client.o monitor.o
	ar rcs $@ $^

shm.o: shm.c shm.h

client.o: client.c client.h

monitor.o: monitor.c monitor.h

clean:
	-rm *.o *.a *.so

.PHONY: clean all

