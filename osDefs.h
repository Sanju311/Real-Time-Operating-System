#ifndef OS_DEFS
#define OS_DEFS

/*
	Since threading and the kernel are split between multiple files but share common
	definitions, and since C doesn't like to define things twice, I am creating a general
	osDefs header file that contains all of the common definitions
*/

#include <stdint.h>
#include <stddef.h>
#include "stdio.h"
#include "stdlib.h"


//Registers used for interrupts
#define SHPR3 *(uint32_t*)0xE000ED20
#define _ICSR *(uint32_t*)0xE000ED04

//My own stack defines
#define MSR_STACK_SIZE 0x400
#define THREAD_STACK_SIZE 0x200

//Some kernel-specific stuff
#define MAX_THREADS 3 //I am choosing to set this statically

//These are potentially useful constants that can be used when our scheduler is more sophisticated
#define NO_THREADS 0 //no non-idle threads are running, literally do nothing
#define ONE_THREAD 1 //only one non-idle thread is running
#define NEW_THREAD 2
#define NORMAL_THREADING 3

//thread states
#define CREATED 0 //created, but not running
#define RUNNING 1 //running and active
#define READY 2 //not running but ready to go
#define DESTROYED 4 //for use later, especially for threads that end. This indicates that a new thread COULD go here if it needs to
#define SLEEP 3 //when the thread is finished running and needs to be d3elayed before becoming ACTIVE again


#define timer1Index 2
#define timer2Index 1

//The fundamental data structure that is the thread
typedef struct thread{
	void (*threadFunction)(void* args);
	int status;
	uint32_t* taskStack; //stack pointer for this task
	int runTime;
	int sleepTime;
}thread;


typedef struct indexNode // queue to store the index of the thread in the thread array
{
    int index;		//index in array
    struct indexNode *next;
}indexNode;




#endif
