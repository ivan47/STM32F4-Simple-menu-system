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
 * documentation provides more details of the standard demo application tasks.
 * In addition to the standard demo tasks, the following tasks and tests are
 * defined and/or created within this file:
 *
 * "Fast Interrupt Test" - A high frequency periodic interrupt is generated
 * using a free running timer to demonstrate the use of the
 * configKERNEL_INTERRUPT_PRIORITY configuration constant.  The interrupt
 * service routine measures the number of processor clocks that occur between
 * each interrupt - and in so doing measures the jitter in the interrupt timing.
 * The maximum measured jitter time is latched in the ulMaxJitter variable, and
 * displayed on the OLED display by the 'OLED' task as described below.  The
 * fast interrupt is configured and handled in the timertest.c source file.
 *
 * "OLED" task - the OLED task is a 'gatekeeper' task.  It is the only task that
 * is permitted to access the display directly.  Other tasks wishing to write a
 * message to the OLED send the message on a queue to the OLED task instead of
 * accessing the OLED themselves.  The OLED task just blocks on the queue waiting
 * for messages - waking and displaying the messages as they arrive.
 *
 * "Check" hook -  This only executes every five seconds from the tick hook.
 * Its main function is to check that all the standard demo tasks are still
 * operational.  Should any unexpected behaviour within a demo task be discovered
 * the tick hook will write an error to the OLED (via the OLED task).  If all the
 * demo tasks are executing with their expected behaviour then the check task
 * writes PASS to the OLED (again via the OLED task), as described above.
 *
 * "uIP" task -  This is the task that handles the uIP stack.  All TCP/IP
 * processing is performed in this task.
 */




/*************************************************************************
 * Please ensure to read http://www.freertos.org/portlm3sx965.html
 * which provides information on configuring and running this demo for the
 * various Luminary Micro EKs.
 *************************************************************************/

/* Set the following option to 1 to include the WEB server in the build.  By
default the WEB server is excluded to keep the compiled code size under the 32K
limit imposed by the KickStart version of the IAR compiler.  The graphics
libraries take up a lot of ROM space, hence including the graphics libraries
and the TCP/IP stack together cannot be accommodated with the 32K size limit. */
#define mainINCLUDE_WEB_SERVER		0


/* Standard includes. */
#include <stdio.h>
#include <string.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/* Hardware library includes. */
#include "hw_memmap.h"
#include "hw_types.h"
#include "hw_sysctl.h"
#include "sysctl.h"
#include "gpio.h"
#include "grlib.h"
#include "rit128x96x4.h"
#include "osram128x64x4.h"
#include "formike128x128x16.h"

/* Demo app includes. */
#include "BlockQ.h"
#include "death.h"
#include "integer.h"
#include "blocktim.h"
#include "flash.h"
#include "partest.h"
#include "semtest.h"
#include "PollQ.h"
#include "lcd_message.h"
#include "bitmap.h"
#include "GenQTest.h"
#include "QPeek.h"
#include "recmutex.h"
#include "IntQueue.h"
#include "QueueSet.h"
#include "EventGroupsDemo.h"

/*-----------------------------------------------------------*/

/* The time between cycles of the 'check' functionality (defined within the
tick hook. */
#define mainCHECK_DELAY						( ( TickType_t ) 5000 / portTICK_PERIOD_MS )

/* Size of the stack allocated to the uIP task. */
#define mainBASIC_WEB_STACK_SIZE            ( configMINIMAL_STACK_SIZE * 3 )

/* The OLED task uses the sprintf function so requires a little more stack too. */
#define mainOLED_TASK_STACK_SIZE			( configMINIMAL_STACK_SIZE + 50 )

/* Task priorities. */
#define mainQUEUE_POLL_PRIORITY				( tskIDLE_PRIORITY + 2 )
#define mainCHECK_TASK_PRIORITY				( tskIDLE_PRIORITY + 3 )
#define mainSEM_TEST_PRIORITY				( tskIDLE_PRIORITY + 1 )
#define mainBLOCK_Q_PRIORITY				( tskIDLE_PRIORITY + 2 )
#define mainCREATOR_TASK_PRIORITY           ( tskIDLE_PRIORITY + 3 )
#define mainINTEGER_TASK_PRIORITY           ( tskIDLE_PRIORITY )
#define mainGEN_QUEUE_TASK_PRIORITY			( tskIDLE_PRIORITY )

