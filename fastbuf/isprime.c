int isprime(int p) {
    if (p == 2) return 1;
    if (p % 2 == 0) return 0;
    for (int i = 3; i < p; i += 2)
        if (p % i == 0) return 0;
    return 1;
}
