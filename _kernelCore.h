#ifndef _KERNELCORE
#define _KERNELCORE

//these includes give us access to things like uint8_t and NULL
#include <stdint.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <LPC17xx.h>

//The general definitions
#include "osDefs.h"


/*
	Performs various initialization tasks
*/
void kernelInit(void);

/*
	The OS Scheduler
*/
void osSched(void);

/*
	Sets the value of PSP to threadStack and sures that the microcontroller
	is using that value by changing the CONTROL register.
*/

void osYield(void);
void osSleep(void);
void osWakeUp(int index);
void print(char call);

void setThreadingWithPSP(uint32_t* threadStack);

//starts the kernel if threads have been created. Returns false otherwise
bool osKernelStart(void);

//a C function to help us to switch PSP so we don't have to do this in assembly
int task_switch(void);



#endif
