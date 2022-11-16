#include "_kernelCore.h"
#include "stdio.h"

//task management: Our first, simple scheduler will just care about the index of the current task
int osCurrentTask = 0;

//this solution won't work next lab
 int sleepTimer1 = 500;
 int sleepTimer2 = 1200;

//I am using a static array of tasks. Feel free to do something more interesting
thread osThreads[MAX_THREADS];

//Head and tail pointers to queue 
struct indexNode *front = NULL;
struct indexNode *rear = NULL;

//enqueue and deqeue functions
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
}


void SysTick_Handler(void){
	
	//decrement all timers
	osThreads[osCurrentTask].runTime--;
	
	if(osThreads[1].sleepTime >= 0)
		osThreads[1].sleepTime--;
	if(osThreads[2].sleepTime >= 0)
		osThreads[2].sleepTime--;
	
	//Sleep timer finish
	if(osThreads[1].sleepTime == 0)
			osWakeUp(1);
	else if(osThreads[2].sleepTime == 0)
			osWakeUp(2);
	
	//Round Robin timer finish
	if(osThreads[osCurrentTask].runTime == 0){
		
		osThreads[osCurrentTask].taskStack = (uint32_t*)(__get_PSP() - 8*4);
		osSched();
			
		_ICSR |= 1<<28;
		__asm("isb");
	}
		
}

/*
	These next two variables are useful but in Lab Project 2 they do the
	exact same thing. Eventually, when I start and put tasks into a BLOCKED state,
	the number of threads we have and the number of threads running will not be the same.
*/
int threadNums = 0; //number of threads actually created
int osNumThreadsRunning = 0; //number of threads that have started runnin

//Having access to the MSP's initial value is important for setting the threads
uint32_t mspAddr; //the initial address of the MSP


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
	SHPR3 |= 0xFF << 16;
	
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

/*
	The OS Scheduler.

	It is responsible for:

	- Saving the current stack pointer
	- Setting the current thread to WAITING (useful later on when we have multiple ways to wait
	- Finding the next task to run (As of Lab Project 2, it just cycles between all of the tasks)
	- Triggering PendSV to perform the task switch
*/


//yields current running thread and calls osSched
//calls osSched at the end to schedule the next thread in queue
void osYield(void){
	
	printf("%d\n",osThreads[osCurrentTask].runTime);
	
	osThreads[osCurrentTask].runTime = -1;
	osThreads[osCurrentTask].taskStack = (uint32_t*)(__get_PSP() - 16*4);
	osSched();

	printf("%d\n",osThreads[osCurrentTask].runTime);
	//context switch-
	_ICSR |= 1<<28;
	__asm("isb");
}

//puts thread to sleep and turns on timer 
//calls osSched at the end to schedule the next thread in queue
void osSleep(void){
	
	osThreads[osCurrentTask].runTime = -1;
	
	if(osCurrentTask == 1)	
		osThreads[osCurrentTask].sleepTime = sleepTimer1;
	else
		osThreads[osCurrentTask].sleepTime = sleepTimer2;

	osThreads[osCurrentTask].status = SLEEP;
	osYield();
	
}

//change status of thread that woke up 
//add it to queue
void osWakeUp(int index){
	osThreads[index].status = READY;
	enqueue(index);
}


//manages ready queue 
void osSched(void)
{
		//first time running case
		//initialize queue and sets currentTask to 0
		if(osCurrentTask == -1)
		{
			for(int i =0; i< threadNums; i++)
				enqueue(i);	
			
			osCurrentTask = 0;
			osThreads[osCurrentTask].status = RUNNING;
			osThreads[osCurrentTask].runTime = 500;
			
			_ICSR |= 1<<28;
			__asm("isb");
			
		}
		//every other time running case
		else{
			
			if (osThreads[osCurrentTask].status != SLEEP){
				osThreads[osCurrentTask].status = READY;
				enqueue(osCurrentTask);
			}
			
			dequeue();
			osCurrentTask = front -> index;
			osThreads[osCurrentTask].status = RUNNING;
			osThreads[osCurrentTask].runTime = 500;
			
		}
}

/*
	Starts the threads if threads have been created. Returns false otherwise
	This function will not return under normal circumstances, since its job is to start the 
	threads, which take over. Therefore, if this function ever returns at all something has gone wrong.
*/
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

/*
	at the moment this just changes the stack from one to the other. I personally found
	this to be easier to do in C. You may want to do more interesting things here.
	For example, when implementing more sophisticated scheduling algorithms, perhaps now is the time
	to figure out which thread should run, and then set PSP to its stack pointer...
*/
int task_switch(void){
		__set_PSP((uint32_t)osThreads[osCurrentTask].taskStack); //set the new PSP
		return 1; //You are free to use this return value in your assembly eventually. It will be placed in r0, so be sure
		//to access it before overwriting r0
}
