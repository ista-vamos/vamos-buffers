CPPFLAGS=#-I.. -I/home/fabian/DynamoRIO-Linux-9.0.0/include -I/home/fabian/DynamoRIO-Linux-9.0.0/ext/include
CFLAGS=-O3 -g3

all: monitor primes #gennums_hardcoded-instr

monitor: monitor.o shm_monitor.o shm_common.o isprime.o
	$(CC) $^ -O0 -g $(CPPFLAGS) -o $@ -lrt -lm -lpthread

isprime.o: isprime.c
	$(CC) -c $< $(CFLAGS) $(CPPFLAGS) -o $@

shm_common.o : shm_common.c shm_common.h
	$(CC) -c $< $(CFLAGS) $(CPPFLAGS) -o $@

shm_monitor.o : shm_monitor.c shm_monitor.h shm_common.h
	$(CC) -c $< $(CFLAGS) $(CPPFLAGS) -o $@

shm_monitored.o : shm_monitored.c shm_monitored.h shm_common.h
	$(CC) -c $< $(CFLAGS) $(CPPFLAGS) -o $@

gennums_hardcoded.o : gennums_hardcoded.c shm_monitored.h shm_common.h
	$(CC) -c $< $(CFLAGS) $(CPPFLAGS) -o $@

primes: gennums_hardcoded.o shm_monitored.o shm_common.o
	$(CC) $^ $(CFLAGS) $(CPPFLAGS) -o $@ -lpthread

clean:
	-rm *.o monitor primes

.PHONY: clean all
