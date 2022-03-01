CFLAGS=-fPIC -Wall -Werror -Wextra -g

all: shmbuf fastbuf drfun drsyscalls experiments events.o

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

.PHONY: clean all shmbuf fastbuf drfun drsyscalls experiments

