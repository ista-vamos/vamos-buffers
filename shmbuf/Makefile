CFLAGS=-fPIC -Wall -Werror -Wextra -g -O2 -flto
CPPFLAGS=-I..

all: libshmbuf.so libshmbuf.a

libshmbuf.so: buffer.o shm.o client.o ../parallel_queue.o
	$(CC) -shared $^ -o $@

libshmbuf.a: buffer.o shm.o client.o ../parallel_queue.o
	ar rcs $@ $^

clean:
	-rm *.o *.a *.so

.PHONY: clean all

