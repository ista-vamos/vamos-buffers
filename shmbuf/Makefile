CFLAGS=-fPIC -Wall -Werror -Wextra -g

all: libshamon.so libshamon.a

libshamon.so: buffer.o shm.o client.o ../parallel_queue.o
	$(CC) -shared $^ -o $@

libshamon.a: buffer.o shm.o client.o ../parallel_queue.o
	ar rcs $@ $^

shm.o: shm.c shm.h

client.o: client.c client.h

clean:
	-rm *.o *.a *.so

.PHONY: clean all

