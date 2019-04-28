#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sched.h>
#include <time.h>

#define maxCPU 3
#define minCPU 0

static int work(int period, int cpuAffinity){
	pid_t forkpid;
	cpu_set_t set;
	forkpid = fork();
	if (cpuAffinity>=maxCPU && cpuAffinity<=minCPU){
		exit(-1);
	}
	CPU_ZERO(&set);

	if (forkpid!=-1){
		printf("Current process is %d, parent is %d \n", getpid(), getppid());
		sleep(period);

		CPU_SET(cpuAffinity, &set);
		if (sched_setaffinity(getpid(), sizeof(set), &set) == -1){
			exit(-1);
		}
		work(period, cpuAffinity);
	} else {
		exit(-1);
	}
	return 0;
}

int main( int argc, char* argv[] ){
	int period = 1;
	int cpuAffinity = maxCPU;

	period = atoi(argv[1]);
	cpuAffinity = atoi(argv[2]);
	
	work(period, cpuAffinity);
	return 0;
}