/* The maximum number of message that can be waiting for display at any one
time. */
#define mainOLED_QUEUE_SIZE					( 3 )

/* Dimensions the buffer into which the jitter time is written. */
#define mainMAX_MSG_LEN						25

/* The period of the system clock in nano seconds.  This is used to calculate
the jitter time in nano seconds. */
#define mainNS_PER_CLOCK					( ( unsigned long ) ( ( 1.0 / ( double ) configCPU_CLOCK_HZ ) * 1000000000.0 ) )

/* Constants used when writing strings to the display. */
#define mainCHARACTER_HEIGHT				( 9 )
#define mainMAX_ROWS_128					( mainCHARACTER_HEIGHT * 14 )
#define mainMAX_ROWS_96						( mainCHARACTER_HEIGHT * 10 )
#define mainMAX_ROWS_64						( mainCHARACTER_HEIGHT * 7 )
#define mainFULL_SCALE						( 15 )
#define ulSSI_FREQUENCY						( 3500000UL )

/*-----------------------------------------------------------*/

/*
 * The task that handles the uIP stack.  All TCP/IP processing is performed in
 * this task.
 */
extern void vuIP_Task( void *pvParameters );

/*
 * The display is written two by more than one task so is controlled by a
 * 'gatekeeper' task.  This is the only task that is actually permitted to
 * access the display directly.  Other tasks wanting to display a message send
 * the message to the gatekeeper.
 */
static void vOLEDTask( void *pvParameters );

/*
 * Configure the hardware for the demo.
 */
static void prvSetupHardware( void );

/*
 * Configures the high frequency timers - those used to measure the timing
 * jitter while the real time kernel is executing.
 */
extern void vSetupHighFrequencyTimer( void );

/*
 * Hook functions that can get called by the kernel.
 */
void vApplicationStackOverflowHook( TaskHandle_t *pxTask, signed char *pcTaskName );
void vApplicationTickHook( void );


/*-----------------------------------------------------------*/

/* The queue used to send messages to the OLED task. */
QueueHandle_t xOLEDQueue;

/* The welcome text. */
const char * const pcWelcomeMessage = "   www.FreeRTOS.org";

/*-----------------------------------------------------------*/


/*************************************************************************
 * Please ensure to read http://www.freertos.org/portlm3sx965.html
 * which provides information on configuring and running this demo for the
 * various Luminary Micro EKs.
 *************************************************************************/
int main( void )
{
	prvSetupHardware();

	/* Create the queue used by the OLED task.  Messages for display on the OLED
	are received via this queue. */
	xOLEDQueue = xQueueCreate( mainOLED_QUEUE_SIZE, sizeof( xOLEDMessage ) );

	/* Start the standard demo tasks. */
    vStartIntegerMathTasks( mainINTEGER_TASK_PRIORITY );
    vStartGenericQueueTasks( mainGEN_QUEUE_TASK_PRIORITY );
    vStartInterruptQueueTasks();
	vStartRecursiveMutexTasks();
	vStartBlockingQueueTasks( mainBLOCK_Q_PRIORITY );
	vCreateBlockTimeTasks();
	vStartSemaphoreTasks( mainSEM_TEST_PRIORITY );
	vStartPolledQueueTasks( mainQUEUE_POLL_PRIORITY );
	vStartQueuePeekTasks();
	vStartQueueSetTasks();
	vStartEventGroupTasks();

	/* Exclude some tasks if using the kickstart version to ensure we stay within
	the 32K code size limit. */
	#if mainINCLUDE_WEB_SERVER != 0
	{
		/* Create the uIP task if running on a processor that includes a MAC and
		PHY. */
		if( SysCtlPeripheralPresent( SYSCTL_PERIPH_ETH ) )
		{
			xTaskCreate( vuIP_Task, "uIP", mainBASIC_WEB_STACK_SIZE, NULL, mainCHECK_TASK_PRIORITY - 1, NULL );
		}
	}
	#endif



	/* Start the tasks defined within this file/specific to this demo. */
	xTaskCreate( vOLEDTask, "OLED", mainOLED_TASK_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL );

	/* The suicide tasks must be created last as they need to know how many
	tasks were running prior to their creation in order to ascertain whether
	or not the correct/expected number of tasks are running at any given time. */
    vCreateSuicidalTasks( mainCREATOR_TASK_PRIORITY );

	/* Configure the high frequency interrupt used to measure the interrupt
	jitter time. */
	vSetupHighFrequencyTimer();

	/* Start the scheduler. */
	vTaskStartScheduler();

    /* Will only get here if there was insufficient memory to create the idle
    task. */
	return 0;
}
/*-----------------------------------------------------------*/

