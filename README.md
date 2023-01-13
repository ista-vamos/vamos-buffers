# Vamos-buffers

VAMOS is a framework for monitoring heterogeneous asynchronous events
sources. Events are efficiently transferred via concurrent buffers in the
shared memory and the implementation of (not only) the shared buffers is
in this repository.

### Configuring and building

```
# simple configuration & build & runnning tests

cmake . -DCMAKE_C_COMPILER=clang
make -j4
make -j4 tests
```

Note that using `clang` is not compulsory, but some helper scripts in other
VAMOS repositories (e.g., `vamos-sources/tsan/compiler.py`) rely on that and
there is a risk that these scripts will not work in the release build if the
library is not compiled with clang. A work-around, if needed, is turning of
interprocedural optimizations (IPO) in cmake (use `-DENABLE_IPO=OFF` while
configuring).

### Project structure

```
 - src/         # main source code
     - core       # implementation of buffers and streams (an abstraction over the buffers)
     - shmbuf     # creating buffers in the shared memory
     - streams    # auxiliary code for streams
 - include/     # public headers
 - python/      # python bindings (experimental)
 - cmake/       # cmake configuration files
 - tests/       # tests
```
