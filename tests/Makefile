CFLAGS=-Wall -Werror -Wextra -g -I..

all: queue-test queue-par-test queue-par-test-2 queue-par-cbmc

queue-test: queue-test.c ../queue.[ch]

queue-par-test: queue-par-test.c ../parallel_queue.c
	$(CC) $(CFLAGS) $^ -lpthread -o $@
	for i in `seq 1 20`; do ./$@; done

queue-par-test-2: queue-par-cbmc_2.c ../parallel_queue.c
	$(CC) $(CFLAGS) $^ -lpthread -o $@
	for i in `seq 1 20`; do ./$@; done

queue-par-cbmc: queue-par-cbmc.c ../parallel_queue.c
	cbmc $^ -I.. --c11 --unwind 2

queue-par-cbmc_2: queue-par-cbmc_2.c ../parallel_queue.c
	cbmc $^ -I.. --c11 --unwind 2

clean:
	-rm *.o *.a *.so

.PHONY: clean all