void prvSetupHardware( void )
{
    /* If running on Rev A2 silicon, turn the LDO voltage up to 2.75V.  This is
    a workaround to allow the PLL to operate reliably. */
    if( DEVICE_IS_REVA2 )
    {
        SysCtlLDOSet( SYSCTL_LDO_2_75V );
    }

	/* Set the clocking to run from the PLL at 50 MHz */
	SysCtlClockSet( SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_8MHZ );

	/* 	Enable Port F for Ethernet LEDs
		LED0        Bit 3   Output
		LED1        Bit 2   Output */
	SysCtlPeripheralEnable( SYSCTL_PERIPH_GPIOF );
	GPIODirModeSet( GPIO_PORTF_BASE, (GPIO_PIN_2 | GPIO_PIN_3), GPIO_DIR_MODE_HW );
	GPIOPadConfigSet( GPIO_PORTF_BASE, (GPIO_PIN_2 | GPIO_PIN_3 ), GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD );

	vParTestInitialise();
}
/*-----------------------------------------------------------*/

void vApplicationTickHook( void )
{
static xOLEDMessage xMessage = { "PASS" };
static unsigned long ulTicksSinceLastDisplay = 0;
portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

	/* Called from every tick interrupt.  Have enough ticks passed to make it
	time to perform our health status check again? */
	ulTicksSinceLastDisplay++;
	if( ulTicksSinceLastDisplay >= mainCHECK_DELAY )
	{
		ulTicksSinceLastDisplay = 0;

		/* Has an error been found in any task? */
		if( xAreGenericQueueTasksStillRunning() != pdTRUE )
		{
			xMessage.pcMessage = "ERROR IN GEN Q";
		}
	    else if( xIsCreateTaskStillRunning() != pdTRUE )
	    {
	        xMessage.pcMessage = "ERROR IN CREATE";
	    }
	    else if( xAreIntegerMathsTaskStillRunning() != pdTRUE )
	    {
	        xMessage.pcMessage = "ERROR IN MATH";
	    }
		else if( xAreIntQueueTasksStillRunning() != pdTRUE )
		{
			xMessage.pcMessage = "ERROR IN INT QUEUE";
		}
		else if( xAreBlockingQueuesStillRunning() != pdTRUE )
		{
			xMessage.pcMessage = "ERROR IN BLOCK Q";
		}
		else if( xAreBlockTimeTestTasksStillRunning() != pdTRUE )
		{
			xMessage.pcMessage = "ERROR IN BLOCK TIME";
		}
		else if( xAreSemaphoreTasksStillRunning() != pdTRUE )
		{
			xMessage.pcMessage = "ERROR IN SEMAPHORE";
		}
		else if( xArePollingQueuesStillRunning() != pdTRUE )
		{
			xMessage.pcMessage = "ERROR IN POLL Q";
		}
		else if( xAreQueuePeekTasksStillRunning() != pdTRUE )
		{
			xMessage.pcMessage = "ERROR IN PEEK Q";
		}
		else if( xAreRecursiveMutexTasksStillRunning() != pdTRUE )
		{
			xMessage.pcMessage = "ERROR IN REC MUTEX";
		}
		else if( xAreQueueSetTasksStillRunning() != pdPASS )
		{
			xMessage.pcMessage = "ERROR IN Q SET";
		}
		else if( xAreEventGroupTasksStillRunning() != pdTRUE )
		{
			xMessage.pcMessage = "ERROR IN EVNT GRP";
		}

		configASSERT( strcmp( ( const char * ) xMessage.pcMessage, "PASS" ) == 0 );

		/* Send the message to the OLED gatekeeper for display. */
		xHigherPriorityTaskWoken = pdFALSE;
		xQueueSendFromISR( xOLEDQueue, &xMessage, &xHigherPriorityTaskWoken );
	}

	/* Write to a queue that is in use as part of the queue set demo to
	demonstrate using queue sets from an ISR. */
	vQueueSetAccessQueueSetFromISR();

	/* Call the event group ISR tests. */
	vPeriodicEventGroupsProcessing();
}
/*-----------------------------------------------------------*/

