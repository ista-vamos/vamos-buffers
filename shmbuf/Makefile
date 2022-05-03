CFLAGS=-fPIC -Wall -Werror -Wextra -g -O2
CPPFLAGS=-I..

all: libshmbuf.so libshmbuf.a

libshmbuf.so: buffer.o shm.o client.o ../parallel_queue.o ../vector.o
	$(CC) -shared $^ -o $@

libshmbuf.a: buffer.o shm.o client.o ../parallel_queue.o ../vector.o
	ar rcs $@ $^

clean:
	-rm *.o *.a *.so

.PHONY: clean all

