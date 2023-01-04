# Shamon

A library and a set of tools for monitoring heterogeneous asynchronous events
sources. Events are efficiently transferred via concurrent buffers in the
shared memory.

### Configuring

```
# simple configuration
cmake . -DCMAKE_C_COMPILER=clang

# configuration without dynamorio sources
cmake . -DCMAKE_C_COMPILER=clang -DDYNAMORIO_SOURCES=OFF
```

Note that using `clang` is not compulsory, but some helper scripts (e.g.,
`sources/tsan/compiler.py`) rely on that and there is a risk that these scripts
will not work in the release build if the library is not compiled with clang. A
work-around, if needed, is turning of interprocedural optimizations (IPO) in
cmake (use `-DENABLE_IPO=OFF` while configuring).
