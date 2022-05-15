#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <threads.h>
#include <string.h>
#include "shm_monitor.h"

int isprime(int);

int monitoring_active=1;
int do_print=0;

int monitoring_thread(void* arg)
{
	monitor_buffer buffer=(monitor_buffer)arg;
	buffer_entry buffer_buffer[32];
	while(monitoring_active)
	{
		size_t count = copy_events_wait(buffer, buffer_buffer, 32);
		for(size_t i=0;i<count;i++)
		{
			int64_t num = buffer_buffer[i].payload64_1;
			if(isprime(num))
			{
				if(do_print)
				{
					printf("Prime: %li\n", num);
				}
			}
			else
			{
				printf("Not a prime: %li\n", num);
			}
		}
	}
	return 0;
}

int register_monitored_thread(monitor_buffer buffer)
{
	thrd_t thrd;
	thrd_create(&thrd, &monitoring_thread, buffer);
	return 0;
}

int main(int argc, char** argv)
{
	if(argc<2)
	{
		printf("Usage: monitor [PID] {print}");
		return 1;
	}
	pid_t process_id = atoi(argv[1]);
	if(argc>2)
	{
		if(strncmp(argv[2],"print",6)==0)
		{
			do_print=1;
		}
	}
	
	monitored_process proc = attach_to_process(process_id, &register_monitored_thread);

	wait_for_process(proc);
	monitoring_active=0;
	return 0;
}
