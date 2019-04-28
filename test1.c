#define _GNU_SOURCE
#include<stdio.h>
#include<stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sched.h>
#include <time.h>    // time()

#define maxCPU 3
#define minCPU 0
#define cpuAffi 2
#define periodParam 1
#define progParam 0
const char* progName="./ano ";
static int slowBranch(int period,int cpuAffinity){
	pid_t forkpid;
	cpu_set_t set;
	forkpid=fork();
	if (cpuAffinity>=maxCPU&&cpuAffinity<=minCPU){
		exit(-1);
	}
	CPU_ZERO(&set);

	if (forkpid!=-1){
		printf("Current process is %d,parent is %d \n",getpid(),getppid());
		sleep(period);
		CPU_SET(cpuAffinity, &set);
		 if (sched_setaffinity(getpid(), sizeof(set), &set) == -1){
		 	exit(-1);
		 }
		slowBranch(period);
	}
	else{
		exit(-1);
	}
	return 0;
}
// 
int main( int argc, char* argv[] ){
	// char* currentType=NULL;
	char* slowType="slowbranch";
	// char* randomType="randombranch";
	int i=0;
	int period=1;
	int forTerminate=50;
	int forFork=50;
	int cpuAffinity;
	// if( argc < num_expected_args){
	// 	printf("Usage: ./anomalous_process <tyep of decision procedure> <period of time in seconds> <terminate value> <fork value>\n");
	// 	exit(-1);
	// }
	if(argc==3)period=atoi(argv[periodParam]);
	if(argc==4)cpuAffinity=atoi(argv[cpuAffi]);
	printf("Name of the Program %s.\n",argv[progParam]);
	printf("Period to fork new process %d.\n", period);
		printf("Type of decision procedure %s.\n",slowType);
		slowBranch(period,cpuAffinity);
	return 0;
}