void vOLEDTask( void *pvParameters )
{
xOLEDMessage xMessage;
unsigned long ulY, ulMaxY;
static char cMessage[ mainMAX_MSG_LEN ];
extern volatile unsigned long ulMaxJitter;
const unsigned char *pucImage;

/* Functions to access the OLED.  The one used depends on the dev kit
being used. */
void ( *vOLEDInit )( unsigned long ) = NULL;
void ( *vOLEDStringDraw )( const char *, unsigned long, unsigned long, unsigned char ) = NULL;
void ( *vOLEDImageDraw )( const unsigned char *, unsigned long, unsigned long, unsigned long, unsigned long ) = NULL;
void ( *vOLEDClear )( void ) = NULL;

	/* Map the OLED access functions to the driver functions that are appropriate
	for the evaluation kit being used. */
	switch( HWREG( SYSCTL_DID1 ) & SYSCTL_DID1_PRTNO_MASK )
	{
		case SYSCTL_DID1_PRTNO_6965	:
		case SYSCTL_DID1_PRTNO_2965	:	vOLEDInit = OSRAM128x64x4Init;
										vOLEDStringDraw = OSRAM128x64x4StringDraw;
										vOLEDImageDraw = OSRAM128x64x4ImageDraw;
										vOLEDClear = OSRAM128x64x4Clear;
										ulMaxY = mainMAX_ROWS_64;
										pucImage = pucBasicBitmap;
										break;

		case SYSCTL_DID1_PRTNO_1968	:
		case SYSCTL_DID1_PRTNO_8962 :	vOLEDInit = RIT128x96x4Init;
										vOLEDStringDraw = RIT128x96x4StringDraw;
										vOLEDImageDraw = RIT128x96x4ImageDraw;
										vOLEDClear = RIT128x96x4Clear;
										ulMaxY = mainMAX_ROWS_96;
										pucImage = pucBasicBitmap;
										break;

		default						:	vOLEDInit = vFormike128x128x16Init;
										vOLEDStringDraw = vFormike128x128x16StringDraw;
										vOLEDImageDraw = vFormike128x128x16ImageDraw;
										vOLEDClear = vFormike128x128x16Clear;
										ulMaxY = mainMAX_ROWS_128;
										pucImage = pucGrLibBitmap;
										break;

	}

	ulY = ulMaxY;

	/* Initialise the OLED and display a startup message. */
	vOLEDInit( ulSSI_FREQUENCY );
	vOLEDStringDraw( "POWERED BY FreeRTOS", 0, 0, mainFULL_SCALE );
	vOLEDImageDraw( pucImage, 0, mainCHARACTER_HEIGHT + 1, bmpBITMAP_WIDTH, bmpBITMAP_HEIGHT );

	for( ;; )
	{
		/* Wait for a message to arrive that requires displaying. */
		xQueueReceive( xOLEDQueue, &xMessage, portMAX_DELAY );

		/* Write the message on the next available row. */
		ulY += mainCHARACTER_HEIGHT;
		if( ulY >= ulMaxY )
		{
			ulY = mainCHARACTER_HEIGHT;
			vOLEDClear();
			vOLEDStringDraw( pcWelcomeMessage, 0, 0, mainFULL_SCALE );
		}

		/* Display the message along with the maximum jitter time from the
		high priority time test. */
		sprintf( cMessage, "%s [%uns]", xMessage.pcMessage, ulMaxJitter * mainNS_PER_CLOCK );
		vOLEDStringDraw( cMessage, 0, ulY, mainFULL_SCALE );
	}
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( TaskHandle_t *pxTask, signed char *pcTaskName )
{
	( void ) pxTask;
	( void ) pcTaskName;

	for( ;; );
}
/*-----------------------------------------------------------*/

void vAssertCalled( const char *pcFile, unsigned long ulLine )
{
volatile unsigned long ulSetTo1InDebuggerToExit = 0;

	taskENTER_CRITICAL();
	{
		while( ulSetTo1InDebuggerToExit == 0 )
		{
			/* Nothing do do here.  Set the loop variable to a non zero value in
			the debugger to step out of this function to the point that caused
			the assertion. */
			( void ) pcFile;
			( void ) ulLine;
		}
	}
	taskEXIT_CRITICAL();
}
