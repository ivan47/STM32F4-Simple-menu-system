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
	BASIC INTERRUPT DRIVEN SERIAL PORT DRIVER FOR UART0.
*/

/* Library includes. */
#include "75x_uart.h"
#include "75x_gpio.h"
#include "75x_eic.h"
#include "75x_mrcc.h"

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "queue.h"

/* Demo application includes. */
#include "serial.h"

#define serINVALID_QUEUE				( ( QueueHandle_t ) 0 )
#define serNO_BLOCK						( ( TickType_t ) 0 )

/*-----------------------------------------------------------*/

/* Queues used to hold received characters, and characters waiting to be
transmitted. */
static QueueHandle_t xRxedChars;
static QueueHandle_t xCharsForTx;

static volatile portBASE_TYPE xQueueEmpty = pdTRUE;

/*-----------------------------------------------------------*/

/* The interrupt service routine - called from the assembly entry point. */
__arm void vSerialISR( void );

/*-----------------------------------------------------------*/

/*
 * See the serial2.h header file.
 */
xComPortHandle xSerialPortInitMinimal( unsigned long ulWantedBaud, unsigned portBASE_TYPE uxQueueLength )
{
xComPortHandle xReturn;
UART_InitTypeDef UART_InitStructure;
GPIO_InitTypeDef GPIO_InitStructure;
EIC_IRQInitTypeDef  EIC_IRQInitStructure;	

	/* Create the queues used to hold Rx and Tx characters. */
	xRxedChars = xQueueCreate( uxQueueLength, ( unsigned portBASE_TYPE ) sizeof( signed char ) );
	xCharsForTx = xQueueCreate( uxQueueLength + 1, ( unsigned portBASE_TYPE ) sizeof( signed char ) );

	/* If the queues were created correctly then setup the serial port
	hardware. */
	if( ( xRxedChars != serINVALID_QUEUE ) && ( xCharsForTx != serINVALID_QUEUE ) )
	{
		portENTER_CRITICAL();
		{
			/* Enable the UART0 Clock. */
			MRCC_PeripheralClockConfig( MRCC_Peripheral_UART0, ENABLE );
			
			/* Configure the UART0_Tx as alternate function */
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
			GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_11;
			GPIO_Init(GPIO0, &GPIO_InitStructure);
			
			/* Configure the UART0_Rx as input floating */
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
			GPIO_Init(GPIO0, &GPIO_InitStructure);
			
			/* Configure UART0. */
			UART_InitStructure.UART_WordLength = UART_WordLength_8D;
			UART_InitStructure.UART_StopBits = UART_StopBits_1;
			UART_InitStructure.UART_Parity = UART_Parity_No;
			UART_InitStructure.UART_BaudRate = ulWantedBaud;
			UART_InitStructure.UART_HardwareFlowControl = UART_HardwareFlowControl_None;
			UART_InitStructure.UART_Mode = UART_Mode_Tx_Rx;
			UART_InitStructure.UART_TxFIFOLevel = UART_FIFOLevel_1_2; /* FIFO size 16 bytes, FIFO level 8 bytes */
			UART_InitStructure.UART_RxFIFOLevel = UART_FIFOLevel_1_2; /* FIFO size 16 bytes, FIFO level 8 bytes */
			UART_Init(UART0, &UART_InitStructure);

			/* Enable the UART0 */
			UART_Cmd(UART0, ENABLE);

			/* Configure the IEC for the UART interrupts. */			
			EIC_IRQInitStructure.EIC_IRQChannelCmd = ENABLE;
			EIC_IRQInitStructure.EIC_IRQChannel = UART0_IRQChannel;
			EIC_IRQInitStructure.EIC_IRQChannelPriority = 1;
			EIC_IRQInit(&EIC_IRQInitStructure);
			
			xQueueEmpty = pdTRUE;
			UART_ITConfig( UART0, UART_IT_Transmit | UART_IT_Receive, ENABLE );
		}
		portEXIT_CRITICAL();
	}
	else
	{
		xReturn = ( xComPortHandle ) 0;
	}

	/* This demo file only supports a single port but we have to return
	something to comply with the standard demo header file. */
	return xReturn;
}
/*-----------------------------------------------------------*/

signed portBASE_TYPE xSerialGetChar( xComPortHandle pxPort, signed char *pcRxedChar, TickType_t xBlockTime )
{
	/* The port handle is not required as this driver only supports one port. */
	( void ) pxPort;

	/* Get the next character from the buffer.  Return false if no characters
	are available, or arrive before xBlockTime expires. */
	if( xQueueReceive( xRxedChars, pcRxedChar, xBlockTime ) )
	{
		return pdTRUE;
	}
	else
	{
		return pdFALSE;
	}
}
/*-----------------------------------------------------------*/

void vSerialPutString( xComPortHandle pxPort, const signed char * const pcString, unsigned short usStringLength )
{
signed char *pxNext;

	/* A couple of parameters that this port does not use. */
	( void ) usStringLength;
	( void ) pxPort;

	/* NOTE: This implementation does not handle the queue being full as no
	block time is used! */

	/* The port handle is not required as this driver only supports UART0. */
	( void ) pxPort;

	/* Send each character in the string, one at a time. */
	pxNext = ( signed char * ) pcString;
	while( *pxNext )
	{
		xSerialPutChar( pxPort, *pxNext, serNO_BLOCK );
		pxNext++;
	}
}
/*-----------------------------------------------------------*/

signed portBASE_TYPE xSerialPutChar( xComPortHandle pxPort, signed char cOutChar, TickType_t xBlockTime )
{
portBASE_TYPE xReturn;

	/* Place the character in the queue of characters to be transmitted. */
	portENTER_CRITICAL();
	{
		if( xQueueEmpty == pdTRUE )
		{
			UART0->DR = cOutChar;
			xReturn = pdPASS;
		}
		else
		{
			if( xQueueSend( xCharsForTx, &cOutChar, xBlockTime ) != pdPASS )
			{
				xReturn = pdFAIL;
			}			
			else
			{
				xReturn = pdPASS;				
			}
		}
		
		xQueueEmpty = pdFALSE;
	}
	portEXIT_CRITICAL();

	return xReturn;
}
/*-----------------------------------------------------------*/

void vSerialClose( xComPortHandle xPort )
{
	/* Not supported as not required by the demo application. */
}
/*-----------------------------------------------------------*/

__arm void vSerialISR( void )
{
signed char cChar;
portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

	do
	{
		if( UART0->MIS & UART_IT_Transmit )
		{
			/* The interrupt was caused by the THR becoming empty.  Are there any
			more characters to transmit? */
			if( xQueueReceiveFromISR( xCharsForTx, &cChar, &xHigherPriorityTaskWoken ) == pdTRUE )
			{
				/* A character was retrieved from the queue so can be sent to the
				THR now. */
				UART0->DR = cChar;
			}
			else
			{
				xQueueEmpty = pdTRUE;		
			}		

			UART_ClearITPendingBit( UART0, UART_IT_Transmit );
		}
	
		if( UART0->MIS & UART_IT_Receive )
		{
			/* The interrupt was caused by a character being received.  Grab the
			character from the RHR and place it in the queue of received
			characters. */
			cChar = UART0->DR;
			xQueueSendFromISR( xRxedChars, &cChar, &xHigherPriorityTaskWoken );
			UART_ClearITPendingBit( UART0, UART_IT_Receive );
		}
	} while( UART0->MIS );

	/* If a task was woken by either a character being received or a character
	being transmitted then we may need to switch to another task. */
	portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
}





	
