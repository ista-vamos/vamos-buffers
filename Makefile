CFLAGS=-fPIC -Wall -Werror -Wextra -g

all: shamon.so shamon.a

shamon.so: shm.o client.o monitor.o
	$(CC) -shared $^ -o $@

shamon.a: shm.o client.o monitor.o
	ar rcs $@ $^

shm.o: shm.c shm.h

client.o: client.c client.h

monitor.o: monitor.c monitor.h

clean:
	-rm *.o *.a *.so

.PHONY: clean all

