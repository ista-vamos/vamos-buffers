#include <stdio.h>

int foo(int x, int y, int z) {
	return x + 1;
}

int main(void) {
    for (int i = 0; i < 1000000; ++i) {
	int a = foo(i, i*i, -i);
	printf("%d\n", a);
	fflush(stdout);
    }
    return 0;
}
