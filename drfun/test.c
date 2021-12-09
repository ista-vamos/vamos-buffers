int foo(int x) {
	return x + 1;
}

int foo2(int *y) {
	return foo(*y);
}

int main(void) {
	int a = 10;
	a = foo2(&a);
	printf("%d\n", a);
	return 0;
}
