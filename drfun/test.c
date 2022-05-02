#include <stdio.h>
#include <stdlib.h>

int foo(int x, int y, int z) {
	return x + 1;
}

int main(int argc, const char *argv[]) {
    int N = atoi(argv[1]);
    for (int i = 0; i < N; ++i) {
	int a = foo(i % 500000 ? i+1 : i, i*i, -i);
	printf("#@%d@#\n", a);
    }
    return 0;
}
