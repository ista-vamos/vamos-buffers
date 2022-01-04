#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <string.h>
#include "shm_monitored.h"

int  __attribute__((noinline)) isprime(int p) {
	return 1;
}
int  __attribute__((noinline)) isprime_mon(int p) {
	push_event_wait_64(1, p);
	return 1;
}
int  __attribute__((noinline)) isprime_print(int p) {
	printf("prime: %u\n", p);
	return 1;
}
int  __attribute__((noinline)) isprime_print_mon(int p) {
	printf("prime: %u\n", p);
	push_event_wait_64(1, p);
	return 1;
}

#define NUM 300000


int main(int argc, char* argv[]) {
	struct timespec begin, end;
	unsigned primes[NUM];
	unsigned primes_num = 0;
	unsigned cur = 2;
	unsigned n = 0;
	int (*isprimefun)(int) = &isprime;
	int doprint=0;
	int domonitor=0;
	for(int i=1;i<argc;i++)
	{
		if(strncmp(argv[i],"print",6)==0)
		{
			doprint=1;
		}
		else if(strncmp(argv[i],"mon",6)==0)
		{
			domonitor=1;
		}
	}
	if(domonitor)
	{
		initialize_application_buffer();
		intialize_thread_buffer(1,2);
		app_buffer_wait_for_client();
		if(doprint)
		{
			isprimefun=&isprime_print_mon;
		}
		else
		{
			isprimefun=&isprime_mon;
		}
	}
	else if(doprint)
	{
		isprimefun=&isprime_print;
	}
	if (clock_gettime(CLOCK_MONOTONIC, &begin) == -1)
		perror("clock_gettime");
	while (n++ < NUM) {
		int prime = 1;
		for (unsigned i = 0; i < primes_num; ++i) {
			if (cur % primes[i] == 0)
				prime = 0;
		}
		if (prime) {
			primes[primes_num++] = cur;
			isprimefun(cur);
#if 0
			if (!isprime(cur)) {
				fprintf(stderr, "Not a prime: %u\n", cur);
			}
#endif
		}
		++cur;
	}
	if (clock_gettime(CLOCK_MONOTONIC, &end) == -1)
		perror("clock_gettime");
	long seconds = end.tv_sec - begin.tv_sec;
	long nanoseconds = end.tv_nsec - begin.tv_nsec;
	double elapsed = seconds + nanoseconds*1e-9;
	printf("Time: %lf seconds.\n", elapsed);
	if(domonitor)
	{
		close_thread_buffer();
		close_app_buffer();
	}
}
