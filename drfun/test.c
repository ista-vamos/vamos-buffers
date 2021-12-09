int foo(int x) {
	return x + 1;
}

int foo2(int *y) {
	return foo(*y);
}

int main(void) {
	int a = 10;
	for (int i = 0; i < 1000; ++i)
		a = foo2(&a);
	printf("%d\n", a);
	return 0;
}
