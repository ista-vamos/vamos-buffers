CFLAGS=-fPIC -Wall -Werror -Wextra -g

all: shamon.a shmbuf fastbuf drfun drsyscalls experiments

shamon.a: shamon.o event.o arbiter.o queue.o stream.o vector.o utils.o
	ar ru $@ $^
	ranlib $@

shmbuf:
	make -C shmbuf

drfun: shmbuf
	make -C drfun

drsyscalls:
	make -C drsyscalls

experiments:
	make -C experiments

clean:
	-rm *.o *.a *.so
	make -C drfun clean
	make -C drsyscalls clean
	make -C streams clean
	make -C shmbuf clean
	make -C fastbuf clean

check:
	make -C tests test

.PHONY: clean all check shmbuf fastbuf drfun drsyscalls experiments

