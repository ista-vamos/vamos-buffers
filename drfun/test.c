#include <stdio.h>
#include <stdlib.h>

int foo(int x, const char *s, int z) __attribute__((noinline));
int foo(volatile int x, const char *s, int z) {
	return x + 1;
}

const char *strs[] = {
        "ahoj",
        "tw",
        "whoo",
        "hell!ou"
};

int main(int argc, const char *argv[]) {
    int N = atoi(argv[1]);
    for (int i = 0; i < N; ++i) {
	int a = foo(i, strs[i % 4], -i);
    }
    return 0;
}
