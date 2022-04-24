#include <stdio.h>

int foo(int x, int y, int z) {
	return x + 1;
}

int main(void) {
	int a = 10;
    for (int i = 0; i < 10; ++i) {
		a = foo(a, a*a, -a);
		printf("%d\n", a);
		fflush(stdout);
	}
	return 0;
}
