/*
    FreeRTOS V8.2.1 - Copyright (C) 2015 Real Time Engineers Ltd.
    All rights reserved

    VISIT http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.

    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation >>!AND MODIFIED BY!<< the FreeRTOS exception.

    ***************************************************************************
    >>!   NOTE: The modification to the GPL is included to allow you to     !<<
    >>!   distribute a combined work that includes FreeRTOS without being   !<<
    >>!   obliged to provide the source code for proprietary components     !<<
    >>!   outside of the FreeRTOS kernel.                                   !<<
    ***************************************************************************

    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  Full license text is available on the following
    link: http://www.freertos.org/a00114.html

    ***************************************************************************
     *                                                                       *
     *    FreeRTOS provides completely free yet professionally developed,    *
     *    robust, strictly quality controlled, supported, and cross          *
     *    platform software that is more than just the market leader, it     *
     *    is the industry's de facto standard.                               *
     *                                                                       *
     *    Help yourself get started quickly while simultaneously helping     *
     *    to support the FreeRTOS project by purchasing a FreeRTOS           *
     *    tutorial book, reference manual, or both:                          *
     *    http://www.FreeRTOS.org/Documentation                              *
     *                                                                       *
    ***************************************************************************

    http://www.FreeRTOS.org/FAQHelp.html - Having a problem?  Start by reading
    the FAQ page "My application does not run, what could be wrong?".  Have you
    defined configASSERT()?

    http://www.FreeRTOS.org/support - In return for receiving this top quality
    embedded software for free we request you assist our global community by
    participating in the support forum.

    http://www.FreeRTOS.org/training - Investing in training allows your team to
    be as productive as possible as early as possible.  Now you can receive
    FreeRTOS training directly from Richard Barry, CEO of Real Time Engineers
    Ltd, and the world's leading authority on the world's leading RTOS.

    http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
    including FreeRTOS+Trace - an indispensable productivity tool, a DOS
    compatible FAT file system, and our tiny thread aware UDP/IP stack.

    http://www.FreeRTOS.org/labs - Where new FreeRTOS products go to incubate.
    Come and try FreeRTOS+TCP, our new open source TCP/IP stack for FreeRTOS.

    http://www.OpenRTOS.com - Real Time Engineers ltd. license FreeRTOS to High
    Integrity Systems ltd. to sell under the OpenRTOS brand.  Low cost OpenRTOS
    licenses offer ticketed support, indemnification and commercial middleware.

    http://www.SafeRTOS.com - High Integrity Systems also provide a safety
    engineered and independently SIL3 certified version for use in safety and
    mission critical applications that require provable dependability.

    1 tab == 4 spaces!
*/


/*
 * Creates all the demo application tasks, then starts the scheduler.  The WEB
 * documentation provides more details of the standard demo application tasks
 * (which just exist to test the kernel port and provide an example of how to use
 * each FreeRTOS API function).
 *
 * In addition to the standard demo tasks, the following tasks and tests are
 * defined and/or created within this file:
 *
 * "Check" hook -  This only executes fully every five seconds from the tick
 * hook.  Its main function is to check that all the standard demo tasks are
 * still operational.  The status can be viewed using on the Task Stats page
 * served by the WEB server.
 *
 * "uIP" task -  This is the task that handles the uIP stack.  All TCP/IP
 * processing is performed in this task.
 *
 * "USB" task - Enumerates the USB device as a CDC class, then echoes back all
 * received characters with a configurable offset (for example, if the offset
 * is 1 and 'A' is received then 'B' will be sent back).  A dumb terminal such
 * as Hyperterminal can be used to talk to the USB task.
 */

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"

/* Demo app includes. */
#include "BlockQ.h"
#include "integer.h"
#include "blocktim.h"
#include "flash.h"
#include "partest.h"
#include "semtest.h"
#include "PollQ.h"
#include "GenQTest.h"
#include "QPeek.h"
#include "recmutex.h"

/*-----------------------------------------------------------*/

/* The time between cycles of the 'check' functionality (defined within the
tick hook). */
#define mainCHECK_DELAY						( ( TickType_t ) 5000 / portTICK_PERIOD_MS )

/* The toggle rate for the LED. */
#define mainLED_TOGGLE_RATE					( ( TickType_t ) 1000 / portTICK_PERIOD_MS )

/* Task priorities. */
#define mainQUEUE_POLL_PRIORITY				( tskIDLE_PRIORITY + 2 )
#define mainSEM_TEST_PRIORITY				( tskIDLE_PRIORITY + 1 )
#define mainBLOCK_Q_PRIORITY				( tskIDLE_PRIORITY + 2 )
#define mainUIP_TASK_PRIORITY				( tskIDLE_PRIORITY + 3 )
#define mainFLASH_TASK_PRIORITY				( tskIDLE_PRIORITY + 2 )
#define mainINTEGER_TASK_PRIORITY           ( tskIDLE_PRIORITY )
#define mainGEN_QUEUE_TASK_PRIORITY			( tskIDLE_PRIORITY )

/* The WEB server has a larger stack as it utilises stack hungry string
handling library calls. */
#define mainBASIC_WEB_STACK_SIZE            ( configMINIMAL_STACK_SIZE * 4 )

