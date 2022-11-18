#include "_kernelCore.h"
#include "stdio.h"

//task management: Our first, simple scheduler will just care about the index of the current task
int osCurrentTask = 0;

//I am using a static array of tasks. Feel free to do something more interesting
thread osThreads[MAX_THREADS];

/*
	These next two variables are useful but in Lab Project 2 they do the
	exact same thing. Eventually, when I start and put tasks into a BLOCKED state,
	the number of threads we have and the number of threads running will not be the same.
*/
int threadNums = 0; //number of threads actually created
int osNumThreadsRunning = 0; //number of threads that have started runnin

//Having access to the MSP's initial value is important for setting the threads
uint32_t mspAddr; //the initial address of the MSP


void SysTick_Handler(void){
	
	//i starts from 1 to skip over idle task
	for(int i =1; i<threadNums; i++){
		osThreads[i].deadline--;
		osThreads[i].sleepTime--;
		
		if(osThreads[i].sleepTime == 0)
			osWakeUp(i);
	}
		
}

/*
	Performs various initialization tasks.
	
	It needs to:
	
		- Set priority of PendSV interrupt
		- Detect and store the initial location of MSP
*/
void kernelInit(void)
{
	//initialize the address of the MSP
	uint32_t* MSP_Original = 0;
	mspAddr = *MSP_Original;
	
	//set the priority of PendSV to the weakest it can be
	SHPR3 |= 0xFE << 16;
	SHPR3 |= 0xFFU << 24;
	SHPR2 |= 0xF << 24;
}

/*
	Sets the value of PSP to threadStack and sures that the microcontroller
	is using that value by changing the CONTROL register.
*/
void setThreadingWithPSP(uint32_t* threadStack)
{
	__set_CONTROL(1<<1);
	__set_PSP((uint32_t)threadStack);
}

bool osKernelStart()
{
	if(threadNums > 0)
	{
		osCurrentTask = -1;
		__set_CONTROL(1<<1);
		__set_PSP((uint32_t)osThreads[0].taskStack);
		osSched();
	}
	return 0;
}

int task_switch(void){
		__set_PSP((uint32_t)osThreads[osCurrentTask].taskStack); //set the new PSP
		return 1; //You are free to use this return value in your assembly eventually. It will be placed in r0, so be sure
		//to access it before overwriting r0
}


void osYield(void){
	__ASM("SVC #0");
}

void osSleep(void){
	__ASM("SVC #1");	
}


//change status of thread that woke up 
//add it to queue
void osWakeUp(int index){
	osThreads[index].status = READY;
	osThreads[index].deadline = osThreads[index].DEADLINE;
	
	if(osThreads[index].deadline < osThreads[osCurrentTask].deadline)
		__ASM("SVC #2");
	
}

/*
	The OS Scheduler.

	It is responsible for:

	- Saving the current stack pointer
	- Setting the current thread to WAITING (useful later on when we have multiple ways to wait
	- Finding the next task to run (As of Lab Project 2, it just cycles between all of the tasks)
	- Triggering PendSV to perform the task switch
*/ 
void osSched(void)
{
		//first time running case
		//initialize queue and sets currentTask to 0
		if(osCurrentTask == -1)
		{
		
			int lowestDeadline = 0;
			
			for(int i = 1; i < threadNums; i++){
				if(osThreads[i].DEADLINE < osThreads[lowestDeadline].DEADLINE)
					lowestDeadline = i;
			}
			
			osCurrentTask = lowestDeadline;
			osThreads[osCurrentTask].status = RUNNING;

			_ICSR |= 1<<28;
			__asm("isb");
			
		}
		//every other time running case
		else{
			
			int lowestDeadline = 0;
			
			//in case thread wakes up with lowest deadline. It's deadline timer will still be the lowest (negative)
			for(int i = 1; i < threadNums; i++){
				if(osThreads[i].deadline < osThreads[lowestDeadline].deadline && osThreads[i].status != SLEEP)
					lowestDeadline = i;
			}
			
			
			osCurrentTask = lowestDeadline;
			osThreads[osCurrentTask].status = RUNNING;
			
		}
}

void SVC_Handler_Main(uint32_t *svc_args)
{
	char call = ((char*)svc_args[6]) [-2];
	
	if(call == 1){
		osThreads[osCurrentTask].status = SLEEP;
		osThreads[osCurrentTask].sleepTime = osThreads[osCurrentTask].SLEEPTIME;
	}
		
	if(call == 0){
		osThreads[osCurrentTask].status = READY;
		osThreads[osCurrentTask].deadline = osThreads[osCurrentTask].DEADLINE;
	}
		
	osThreads[osCurrentTask].taskStack = (uint32_t*)(__get_PSP() - 8*4);
	
	osSched();
			
	_ICSR |= 1<<28;
	__asm("isb");

}
	




/*Head and tail pointers to queue 
struct indexNode *front = NULL;
struct indexNode *rear = NULL;
enqueue and deqeue functions
void enqueue(int nextIndex)
{
		//create node and return pointer to it
		struct indexNode * ptr;
		ptr = malloc(sizeof(struct indexNode));
	

    ptr->index = nextIndex;
    ptr->next = NULL;
    if (rear == NULL)
    {
        front = ptr;
        rear = ptr;
    }
    else
    {
        rear->next = ptr;
        rear = rear->next;
    }
}

void dequeue()
{
    if (front == NULL)
    {
        printf("queue is empty");
    }
    else
    {
        struct indexNode *temp;
        temp = front;
        front = front->next;
        free(temp);
    }
}*/

