CFLAGS=-Wall -Werror -Wextra -g -I..

TESTS=queue-test queue-par-test queue-par-test-2 queue-par-test-cbmc queue-par-cbmc buffer-test
all: $(TESTS)

queue-test: queue-test.c ../queue.[ch]
	$(CC) $(CFLAGS) $^ -o $@
	./$@

buffer-test: buffer-test.c ../shmbuf/buffer.[ch] ../shmbuf/shm.c
	$(CC) $(CFLAGS) $^ -o $@
	./$@

queue-par-test: queue-par-test.c ../parallel_queue.c
	$(CC) $(CFLAGS) $^ -lpthread -o $@
	for i in `seq 1 20`; do ./$@; done

queue-par-test-2: queue-par-test-2.c ../parallel_queue.c
	$(CC) $(CFLAGS) $^ -lpthread -o $@
	./$@

queue-par-test-cbmc: queue-par-cbmc_2.c ../parallel_queue.c
	$(CC) $(CFLAGS) $^ -lpthread -o $@
	for i in `seq 1 20`; do ./$@; done

queue-par-cbmc: queue-par-cbmc.c ../parallel_queue.c
	cbmc $^ -I.. --c11 --unwind 2

queue-par-cbmc_2: queue-par-cbmc_2.c ../parallel_queue.c
	cbmc $^ -I.. --c11 --unwind 2

clean:
	-rm *.o *.a *.so

.PHONY: clean all $(TESTS)

