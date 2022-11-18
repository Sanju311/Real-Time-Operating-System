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


/*SysTick handler decrements all deadlines and sleepTimers. 
If a sleepTimer runs out it calls the wakeUp function and passes in that thread's index*/
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

//sets psp and calls sched to initalize threads
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

//yield function is called when a thread yields
void osYield(void){
	__ASM("SVC #0");
}

//sleep function is called when a thread needs to go to sleep after yielding
//called for periodic threads & sleep threads
void osSleep(void){
	__ASM("SVC #1");	
}


//change status of thread that woke up and set's it's deadline
//if the newly woken thread has the lowest deadline a context switch happens if not nothing occurs
void osWakeUp(int index){
	osThreads[index].status = READY;
	osThreads[index].deadline = osThreads[index].DEADLINE;
	
	if(osThreads[index].deadline < osThreads[osCurrentTask].deadline)
		__ASM("SVC #2");
	
}

/*
	scheduler determines lowest deadline and sets its status to running 
*/ 
void osSched(void)
{
		//first time running case detemines lowest deadline and context switches to it
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


//has seperate cases if called by osSleep(1) or osYield(0) 
//calls sched to schedule the next thread
//handles all context switches besides the initial one
void SVC_Handler_Main(uint32_t *svc_args)
{
	char call = ((char*)svc_args[6]) [-2];
	
	//osSleep case
	if(call == 1){
		osThreads[osCurrentTask].status = SLEEP;
		osThreads[osCurrentTask].sleepTime = osThreads[osCurrentTask].SLEEPTIME;
	}
	
	//osYield case
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

