#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <pthread.h>
#include <unistd.h>
#include <sched.h>
#include <time.h>
#include <fcntl.h> //wrapper
#include <errno.h> //system error
#include <sys/ioctl.h> //ioctl
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/syscall.h>//added for syscall
#define DRIVER_NAME "/dev/barrier_driver"	//setting the device name

//structure that holds the details of the Barrier which is sent to the Kernel Space
typedef struct barrierInit {
	unsigned int id;		//barrier ID
	long long timeout;	//barrier timeout
	} barrier_sync;

int fd,threadCount1,threadCount2,averageSleep;
long long timeout1,timeout2;
int numberOfSync = 100;	//number of synchronizations
pid_t pid;

static void child_Process();
static void* threadCall(void *barrierData);

int main()
{
	printf("Enter the average sleep value in microseconds: ");
	scanf("%d",&averageSleep);
	
	printf("Enter the count for Barrier 1:");
	scanf("%d", &threadCount1);	

	printf("Enter the timeout for Barrier 1:");
	scanf("%llu", &timeout1);

	printf("Enter the count for Barrier 2:");
	scanf("%d", &threadCount2);

	printf("Enter the timeout for Barrier 2:");
	scanf("%llu", &timeout2);

	int child1PID = fork();
	if(child1PID < 0)
	{
		printf("Creating first child process failed \n");
	}
	else if(child1PID == 0)
	{
		child_Process();	//Creating first child process
	}
	else
	{
		int child2PID = fork();
		if(child2PID < 0)
		{
			printf("Creating second child process failed \n");
		}
		else if (child2PID == 0)
		{
			child_Process();	//Creating second child process
		}
		else
		{
			pid = wait(NULL);  //waiting for child 2
		}
		pid = wait(NULL);	//waiting for child 1
	}

	return 0;
}

void child_Process()
{
	struct barrierInit barrierData[threadCount1+threadCount2];
	barrier_sync *ptr_read = malloc(sizeof(barrier_sync));
	pthread_t pThread[threadCount1+threadCount2];
	int currentPID,i,j,ret,barrierId1,barrierId2,io,rd;

	currentPID = getpid();   //getting the process id

	ptr_read->timeout = timeout1;	//setting the timeout value 
	rd= syscall(359, threadCount1, &ptr_read->id, (int)timeout1);	//invoking the barrier init 
	barrierId1 = ptr_read->id;	//retreiving the id generated
	if ((errno == EINVAL) & (rd == -1))		
	{
		printf("\nError!! Barrier Init Failed for %d PID: %d",barrierId1,currentPID);
	}
	else
	{
		printf("\nBarrier Init successful for %d PID: %d",barrierId1,currentPID);
	}
	
	ptr_read->timeout = timeout2;	//setting the timeout value 
	rd= syscall(359, threadCount2, &ptr_read->id, (int)timeout2);	//invoking the barrier init 
	
	barrierId2 = ptr_read->id;	//retreiving the id generated
	if ((errno == EINVAL) & (rd == -1))		
	{
		printf("\nError!! Barrier Init Failed for %d PID: %d",barrierId2,currentPID);
	}
	else
	{
		printf("\nBarrier Init successful for %d PID: %d",barrierId2,currentPID);
	}
	
	for(i=0;i<threadCount1;i++) //for the first barrier
	{
		barrierData[i].id = barrierId1;	//setting the generated barrier id
		ret = pthread_create( &pThread[i], NULL, threadCall, (void*)&barrierData[i]); //creating the threads
		if(ret != 0)
		{
			printf("\nError!! Thread Creation Failed for %d PID: %d",i,currentPID);
		}
		else
		{
			printf("\nThread Created for %d PID: %d",i,currentPID);
		}
		if(i == (threadCount1-1))
		{
			if(ret == 0)
			{
				printf("\nBarrier Synchronization successful for %d PID: %d",barrierId1,currentPID);	
			}
		}
	}
	for(j=0;j<threadCount2;j++)	//for the second barrier
	{
		barrierData[i+j].id = barrierId2;	//setting the generated barrier id
		ret = pthread_create( &pThread[i+j], NULL, threadCall, (void*)&barrierData[i+j]);	//creating the threads
		if(ret != 0)
		{
			printf("\nError!! Thread Creation Failed for %d PID: %d",i+j+1,currentPID);
		}
		else
		{
			printf("\nThread Created for %d PID: %d",i+j,currentPID);
		}
		if(j == (threadCount2-1))
		{
			if(ret == 0)
			{
				printf("\nBarrier Synchronization successful for %d PID: %d",barrierId2,currentPID);	
			}
		}
	}
	
	for(i=0;i<(threadCount1+threadCount2);i++)
	{	
		ret = pthread_join(pThread[i], NULL);	//performing thread join to synchronize the destroy	
		if(ret != 0)
		{
			printf("\nError!! Thread join failed for %d PID: %d",i,currentPID);
		}
	}
	io = syscall(361, barrierId1);	//invoking the barrier destroy for id1
	if(io != 0)
	{
		printf("\nError!! Destroy Failed for %d PID: %d",barrierId1,currentPID);
	}
	else
	{
		printf("\nBarrier Destroy Successfull for %d PID: %d",barrierId1,currentPID);
	}
	io = syscall(361, barrierId2);	//invoking the barrier destroy for id2
	if(io != 0)
	{
		printf("\nError!! Barrier Destroy Failed for %d PID: %d",barrierId2,currentPID);
	}
	else
	{
		printf("\nBarrier Destroy Successfull for %d PID: %d",barrierId2,currentPID);
	}
	free(ptr_read);
	return;
}

void *threadCall(void *barrierData)
{
	int k,pid,wr;
	barrier_sync *ptr_write = malloc(sizeof(barrier_sync));
	struct barrierInit *barrierdata = (struct barrierInit *)barrierData;
	printf("\n wait called for id : %d",barrierdata->id);
	ptr_write->id = barrierdata->id;	//setting the barrier id
	pid = getpid();		//getting the current process id
	for(k=0;k<numberOfSync;k++)	//for each synchronization cycle
	{
		usleep(rand() % (averageSleep * 2));	//sleep for a random time to create a random behaviour
		wr= syscall(360, barrierdata->id);	//invoking the barrier wait after the sleep
		if ((errno == EINVAL) & (wr == -1))		
		{
			printf("\nError!! Barrier wait Timed Out for %d PID: %d",barrierdata->id,pid);
		}
		else
		{
			printf("\nBarrier wait successful for %d PID: %d",barrierdata->id,pid);
		}
	}
	free(ptr_write);
	return NULL;
}
