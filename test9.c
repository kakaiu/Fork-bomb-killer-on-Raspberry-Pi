#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sched.h>
#include <time.h>


static int slowBranch(int cpuAffinity){
	cpu_set_t set;
	CPU_ZERO(&set);
	fork();
	CPU_SET(cpuAffinity, &set);
	if (sched_setaffinity(getpid(), sizeof(set), &set) == -1){
		exit(-1);
	}
	while(1) {
		continue;
	}

	return 0;
}

int main( int argc, char* argv[] ){
	slowBranch(1);
	return 0;
}