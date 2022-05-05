CFLAGS=-fPIC -Wall -Werror -Wextra -g -O3 -flto
CPPFLAGS=-D_POSIX_C_SOURCE200809L

all: shamon.a shmbuf drfun drsyscalls experiments

shamon.a: shamon.o event.o queue.o stream.o vector.o utils.o parallel_queue.o arbiter.o
	ar ru $@ $^
	ranlib $@

shmbuf:
	+make -C shmbuf

drfun: shmbuf
	+make -C drfun

drsyscalls:
	+make -C drsyscalls

experiments:
	+make -C experiments

clean:
	-rm *.o *.a *.so
	+make -C drfun clean
	+make -C drsyscalls clean
	+make -C streams clean
	+make -C shmbuf clean
	+make -C fastbuf clean

check:
	+make -C tests test

.PHONY: clean all check shmbuf fastbuf drfun drsyscalls experiments

