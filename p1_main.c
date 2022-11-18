//This file contains relevant pin and other settings 
#include <LPC17xx.h>

//This file is for printf and other IO functions
#include "stdio.h"

//used for malloc functions
#include "stdlib.h"

//this file sets up the UART
#include "uart.h"

//This file is our threading library
#include "_threadsCore.h"

//Include the kernel
#include "_kernelCore.h"

/*
	Main, or some programmer-defined library, is where the user of your RTOS API 
	creates their threads. I'm going to make three threads and use global variables to indicate
	that they are working.
*/

//code for 3rd test case: 
void idle (void* args)
{
	while(1)
	{
		printf("\nidle");
	}
}


void sleep1 (void* args)
{
	int counter = 0;
	while(1)
	{
		printf("\nsleep1");
		counter++;
		
		if(counter == 100){
			counter = 0;
			osSleep();
		}
	}
}

void sleep2 (void* args)
{
	int counter = 0;
	while(1)
	{
		printf("\nsleep2");
		counter++;
		
		if(counter == 100){
			counter = 0;
			osSleep();
		}
	}
}

void t12 (void* args)
{
	while(1)
	{
		printf("\n200");
		osSleep();
	}
}


//This is C. The expected function heading is int main(void)
int main( void ) 
{
	//Always call this function at the start. It sets up various peripherals, the clock etc. If you don't call this
	//you may see some weird behaviour
	SystemInit();
	
	//Printf now goes to the UART, so be sure to have PuTTY open and connected
	printf("Hello, world!\r\n");
	
	//We need to get our stack location. In Lab Project 2 this will be done 
	//in a Kernel function instead
	uint32_t* initMSP = getMSPInitialLocation();
	
	//Let's see what it is
	printf("MSP Initially: %x\n",(uint32_t)initMSP);
	
	//Initialize the kernel. We'll need to do this every lab project
	kernelInit();
	
	SysTick_Config(SystemCoreClock/1000);
	
	//set up my threads
	osThreadNew(idle, 10000 ,-1 );
	osThreadNew(sleep1, 1000 ,2000 );
	osThreadNew(sleep2, 1000 ,3000  );
	//osThreadNew(t12, 1000/200 ,PERIODIC_SLEEP );
	
	//Now start the kernel, which will run our first thread
	osKernelStart();
	
	//Your code should always terminate in an endless loop if it is done. If you don't
	//the processor will enter a hardfault and will be weird about it the whole time
	while(1);
}
