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

/* NOTE:  This is just a test file and not intended to be a generic 
COM driver. */

#include "altera_avalon_uart.h"
#include "altera_avalon_uart_regs.h"
#include "sys/alt_irq.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "system.h"
#include "Serial.h"
/*---------------------------------------------------------------------------*/

#define serINVALID_QUEUE				( ( QueueHandle_t ) 0 )
#define serNO_BLOCK						( ( TickType_t ) 0 )
/*---------------------------------------------------------------------------*/

static QueueHandle_t xRxedChars; 
static QueueHandle_t xCharsForTx; 

alt_u32 uartControl;
/*---------------------------------------------------------------------------*/

static void vUARTInterruptHandler( void* context, alt_u32 id );
static void vUARTReceiveHandler( alt_u32 status );
static void vUARTTransmitHandler( alt_u32 status );
/*---------------------------------------------------------------------------*/

xComPortHandle xSerialPortInitMinimal( unsigned long ulWantedBaud, unsigned portBASE_TYPE uxQueueLength )
{
	/* Create the queues used to hold Rx and Tx characters. */
	xRxedChars = xQueueCreate( uxQueueLength, ( unsigned portBASE_TYPE ) sizeof( signed char ) );
	xCharsForTx = xQueueCreate( uxQueueLength + 1, ( unsigned portBASE_TYPE ) sizeof( signed char ) );

	/* If the queues were created correctly then setup the serial port hardware. */
	if( ( xRxedChars != serINVALID_QUEUE ) && ( xCharsForTx != serINVALID_QUEUE ) )
	{
		portENTER_CRITICAL();
		{
			uartControl = ALTERA_AVALON_UART_CONTROL_RTS_MSK | ALTERA_AVALON_UART_CONTROL_RRDY_MSK | ALTERA_AVALON_UART_CONTROL_DCTS_MSK;
			IOWR_ALTERA_AVALON_UART_CONTROL( UART_BASE, uartControl ); 
		  
		    /* register the interrupt handler */
			alt_irq_register ( UART_IRQ, NULL, vUARTInterruptHandler );
		}
		portEXIT_CRITICAL();
	}
	else
	{
		return ( xComPortHandle ) 0;
	}
    return ( xComPortHandle ) 1;
}
/*---------------------------------------------------------------------------*/

void vSerialClose( xComPortHandle xPort )
{
    /* Never used. */
}
/*---------------------------------------------------------------------------*/

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
		uartControl |= ALTERA_AVALON_UART_CONTROL_RRDY_MSK;
		IOWR_ALTERA_AVALON_UART_CONTROL( UART_BASE, uartControl );
		return pdFALSE;
	}
}
/*---------------------------------------------------------------------------*/

signed portBASE_TYPE xSerialPutChar( xComPortHandle pxPort, signed char cOutChar, TickType_t xBlockTime )
{
signed portBASE_TYPE lReturn = pdPASS;

	/* Place the character in the queue of characters to be transmitted. */
	if( xQueueSend( xCharsForTx, &cOutChar, xBlockTime ) == pdPASS )
	{
        /*Triggers an interrupt on every character or (down) when queue is full. */
        uartControl |= ALTERA_AVALON_UART_CONTROL_TRDY_MSK; 
        IOWR_ALTERA_AVALON_UART_CONTROL( UART_BASE, uartControl );
        lReturn = pdPASS;
    }
    else
    {	
		lReturn = pdFAIL;
	}
	return lReturn;
}
/*---------------------------------------------------------------------------*/

void vSerialPutString( xComPortHandle pxPort, const signed char * const pcString, unsigned short usStringLength )
{
signed char *pxNext;

	/* A couple of parameters that this port does not use. */
	( void ) usStringLength;
	( void ) pxPort;

	/* NOTE: This implementation does not handle the queue being full as no block time is used! */

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

static void vUARTInterruptHandler( void* context, alt_u32 id )
{
	alt_u32 status;

	/* Read the status register in order to determine the cause of the 
    interrupt. */
	status = IORD_ALTERA_AVALON_UART_STATUS( UART_BASE );
	
	/* Clear any error flags set at the device */
	IOWR_ALTERA_AVALON_UART_STATUS( UART_BASE, 0 );
	
	/* process a read irq */
	if ( status & ALTERA_AVALON_UART_STATUS_RRDY_MSK )
	{
		vUARTReceiveHandler( status );
	}
	
	/* process a write irq */
	if ( status & ( ALTERA_AVALON_UART_STATUS_TRDY_MSK  ) )
	{
		vUARTTransmitHandler( status );
	}
}
/*---------------------------------------------------------------------------*/

static void vUARTReceiveHandler( alt_u32 status )
{
signed char cChar;
portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

	/* If there was an error, discard the data */
	if ( status & ( ALTERA_AVALON_UART_STATUS_PE_MSK | ALTERA_AVALON_UART_STATUS_FE_MSK ) )
	{
        asm("break");
		return;
	}

	/* Transfer data from the device to the circular buffer */
	cChar = IORD_ALTERA_AVALON_UART_RXDATA( UART_BASE );
	if ( pdTRUE != xQueueSendFromISR( xRxedChars, &cChar, &xHigherPriorityTaskWoken ) )
	{
		/* If the circular buffer was full, disable interrupts. Interrupts will 
        be re-enabled when data is removed from the buffer. */
		uartControl &= ~ALTERA_AVALON_UART_CONTROL_RRDY_MSK;
		IOWR_ALTERA_AVALON_UART_CONTROL( UART_BASE, uartControl );
	}
    
	portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
}
/*---------------------------------------------------------------------------*/

static void vUARTTransmitHandler( alt_u32 status )
{
signed char cChar;
portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	/* Transfer data if there is some ready to be transferred */
	if( xQueueReceiveFromISR( xCharsForTx, &cChar, &xHigherPriorityTaskWoken ) == pdTRUE )
	{
		IOWR_ALTERA_AVALON_UART_TXDATA( UART_BASE, cChar );
    }
    else
    {
		uartControl &= ~ALTERA_AVALON_UART_CONTROL_TRDY_MSK;
    }
	
	IOWR_ALTERA_AVALON_UART_CONTROL( UART_BASE, uartControl );
    portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );    
}    
/*---------------------------------------------------------------------------*/