/* The message displayed by the WEB server when all tasks are executing
without an error being reported. */
#define mainPASS_STATUS_MESSAGE				"All tasks are executing without error."

/*-----------------------------------------------------------*/

/*
 * Configure the hardware for the demo.
 */
static void prvSetupHardware( void );

/*
 * The task that handles the uIP stack.  All TCP/IP processing is performed in
 * this task.
 */
extern void vuIP_Task( void *pvParameters );

/*
 * The task that handles the USB stack.
 */
extern void vUSBTask( void *pvParameters );

/*
 * Very basic task that does nothing but use delays to flash an LED.
 */
static void prvFlashTask( void *pvParameters );

/*
 * Simply returns the current status message for display on served WEB pages.
 */
char *pcGetTaskStatusMessage( void );

/*-----------------------------------------------------------*/

/* Holds the status message displayed by the WEB server. */
static char *pcStatusMessage = mainPASS_STATUS_MESSAGE;

/*-----------------------------------------------------------*/

int main( void )
{
	/* Configure the hardware for use by this demo. */
	prvSetupHardware();

	/* Start the standard demo tasks.  These are just here to exercise the
	kernel port and provide examples of how the FreeRTOS API can be used. */
	vStartBlockingQueueTasks( mainBLOCK_Q_PRIORITY );
    vCreateBlockTimeTasks();
    vStartSemaphoreTasks( mainSEM_TEST_PRIORITY );
    vStartPolledQueueTasks( mainQUEUE_POLL_PRIORITY );
    vStartIntegerMathTasks( mainINTEGER_TASK_PRIORITY );
    vStartGenericQueueTasks( mainGEN_QUEUE_TASK_PRIORITY );
    vStartQueuePeekTasks();
    vStartRecursiveMutexTasks();

	/* Create the simple LED flash task. */
	xTaskCreate( prvFlashTask, "Flash", configMINIMAL_STACK_SIZE, ( void * ) NULL, mainFLASH_TASK_PRIORITY, NULL );

    /* Create the USB task. */
    xTaskCreate( vUSBTask, "USB", configMINIMAL_STACK_SIZE, ( void * ) NULL, tskIDLE_PRIORITY, NULL );

	/* Create the uIP task.  The WEB server runs in this task. */
    xTaskCreate( vuIP_Task, "uIP", mainBASIC_WEB_STACK_SIZE, ( void * ) NULL, mainUIP_TASK_PRIORITY, NULL );

    /* Start the scheduler. */
	vTaskStartScheduler();

    /* Will only get here if there was insufficient memory to create the idle
    task.  The idle task is created within vTaskStartScheduler(). */
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationTickHook( void )
{
static unsigned long ulTicksSinceLastDisplay = 0;

	/* Called from every tick interrupt as described in the comments at the top
	of this file.

	Have enough ticks passed to make it	time to perform our health status
	check again? */
	ulTicksSinceLastDisplay++;
	if( ulTicksSinceLastDisplay >= mainCHECK_DELAY )
	{
		/* Reset the counter so these checks run again in mainCHECK_DELAY
		ticks time. */
		ulTicksSinceLastDisplay = 0;

		/* Has an error been found in any task? */
		if( xAreGenericQueueTasksStillRunning() != pdTRUE )
		{
			pcStatusMessage = "An error has been detected in the Generic Queue test/demo.";
		}
		else if( xAreQueuePeekTasksStillRunning() != pdTRUE )
		{
			pcStatusMessage = "An error has been detected in the Peek Queue test/demo.";
		}
		else if( xAreBlockingQueuesStillRunning() != pdTRUE )
		{
			pcStatusMessage = "An error has been detected in the Block Queue test/demo.";
		}
		else if( xAreBlockTimeTestTasksStillRunning() != pdTRUE )
		{
			pcStatusMessage = "An error has been detected in the Block Time test/demo.";
		}
	    else if( xAreSemaphoreTasksStillRunning() != pdTRUE )
	    {
	        pcStatusMessage = "An error has been detected in the Semaphore test/demo.";
	    }
	    else if( xArePollingQueuesStillRunning() != pdTRUE )
	    {
	        pcStatusMessage = "An error has been detected in the Poll Queue test/demo.";
	    }
	    else if( xAreIntegerMathsTaskStillRunning() != pdTRUE )
	    {
	        pcStatusMessage = "An error has been detected in the Int Math test/demo.";
	    }
	    else if( xAreRecursiveMutexTasksStillRunning() != pdTRUE )
	    {
	    	pcStatusMessage = "An error has been detected in the Mutex test/demo.";
	    }
	}
}
/*-----------------------------------------------------------*/

static void prvFlashTask( void *pvParameters )
{
TickType_t xLastFlashTime;

	/* We need to initialise xLastFlashTime prior to the first call to
	vTaskDelayUntil(). */
	xLastFlashTime = xTaskGetTickCount();

	for(;;)
	{
		/* Simply toggle the LED between delays. */
		vTaskDelayUntil( &xLastFlashTime, mainLED_TOGGLE_RATE );
		vParTestToggleLED( 0 );
	}
}
/*-----------------------------------------------------------*/

char *pcGetTaskStatusMessage( void )
{
	/* Not bothered about a critical section here. */
	return pcStatusMessage;
}
/*-----------------------------------------------------------*/

void prvSetupHardware( void )
{
	/* Disable peripherals power. */
	SC->PCONP = 0;

	/* Enable GPIO power. */
	SC->PCONP = PCONP_PCGPIO;

	/* Disable TPIU. */
	PINCON->PINSEL10 = 0;

	if ( SC->PLL0STAT & ( 1 << 25 ) )
	{
		/* Enable PLL, disconnected. */
		SC->PLL0CON = 1;
		SC->PLL0FEED = PLLFEED_FEED1;
		SC->PLL0FEED = PLLFEED_FEED2;
	}

	/* Disable PLL, disconnected. */
	SC->PLL0CON = 0;
	SC->PLL0FEED = PLLFEED_FEED1;
	SC->PLL0FEED = PLLFEED_FEED2;

	/* Enable main OSC. */
	SC->SCS |= 0x20;
	while( !( SC->SCS & 0x40 ) );

	/* select main OSC, 12MHz, as the PLL clock source. */
	SC->CLKSRCSEL = 0x1;

	SC->PLL0CFG = 0x20031;
	SC->PLL0FEED = PLLFEED_FEED1;
	SC->PLL0FEED = PLLFEED_FEED2;

	/* Enable PLL, disconnected. */
	SC->PLL0CON = 1;
	SC->PLL0FEED = PLLFEED_FEED1;
	SC->PLL0FEED = PLLFEED_FEED2;

	/* Set clock divider. */
	SC->CCLKCFG = 0x03;

	/* Configure flash accelerator. */
	SC->FLASHCFG = 0x403a;

	/* Check lock bit status. */
	while( ( ( SC->PLL0STAT & ( 1 << 26 ) ) == 0 ) );

	/* Enable and connect. */
	SC->PLL0CON = 3;
	SC->PLL0FEED = PLLFEED_FEED1;
	SC->PLL0FEED = PLLFEED_FEED2;
	while( ( ( SC->PLL0STAT & ( 1 << 25 ) ) == 0 ) );




	/* Configure the clock for the USB. */

	if( SC->PLL1STAT & ( 1 << 9 ) )
	{
		/* Enable PLL, disconnected. */
		SC->PLL1CON = 1;
		SC->PLL1FEED = PLLFEED_FEED1;
		SC->PLL1FEED = PLLFEED_FEED2;
	}

	/* Disable PLL, disconnected. */
	SC->PLL1CON = 0;
	SC->PLL1FEED = PLLFEED_FEED1;
	SC->PLL1FEED = PLLFEED_FEED2;

	SC->PLL1CFG = 0x23;
	SC->PLL1FEED = PLLFEED_FEED1;
	SC->PLL1FEED = PLLFEED_FEED2;

	/* Enable PLL, disconnected. */
	SC->PLL1CON = 1;
	SC->PLL1FEED = PLLFEED_FEED1;
	SC->PLL1FEED = PLLFEED_FEED2;
	while( ( ( SC->PLL1STAT & ( 1 << 10 ) ) == 0 ) );

	/* Enable and connect. */
	SC->PLL1CON = 3;
	SC->PLL1FEED = PLLFEED_FEED1;
	SC->PLL1FEED = PLLFEED_FEED2;
	while( ( ( SC->PLL1STAT & ( 1 << 9 ) ) == 0 ) );

	/*  Setup the peripheral bus to be the same as the CPU output (100 MHz). */
	SC->PCLKSEL0 = 0x05555555;

	/* Configure the LEDs. */
	vParTestInitialise();
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
	/* This function will get called if a task overflows its stack. */

	( void ) pxTask;
	( void ) pcTaskName;

	for( ;; );
}
/*-----------------------------------------------------------*/

void vConfigureTimerForRunTimeStats( void )
{
const unsigned long TCR_COUNT_RESET = 2, CTCR_CTM_TIMER = 0x00, TCR_COUNT_ENABLE = 0x01;

	/* This function configures a timer that is used as the time base when
	collecting run time statistical information - basically the percentage
	of CPU time that each task is utilising.  It is called automatically when
	the scheduler is started (assuming configGENERATE_RUN_TIME_STATS is set
	to 1). */

	/* Power up and feed the timer. */
	SC->PCONP |= 0x02UL;
	SC->PCLKSEL0 = (SC->PCLKSEL0 & (~(0x3<<2))) | (0x01 << 2);

	/* Reset Timer 0 */
	TIM0->TCR = TCR_COUNT_RESET;

	/* Just count up. */
	TIM0->CTCR = CTCR_CTM_TIMER;

	/* Prescale to a frequency that is good enough to get a decent resolution,
	but not too fast so as to overflow all the time. */
	TIM0->PR =  ( configCPU_CLOCK_HZ / 10000UL ) - 1UL;

	/* Start the counter. */
	TIM0->TCR = TCR_COUNT_ENABLE;
}
/*-----------------------------------------------------------*/

