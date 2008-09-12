#define USE_FIFO
/*
-------------------------------------------------------------------------------
coreIPM/serial.c

Author: Gokhan Sozmen
-------------------------------------------------------------------------------
Copyright (C) 2007-2008 Gokhan Sozmen
-------------------------------------------------------------------------------
coreIPM is free software; you can redistribute it and/or modify it under the 
terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later 
version.

coreIPM is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with 
coreIPM; if not, write to the Free Software Foundation, Inc., 51 Franklin
Street, Fifth Floor, Boston, MA 02110-1301, USA.
-------------------------------------------------------------------------------
See http://www.coreipm.com for documentation, latest information, licensing, 
support and contact details.
-------------------------------------------------------------------------------
*/


/* Universal Asynchronous Receiver Transmitter 0 & 1 (UART0/1) registers */
/*
UART0 UART1
----- -----
U0RBR U1RBR  Receiver Buffer Register  
U0THR U1THR  Transmit Holding Register  
U0IER U1IER  Interrupt Enable Register 
U0IIR U1IIR  Interrupt Identification Register  
U0FCR U1FCR  FIFO Control Register 
U0LCR U1LCR  Line Control Register
U0MCR U1MCR  Modem control reg 
U0LSR U1LSR  Line Status Register
U0MSR U1MSR  Modem status reg  
U0SCR U1SCR  Scratch pad register 
U0DLL U1DLL  Divisor Latch Registers (LSB) 
U0DLM U1DLM  Divisor Latch Registers (MSB) 
U0ACR U1ACR  Auto-baud Control Register
U0FDR U1FDR  Fractional Divider Register
U0TER U1TER  Transmit Enable Register
*/

/* 
GENERAL OPERATION
-----------------
The UART0 receiver block, U0RX, monitors the serial input line, RXD0, for valid
input. The UART0 RX Shift Register (U0RSR) accepts valid characters via RXD0. 

After a valid character is assembled in the U0RSR, it is passed to the UART0 
RX Buffer Register FIFO to await access by the CPU or host via the generic 
host interface.

The UART0 transmitter block, U0TX, accepts data written by the CPU or host and
buffers the data in the UART0 TX Holding Register FIFO (U0THR). The UART0 TX 
Shift Register (U0TSR) reads the data stored in the U0THR and assembles the data
to transmit via the serial output pin, TXD0.

The UARTn Baud Rate Generator block, U0BRG, generates the timing enables used 
by the UART0 TX block. The U0BRG clock input source is the VPB clock (PCLK). 
The main clock is divided down per the divisor specified in the U0DLL and U0DLM
registers. This divided down clock is a 16x oversample clock, NBAUDOUT.

The interrupt interface contains registers U0IER and U0IIR. The interrupt interface
receives several one clock wide enables from the U0TX and U0RX blocks.
Status information from the U0TX and U0RX is stored in the U0LSR. Control information
for the U0TX and U0RX is stored in the U0LCR.

The modem interface contains registers U1MCR and U1MSR. This interface is
responsible for handshaking between a modem peripheral and the UART1.
*/
#include <stdio.h>
#include <string.h>
#include "arch.h"
#include "ipmi.h"
#include "ws.h"
#include "serial.h"
#include "i2c.h"
#include "strings.h"
#include "debug.h"
#include "gpio.h"
#include "error.h"
#include "module.h"

#define uchar unsigned char

#define CR     0x0D
#define LF     0x0A
#define CLI_PROMPT "BMC>"

int buf_index_1 = 0;
int data_ready_1 = 0;
int buf_index_0 = 0;
int data_ready_0 = 0;

typedef struct port_info {
	uchar port_name;  // UART_DEBUG or UART_ITLA which map to UART0 or UART1
	uchar filter_type;
	void(*callback_fn)( uchar * );
	uchar buf[80]; 
} PORT_INFO;

PORT_INFO serial_port[2];
uchar flash_buf[256] = { 0 } ;
int xfp_addr = 0;


/*==============================================================*/
/* Local Function Prototypes					*/
/*==============================================================*/
/* UART ISR */
#if defined (__CA__) || defined (__CC_ARM)
void UART_ISR_0( void ) __irq;
void UART_ISR_1( void ) __irq;
#elif defined (__GNUC__)
void UART_ISR_0(void) __attribute__ ((interrupt));
void UART_ISR_1(void) __attribute__ ((interrupt));
#endif

void term_process( uchar *buf );
int putchar_0( int ch );
int putchar_1( int ch );

void serial_dbg_port_msg_send( unsigned char *buf ); 

/*
 
+-----------------------------------------------------------------------------+
UART0 Receiver Buffer Register (U0RBR, when DLAB = 0, Read Only)
+-----------------------------------------------------------------------------+
The U0RBR is the top byte of the UART0 Rx FIFO. The top byte of the Rx FIFO contains
the oldest character received and can be read via the bus interface. The LSB (bit 0)
represents the “oldest” received data bit. If the character received is less than 8 bits, the
unused MSBs are padded with zeroes.

The Divisor Latch Access Bit (DLAB) in U0LCR must be zero in order to access the
U0RBR. The U0RBR is always Read Only.

Since PE, FE and BI bits correspond to the byte sitting on the top of the RBR FIFO (i.e.
the one that will be read in the next read from the RBR), the right approach for fetching the
valid pair of received byte and its status bits is first to read the content of the U0LSR
register, and then to read a byte from the U0RBR.
+-----------------------------------------------------------------------------+


+-----------------------------------------------------------------------------+
Interrupt Identification Register (U0IIR, Read Only)
+-----------------------------------------------------------------------------+
Provides a status code that denotes the priority and source of a pending 
interrupt. The interrupts are frozen during an U0IIR access. If an interrupt
occurs during an U0IIR access, the interrupt is recorded for the next U0IIR
access.

Bit	Meaning
[0]	0 At least one interrupt is pending.
	1 No pending interrupts.

	The pending interrupt can be determined by evaluating U0IIR[3:1].

[3:1] 	Interrupt Identification

	U0IER[3:1] identifies an interrupt corresponding to the
	UART0 Rx FIFO. All other combinations of U0IER[3:1] not
	listed above are reserved (000,100,101,111).

	011 - Receive Line Status (RLS).
	010 2a - Receive Data Available (RDA).
	110 2b - Character Time-out Indicator (CTI).
	001 3 - THRE Interrupt

[5:4]	Reserved.

[7:6]	FIFO Enable These bits are equivalent to U0FCR[0].
	TODO: Does this indicate whether the FIFO is enabled or not ?

[8]	ABEOInt End of auto-baud interrupt. True if auto-baud has finished
	successfully and interrupt is enabled.

[9]	ABTOInt Auto-baud time-out interrupt. True if auto-baud has timed
	out and interrupt is enabled.

[31:10]	Reserved.

Interrupts are handled as described in Table 105. Given the status of U0IIR[3:0], an
interrupt handler routine can determine the cause of the interrupt and how to clear the
active interrupt. The U0IIR must be read in order to clear the interrupt prior to exiting 
the Interrupt Service Routine.

RLS interrupt (U0IIR[3:1] = 011)
--------------------------------
The UART0 RLS interrupt (U0IIR[3:1] = 011) is the highest priority interrupt and is set
whenever any one of four error conditions occur on the UART0 Rx input: overrun error
(OE), parity error (PE), framing error (FE) and break interrupt (BI). The UART0 Rx error
condition that set the interrupt can be observed via U0LSR[4:1]. The interrupt is cleared
upon an U0LSR read.
Action: --> check Line Status Register (U0LSR)

RDA interrupt (U0IIR[3:1] = 010)
--------------------------------
The UART0 RDA interrupt (U0IIR[3:1] = 010) shares the second level priority with the CTI
interrupt (U0IIR[3:1] = 110). The RDA is activated when the UART0 Rx FIFO reaches the
trigger level defined in U0FCR[7:6] and is reset when the UART0 Rx FIFO depth falls
below the trigger level. When the RDA interrupt goes active, the CPU can read a block of
data defined by the trigger level.

CTI interrupt (U0IIR[3:1] = 110)
--------------------------------
The CTI interrupt (U0IIR[3:1] = 110) is a second level interrupt and is set when the UART0
Rx FIFO contains at least one character and no UART0 Rx FIFO activity has occurred in
3.5 to 4.5 character times. Any UART0 Rx FIFO activity (read or write of UART0 RSR) will
clear the interrupt. This interrupt is intended to flush the UART0 RBR after a message has
been received that is not a multiple of the trigger level size. For example, if a peripheral
wished to send a 105 character message and the trigger level was 10 characters, the CPU
would receive 10 RDA interrupts resulting in the transfer of 100 characters and 1 to 5 CTI
interrupts (depending on the service routine) resulting in the transfer of the remaining 5
characters.

THRE interrupt (U0IIR[3:1] = 001)
---------------------------------
The UART0 THRE interrupt (U0IIR[3:1] = 001) is a third level interrupt and is activated
when the UART0 THR FIFO is empty provided certain initialization conditions have been
met. These initialization conditions are intended to give the UART0 THR FIFO a chance to
fill up with data to eliminate many THRE interrupts from occurring at system start-up. The
initialization conditions implement a one character delay minus the stop bit whenever
THRE=1 and there have not been at least two characters in the U0THR at one time since
the last THRE = 1 event. This delay is provided to give the CPU time to write data to
U0THR without a THRE interrupt to decode and service. A THRE interrupt is set
immediately if the UART0 THR FIFO has held two or more characters at one time and
currently, the U0THR is empty. The THRE interrupt is reset when a U0THR write occurs or
a read of the U0IIR occurs and the THRE is the highest interrupt (U0IIR[3:1] = 001).

U0IIR Priority  Interrupt Type 	      Interrupt Source 	       Interrupt Reset
[3:0]
--------------------------------------------------------------------------------
0001  - 	None 		      None			-
0110  Highest	RX Line Status/Error  OE or PE or FE or BI     U0LSR Read

0100  Second 	RX Data Available     Rx data available or     U0RBR Read or
				      trigger level reached    UART0 FIFO drops
				      in FIFO (U0FCR0=1)       below trigger level

1100  Second	Character Time-out    Minimum of one character U0RBR Read[3]
		indication	      in the Rx FIFO and no 
				      character input or 
				      removed during a time 
				      period depending on how
				      many characters are in 
				      FIFO and what the trigger
				      level is set at (3.5 to 4.5
				      character times).
				      The exact time will be:
				      [(word length) × 7 - 2] × 8 + 
				      [(trigger level - number of 
				      characters) × 8 + 1] RCLKs

0010 	Third 	THRE 		      THRE[2]		       U0IIR Read (if source
	       						       of interrupt) or THR 
							       write
							       
0000	Fourth 	Modem Status 	      CTS or DSR or RI or DCD  MSR Read
        (UART1 only)
+-----------------------------------------------------------------------------+



+-----------------------------------------------------------------------------+
Line Status Register (U0LSR, Read Only)
+-----------------------------------------------------------------------------+
The U0LSR is a read-only register that provides status information on the UART0 TX and
RX blocks.

[0] Receiver Data Ready (RDR) 0
U0LSR0 is set when the U0RBR holds an unread character and is cleared
when the UART0 RBR FIFO is empty.
0 U0RBR is empty.
1 U0RBR contains valid data.

[1] Overrun Error (OE)
The overrun error condition is set as soon as it occurs. An U0LSR read clears
U0LSR1. U0LSR1 is set when UART0 RSR has a new character assembled
and the UART0 RBR FIFO is full. In this case, the UART0 RBR FIFO will not
be overwritten and the character in the UART0 RSR will be lost.
0 Overrun error status is inactive.
1 Overrun error status is active.

[2] Parity Error (PE)
When the parity bit of a received character is in the wrong state, a parity error
occurs. An U0LSR read clears U0LSR[2]. Time of parity error detection is
dependent on U0FCR[0].
Note: A parity error is associated with the character at the top of the UART0
RBR FIFO.
0 Parity error status is inactive.
1 Parity error status is active.

[3] Framing Error (FE)
When the stop bit of a received character is a logic 0, a framing error occurs.
An U0LSR read clears U0LSR[3]. The time of the framing error detection is
dependent on U0FCR0. Upon detection of a framing error, the Rx will attempt
to resynchronize to the data and assume that the bad stop bit is actually an
early start bit. However, it cannot be assumed that the next received byte will
be correct even if there is no Framing Error.
Note: A framing error is associated with the character at the top of the UART0
RBR FIFO.
0 Framing error status is inactive.
1 Framing error status is active.

[4] Break Interrupt (BI)
When RXD0 is held in the spacing state (all 0’s) for one full character
transmission (start, data, parity, stop), a break interrupt occurs. Once the
break condition has been detected, the receiver goes idle until RXD0 goes to
marking state (all 1’s). An U0LSR read clears this status bit. The time of break
detection is dependent on U0FCR[0].
Note: The break interrupt is associated with the character at the top of the
UART0 RBR FIFO.
0 Break interrupt status is inactive.
1 Break interrupt status is active.

[5] Transmitter Holding Register Empty (THRE))
THRE is set immediately upon detection of an empty UART0 THR and is
cleared on a U0THR write.
0 U0THR contains valid data.
1 U0THR is empty.

[6] Transmitter Empty (TEMT) 
TEMT is set when both U0THR and U0TSR are empty; TEMT is cleared when
either the U0TSR or the U0THR contain valid data.
0 U0THR and/or the U0TSR contains valid data.
1 U0THR and the U0TSR are empty.

[7] Error in RX FIFO (RXFE)
U0LSR[7] is set when a character with a Rx error such as framing error, parity
error or break interrupt, is loaded into the U0RBR. This bit is cleared when the
U0LSR register is read and there are no subsequent errors in the UART0
FIFO.
0 U0RBR contains no UART0 RX errors or U0FCR[0]=0.
1 UART0 RBR contains at least one UART0 RX error.
+-----------------------------------------------------------------------------+
*/

#define UARTINT_MODEM		0x00	// Modem interrupt
#define UARTINT_ERROR		0x06	// RX Line Status/Error
#define UARTINT_RX_DATA_AVAIL	0x04	// Rx data avail or trig level in FIFO
#define UARTINT_CHAR_TIMEOUT	0x0C	// Character Time-out indication	      
#define UARTINT_THRE		0x02	// Transmit Holding Register Empty

#define UARTINT_ENABLE_RX_DATA	0x001	// Enable the RDA interrupts.
#define UARTINT_ENABLE_THRE	0x002	// Enable the THRE interrupts.
#define UARTINT_ENABLE_ERROR	0x004	// Enable the RX line status interrupts.
#define UARTINT_ENABLE_MODEM	0x008	// Enable the modem interrupt.
#define UARTINT_ENABLE_CTS	0x080	// Enable the CTS interrupt.
#define UARTINT_ENABLE_AUTO_BAUD	0x100	// Enable Auto-baud Time-out Interrupt.
#define UARTINT_ENABLE_END_AUTO_BAUD	0x200	// Enable End of Auto-baud Interrupt.

#define UART_LSR_RX_DATA_RDY		0x01
#define UART_LSR_OVERRUN_ERROR		0x02
#define UART_LSR_PARITY_ERROR		0x04
#define UART_LSR_FRAMING_ERROR		0x08
#define UART_LSR_BREAK_INTERRUPT	0x10
#define UART_LSR_TX_HOLDING_REG_EMPTY	0x20
#define UART_LSR_TX_EMPTY		0x40
#define UART_LSR_RX_FIFO_ERROR		0x80

#define UART_FCR_FIFO_ENABLE	0x01
#define UART_FCR_FIFO_1_CHAR	0x00
#define UART_FCR_FIFO_4_CHAR	0x40
#define UART_FCR_FIFO_8_CHAR	0x80
#define UART_FCR_FIFO_14_CHAR	0xC0

/* TODO: change getchar, putchar to use interrupt versions instead of polling */
/* TODO use a pool of buffers instead of line_buf_1 etc */


/*==============================================================
 * uart_initialize()
 *==============================================================*/
void 
uart_initialize( void ) 
{
	/* Initialize VIC */
	/* Interrupt Select register (VICIntSelect) is a read/write accessible 
	 * register. This register classifies each of the 32 interrupt requests
	 * as contributing to FIQ or IRQ.
	 * 0 The interrupt request with this bit number is assigned to the IRQ category.
	 * 1 The interrupt request with this bit number is assigned to the FIQ category. */
	VICIntSelect = 0x0; /* assign all interrupt reqs to the IRQ category */
	
	/* Symbol  Function  PINSEL0 bit
	   P0.0  = TxD0      [1:0]
	   P0.1  = RxD0      [3:2] */
	PINSEL0 |= 0x00000005;	/* Enable RxD0 and TxD0 */
	U0LCR = 0x83;		/* 8 bits, no Parity, 1 Stop bit */
	U0DLL = 98;		/* 9600 Baud Rate @ 12MHz VPB Clock */
	U0LCR = 0x03;		/* Disable access to Divisor Latches */
	U0IER = UARTINT_ENABLE_RX_DATA;   /* Enable the RDA interrupts */
#ifdef USE_FIFO
	U0FCR = UART_FCR_FIFO_ENABLE | UART_FCR_FIFO_4_CHAR;
#endif
	VICVectAddr5 = ( unsigned long )UART_ISR_0;	/* set interrupt vector in 5 */
	VICVectCntl5 = 0x20 | IS_UART0;			/* use it for UART0 interrupt */
	VICIntEnable = IER_UART0;			/* enable UART1 interrupt */
	   
	/* P0.8  = TxD1      [17:16]
	   P0.9  = RxD1      [19:18]
	   P0.10 = RTS1      [21:20] (NC on connector, avail on board)
	   P0.11 = CTS1      [23:22] (NC on connector, avail on board) */
	PINSEL0 |= 0x00050000;	/* Enable RxD1 and TxD1 */
	U1LCR = 0x83;		/* 8 bits, no Parity, 1 Stop bit */
	U1DLL = 98;		/* 9600 Baud Rate @ 12MHz VPB Clock */
	U1LCR = 0x03;		/* Disable access to Divisor Latches */
	U1IER = UARTINT_ENABLE_RX_DATA;   /* Enable the RDA interrupts */
#ifdef USE_FIFO
	U1FCR = UART_FCR_FIFO_ENABLE | UART_FCR_FIFO_4_CHAR;
#endif
	VICVectAddr4 = ( unsigned long )UART_ISR_1;	/* set interrupt vector in 4 */
	VICVectCntl4 = 0x20 | IS_UART1;			/* use it for UART1 interrupt */
	VICIntEnable = IER_UART1;			/* enable UART1 interrupt */

	serial_port[0].port_name = UART_DEBUG; 
	serial_port[0].filter_type = UART_FILTER_TERM;
	serial_port[0].callback_fn = term_process;

	serial_port[1].port_name = UART_ITLA; 
	serial_port[1].filter_type = UART_FILTER_RAW;
	serial_port[1].callback_fn = 0;

}


#if defined (__CA__) || defined (__CC_ARM)
void UART_ISR_1( void ) __irq 
#elif defined (__GNUC__)
void UART_ISR_1( void )
#endif
{
	uchar int_reg;
	uchar rx_char;
	unsigned char temp;
	
	int_reg = U1IIR;	// read U1IIR to clear UART interrupt flag

	switch( int_reg & 0x0f )
	{
		case UARTINT_ERROR:		// TODO: error handling
	  		temp = U1LSR;		// clear interrupt
	   		break;							
		case UARTINT_RX_DATA_AVAIL:	// Rx char available
		case UARTINT_CHAR_TIMEOUT:	// Character Time-out indication
#ifdef USE_FIFO
		    while( U1LSR & UART_LSR_RX_DATA_RDY ) 
#endif
		    {
			rx_char = U1RBR;	// get char and reset int
			
			serial_port[1].buf[buf_index_1++] = rx_char;
			if( serial_port[1].filter_type == UART_FILTER_RAW ) {
				if( buf_index_1 >= 32 ) {
					data_ready_1 = 1;
					buf_index_1 = 0;
				}
			} else {
				U1THR = rx_char;	// echo character
				if (rx_char == CR) {
					U1THR = LF;	// echo character
					serial_port[1].buf[buf_index_1] = 0; // end of string
					data_ready_1 = 1;
					buf_index_1 = 0;
				}
			}
		    }
			break;
		case UARTINT_THRE:		// Transmit Holding Register Empty
			break;			// U1IIR Read has reset interrupt
		case UARTINT_MODEM:
			temp = U1MSR;		// clear interrupt
			break;
	} 
	
	VICVectAddr = 0;          		// Acknowledge Interrupt
}

#if defined (__CA__) || defined (__CC_ARM)
void UART_ISR_0( void ) __irq 
#elif defined (__GNUC__)
void UART_ISR_0( void )
#endif
{
	uchar int_reg;
	uchar rx_char;
	unsigned char temp;
	
	int_reg = U0IIR;	// read U0IIR to clear UART interrupt flag

	switch( int_reg & 0x0f)
	{
		case UARTINT_ERROR:		// TODO: error handling
	  		temp = U0LSR;		// clear interrupt
	   		break;							
		case UARTINT_RX_DATA_AVAIL:	// Rx char available
		case UARTINT_CHAR_TIMEOUT:	// Character Time-out indication
#ifdef USE_FIFO
		    while( U0LSR & UART_LSR_RX_DATA_RDY ) 
#endif
		    {
			rx_char = U0RBR;	// get char and reset int
						
			serial_port[0].buf[buf_index_0++] = rx_char;
			if( serial_port[0].filter_type == UART_FILTER_RAW ) {
				if( buf_index_0 >= 32 ) {
					data_ready_0 = 1;
					buf_index_0 = 0;
				}
			} else {
				U0THR = rx_char;	// echo character
				if (rx_char == CR) {
					U0THR = LF;	// echo character
					serial_port[0].buf[buf_index_0] = 0; // end of string
					data_ready_0 = 1;
					buf_index_0 = 0;
				}
			}
		    }
			break;
		case UARTINT_THRE:		// Transmit Holding Register Empty
			break;			// U1IIR Read has reset interrupt
		case UARTINT_MODEM:
			temp = U0MSR;		// clear interrupt
			break;
	} 
	
	VICVectAddr = 0;          		// Acknowledge Interrupt
}


// Simple polled routines
/*
putchar()

Parameters
	ch - Character to be written. The character is passed as its int promotion. 

Return Value
	If there are no errors, the same character that has been written is returned.
	If an error occurs, EOF is returned.
*/
 
int 
#if defined (__CA__) || defined ( __GNUC__ )
putchar( int ch )	// Write character to the debug serial port 
#elif defined (__CC_ARM)  
sendchar( int ch )	// Write character to the debug serial port 
#endif	
{
	switch( UART_DEBUG ) {
		case UART_0:
			return ( putchar_0( ch ) );
		case UART_1:
			return ( putchar_1( ch ) );
	}
	return EOF;
}

/*
putc()

Writes a character to the stream.

If there are no errors, the same character that has been written is returned.
If an error occurs, EOF is returned and the error indicator is set.
*/
/*
int
putc( int ch, int handle ) 
{
	if ( !(( handle !=  0) || ( handle != 1 ) ) ) {
		return( EOF );
	}

	switch( serial_port[handle].port_name ) {
		case UART_0:
			putchar_0( ch );
			break;
		case UART_1:
			putchar_1( ch );
			break;
		default:
			return( EOF );
			break;
	}
	return( ch );		
}
*/

/*
fputs()

On success, a non-negative value is returned.
On error, the function returns EOF.
*/
/*
int 
fputs ( const char * str, int handle )
{
	int i, len, ret;
	
	len = strlen( str );
	for( i = 0; i < len; i++ ) {
		ret = putc( str[i], handle );
		if( ret != 1 )
			return( EOF );
	}
	return( ESUCCESS );
}
*/

/*
fflush()

On success, a non-negative value is returned.
On error, the function returns EOF.
*/
/*
int
fflush( int handle )
{
	switch( handle ) {
		case 0:
			buf_index_0 = 0;
			break;
		case 1:
			buf_index_1 = 0;
			break;
		default:
			return( EOF );			
	}
	return( ESUCCESS );
}	
*/

int 
putchar_1( int ch )	// Write character to Serial Port 1   
{
	if ( ch == '\n' )  {
		while ( !( U1LSR & 0x20 ) );	// spin while Transmitter Holding Register not Empty
		U1THR = CR;			// output CR 
	}

	while ( !( U1LSR & 0x20 ) );

	return ( U1THR = ch );
}

int 
putchar_0( int ch )	// Write character to Serial Port 0  
{
	if ( ch == '\n' )  {
		while ( !( U0LSR & 0x20 ) );	// spin while Transmitter Holding Register not Empty
		U0THR = CR;			// output CR 
	}

	while ( !( U0LSR & 0x20 ) );

	return ( U0THR = ch );
}
/*
int 
getchar( void )		// Read character from the debug sSerial port 
{
	switch( UART_DEBUG ) {
		case UART_0:
			return ( getchar_0() );
		case UART_1:
			return ( getchar_1() );
	}

}
*/
int 
getchar_1( void )		// Read character from Serial Port 
{	
	while ( !( U1LSR & 0x01 ) );	// wait until RBR contains valid data

	return (U1RBR);
}

int 
getchar_0( void )		// Read character from Serial Port 
{	
	while ( !( U0LSR & 0x01 ) );	// wait until RBR contains valid data

	return (U0RBR);
}
/*
Terminal mode messages are of the general format:
[<message data>]<newline>
The left-bracket and right-bracket+<newline> characters serve as START and STOP delimiters for the
message. Note that the right-bracket and <newline> characters together form the sequence that indicates the end
of the message. <newline> characters may appear within the message as a result of input line editing and multiline
output message data.
IPMI Messages are sent and received in Terminal Mode <message data> as a series of case-insensitive hex-
ASCII pairs, where each is optionally separated from the preceding pair by a single <space> character.
The Terminal Mode Request Message field definitions follow those used for the Basic Mode except that there
is no Slave address / Software ID field or LUN information for the requester. The software ID and LUN for the
remote console are fixed and implied by the command. The SWID for messages to the remote console is always
40h, and the LUN is 00b.
Instead, there is a ‘bridge’ field that is used to identify whether the message should be routed to the BMC’s
bridged message tracking functionality or not.
messages to and from the system interface are transferred using the BMC SMS LUN,
10b, with the bridge field set to 00b.
See Table 14-12, Terminal Mode Message Bridge Field*/
/* buf is a null terminated string */
/* this function processes the debug/terminal port serial data */
void
term_process( uchar *buf )
{
	uchar *ptr = buf;
	int nibble[2];
	int nibble_count;
	int val, ret;
	int count = 0;
	int buf_len;
	char *cmd_str;
	unsigned power_state;
	uchar out_buf[32]; // TODO make this bigger ?
	int port_handle;
	int i;
	IPMI_TERMINAL_MODE_HDR tm_hdr = { 0, 0, 0 };
	unsigned char *req_ptr = (unsigned char *)&tm_hdr;
	IPMI_WS *ws;

	/* first character must be '[' */
	if( strncmp( ( const char * )ptr++, "[", 1 ) ) {
		putstr( "[ERR]\n" );
		return;
	}

	/* if not prefixed by SYS, check if this is an ipmi message */
	if( strncmp( ( const char * )ptr, "SYS", 3 ) && strncmp( ( const char * )ptr, "sys", 3 ) ) 
		goto message_process;

	if( ptr = ( unsigned char * )strchr( ( const char * )ptr, ' ' ) ) {
		ptr++;
	}
	else {
		putstr( "[ERR]\n" );
		return;
	}

	if( ( strncmp( ( const char * )ptr, "POWER", 5 ) == 0 ) 
			|| ( strncmp( ( const char * )ptr, "power", 5 ) == 0 ) ) {

		if( ptr = ( unsigned char * )strchr( ( const char * )ptr, ' ' ) ) {
			ptr++;
		}
		else
			return;

		if( ( strncmp( ( const char * )ptr, "OFF]", 4 ) == 0 ) 
				|| ( strncmp( ( const char * )ptr, "off]", 4 ) == 0 ) ) {
			putstr( "[OK]\n" );
			gpio_power_off();
		} else if( ( strncmp( ( const char * )ptr, "ON]", 3 ) == 0 ) 
				|| (strncmp( ( const char * )ptr, "on]", 3 ) == 0) ) {
			putstr( "[OK]\n" );
			gpio_power_on();
		} else {
			putstr( "[ERR]\n" );
		}
		return;
	}

	if( ( strncmp( ( const char * )ptr, "TMODE]", 6 ) == 0 ) 
			|| ( strncmp( ptr, "tmode]", 6 ) == 0 ) ) {
		putstr( "[OK TMODE]\n" );
		return;
	}

	if( ( strncmp( ( const char * )ptr, "RESET]", 6 ) == 0 ) 
			|| ( strncmp( ptr, "reset]", 6 ) == 0 ) ) {
		putstr( "[OK]\n" );
		return;
	}

	if( ( strncmp( ( const char * )ptr, "IDENTIFY]", 9 ) == 0 ) 
			|| ( strncmp( ( const char * )ptr, "identify]", 9 ) == 0 ) ) {
		gpio_led_blink( GPIO_IDENTIFY_LED, 5, 5, 0 );
		putstr( "[OK]\n" );
		return;
	}

	if( ( strncmp( ( const char * )ptr, "HEALTH QUERY]", 13 ) == 0 ) 
			|| ( strncmp( ( const char * )ptr, "health query]", 13 ) == 0 ) ) {

		/*
		Return a high level version of the system health status in ‘terse’
		format. The BMC returns a string with the following format if 
		command is accepted.
		
		PWR:zzz H:xx T:xx V:xx PS:xx C:xx D:xx S:xx O:xx

		Where:
			PWR is system POWER state
			H is overall Health
			T is Temperature
			V is Voltage
			PS is Power Supply subsystem
			F is cooling subsystem (Fans)
			D is Hard Drive / RAID Subsystem
			S is physical Security
			O is Other (OEM)

			zzz is: “ON”, 
				“OFF” (soft-off or mechanical off), 
				“SLP” (sleep - used when can’t distinguish sleep level), 
				“S4”, 
				“S3”, 
				“S2”, 
				“S1”, 
				“??” (unknown)

			and xx is: ok, nc, cr, nr, uf, or ?? where:

			“ok” = OK (monitored parameters within normal operating ranges)
			“nc” = non-critical (‘warning’: hardware outside normal operating range)
			“cr” = critical (‘fatal’ :hardware exceeding specified ratings)
			“nr” = non-recoverable (‘potential damage’: system hardware in jeopardy or damaged)
			“uf” = unspecified fault (fault detected, but severity unspecified)
			“??” = status not available/unknown (typically because system power is OFF)
		*/

		unsigned power_state = gpio_get_power_state();

		switch( power_state ) {
			case POWER_STATE_ON:
				putstr( "[OK PWR:ON " );
				break;
			case POWER_STATE_OFF:
				putstr( "[OK PWR:OFF " );
				break;
			case POWER_STATE_SLP:
				putstr( "[OK PWR:SLP " );
				break;
			case POWER_STATE_S1:
				putstr( "[OK PWR:S1 " );
				break;
			case POWER_STATE_S2:
				putstr( "[OK PWR:S2 " );
				break;
			case POWER_STATE_S3:
				putstr( "[OK PWR:S3 " );
				break;
			case POWER_STATE_S4:
				putstr( "[OK PWR:S4 " );
				break;
			default:
				putstr( "[OK PWR:?? " );
				break;
		}
		
		/* the rest can be filled once we determine which parameters to monitor */		
		putstr( "H:?? T:?? V:?? PS:?? C:?? D:?? S:?? O:??]\n" );
		return;
	}
	
	/* perform any module specific processing */
	module_term_process( ptr );
	
	putstr( "[ERR]\n" );
	return;

message_process:
	putstr( "Processing Message\n" );
	nibble_count = 0;
	buf_len = strlen( ( const char * )buf );
	while( ptr < buf + buf_len ) {
		if( ( ( ( *ptr >= 'A' ) && ( *ptr <= 'F' ) ) ||
		      ( ( *ptr >= 'a' ) && ( *ptr <= 'f' ) ) ||
		      ( ( *ptr >= '0' ) && ( *ptr <= '9' ) ) )&&
			( nibble_count < 2 ) ) {
			nibble[nibble_count] = *ptr;
			nibble_count++;
			//printf( "nibble_count %d = %c\n", nibble_count, *ptr );
			ptr++;
			if( nibble_count == 2 ) {
				//printf( "nibble = %c%c\n", nibble[0], nibble[1] );
				switch( nibble[0] ) {
					case 'A': case 'a': val = 10 << 4; break;
					case 'B': case 'b': val = 11 << 4; break;
					case 'C': case 'c': val = 12 << 4; break;
					case 'D': case 'd': val = 13 << 4; break;
					case 'E': case 'e': val = 14 << 4; break;
					case 'F': case 'f': val = 15 << 4; break;
					default: val = ( nibble[0] - 48 ) << 4; break;
				}
				switch( nibble[1] ) {
					case 'A': case 'a': val += 10; break;
					case 'B': case 'b': val += 11; break;
					case 'C': case 'c': val += 12; break;
					case 'D': case 'd': val += 13; break;
					case 'E': case 'e': val += 14; break;
					case 'F': case 'f': val += 15; break;
					default: val += ( nibble[1] - 48 ); break;
				}
				//printf( "val = %d\n", val );
				*req_ptr++ = val;
				count++;
			} 
		} else if ( *ptr == ' ' ) {
			nibble_count = 0;
			putchar( *ptr++ );
		} else if ( *ptr == ']' ) {
			putstr( "\n" );

			dprintf( DBG_SERIAL | DBG_LVL1, "        netfn = 0x%2.2x  %2d  ", tm_hdr.netfn, tm_hdr.netfn );
			switch( tm_hdr.netfn ) { 
				case NETFN_CHASSIS_REQ: 
					putstr("NETFN_CHASSIS_REQ\n"); 
					cmd_str = string_find( chassis_str, tm_hdr.command ); 
					break;
				case NETFN_CHASSIS_RESP: 
					putstr("NETFN_CHASSIS_RESP\n"); 
					cmd_str = string_find( chassis_str, tm_hdr.command ); 
					break;
				case NETFN_BRIDGE_REQ: 
					putstr("NETFN_BRIDGE_REQ\n"); 
					cmd_str = string_find( bridge_str, tm_hdr.command ); 
					break;
				case NETFN_BRIDGE_RESP: 
					putstr("NETFN_BRIDGE_RESP\n"); 
					cmd_str = string_find( bridge_str, tm_hdr.command ); 
					break;
				case NETFN_EVENT_REQ: 
					putstr("NETFN_EVENT_REQ\n"); 
					cmd_str = string_find( event_str, tm_hdr.command ); 
					break;
				case NETFN_EVENT_RESP:
					putstr("NETFN_EVENT_RESP\n"); 
					cmd_str = string_find( event_str, tm_hdr.command ); 
					break;
				case NETFN_APP_REQ: 
					putstr("NETFN_APP_REQ\n"); 
					cmd_str = string_find( app_str, tm_hdr.command ); 
					break;
				case NETFN_APP_RESP: 
					putstr("NETFN_APP_RESP\n"); 
					cmd_str = string_find( app_str, tm_hdr.command ); 
					break;
				case NETFN_FW_REQ: 
					putstr("NETFN_FW_REQ\n"); 
					cmd_str = string_find( firmware_str, tm_hdr.command ); 
					break;
				case NETFN_FW_RESP: 
					putstr("NETFN_FW_RESP\n"); 
					cmd_str = string_find( firmware_str, tm_hdr.command ); 
					break;
				case NETFN_NVSTORE_REQ: 
					putstr("NETFN_NVSTORE_REQ\n"); 
					cmd_str = string_find( nvstore_str, tm_hdr.command ); 
					break;
				case NETFN_NVSTORE_RESP: 
					putstr("NETFN_NVSTORE_RESP\n"); 
					cmd_str = string_find( nvstore_str, tm_hdr.command ); 
					break;
				case NETFN_MEDIA_SPECIFIC_REQ: 
					putstr("NETFN_MEDIA_SPECIFIC_REQ\n"); 
					cmd_str = string_find( media_specific_str, tm_hdr.command ); 
					break;
				case NETFN_MEDIA_SPECIFIC_RESP: 
					putstr("NETFN_MEDIA_SPECIFIC_RESP\n"); 
					cmd_str = string_find( media_specific_str, tm_hdr.command ); 
					break;
				case NETFN_GROUP_EXTENSION_REQ: 
					putstr("NETFN_GROUP_EXTENSION_REQ\n"); 
					cmd_str = string_find( group_extension_str, tm_hdr.command ); 
					break;
				case NETFN_GROUP_EXTENSION_RESP: 
					putstr("NETFN_GROUP_EXTENSION_RESP\n"); 
					cmd_str = string_find( group_extension_str, tm_hdr.command ); 
					break;
				case NETFN_OEM_REQ: 
					putstr("NETFN_OEM_REQ\n"); 
					cmd_str = string_find( oem_str, tm_hdr.command ); 
					break;
				case NETFN_OEM_RESP: 
					putstr("NETFN_OEM_RESP\n"); 
					cmd_str = string_find( oem_str, tm_hdr.command ); 
					break;
				default: putstr("UNKNOWN\n"); break;
			}

			dprintf( DBG_SERIAL | DBG_LVL1, "          lun = 0x%2.2x  %2d\n", tm_hdr.lun, tm_hdr.lun );
			dprintf( DBG_SERIAL | DBG_LVL1, "          seq = 0x%2.2x  %2d\n", tm_hdr.seq, tm_hdr.seq );
			dprintf( DBG_SERIAL | DBG_LVL1, "       bridge = 0x%2.2x  %2d\n", tm_hdr.bridge, tm_hdr.bridge );
			if( cmd_str )
				dprintf( DBG_SERIAL | DBG_LVL1, "      command = 0x%2.2x  %2d  %s\n", 
						tm_hdr.command, tm_hdr.command, cmd_str );
			else
				dprintf( DBG_SERIAL | DBG_LVL1, "      command = 0x%2.2x  %2d  Unknown\n", 
						tm_hdr.command, tm_hdr.command );
				
			if( ws = ws_alloc() ) {
				ws->incoming_protocol = IPMI_CH_PROTOCOL_TMODE;
				ws->incoming_medium = IPMI_CH_MEDIUM_SERIAL;
				ws->incoming_channel = IPMI_CH_NUM_CONSOLE;
				ws->len_in = count;
				memcpy( ws->pkt_in, &tm_hdr, count ); 
				ws_set_state( ws, WS_ACTIVE_IN );
			} else {
				dputstr( DBG_SERIAL | DBG_LVL1, "Insufficient resources to complete command\n");
			}
			return;
		} else {
			/* invalid character */
			putstr( "[ERR]\n" );
			return;
		}
	}
	putstr( "[OK]\n" );
}

void
terminal_process_work_list( void )
{

	if( data_ready_0 ) {
		data_ready_0 = 0;
		if( serial_port[0].callback_fn )
			(*serial_port[0].callback_fn)( serial_port[0].buf );
	}

	if( data_ready_1 ) {
		data_ready_1 = 0;
		if( serial_port[1].callback_fn )
			(*serial_port[1].callback_fn)( serial_port[1].buf );
	}
	
}	

char hex_chars[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

void
serial_tm_send( unsigned char *ws ) 
{
	int i, hi_nibble, lo_nibble;
	
	putstr( "[" );
	for( i = 0; i < ((IPMI_WS *)ws)->len_out; i++ ) { 
		//printf( "%2.2x", ((IPMI_WS *)ws)->pkt_out[i] );
		hi_nibble = ((IPMI_WS *)ws)->pkt_out[i] >> 4;
		lo_nibble = ((IPMI_WS *)ws)->pkt_out[i] & 0x0f;
		putchar( hex_chars[hi_nibble] );
		putchar( hex_chars[lo_nibble] );
		if( i < ( ((IPMI_WS *)ws)->len_out - 1 ) )
			putstr( " " );
	}
	putstr( "]\n" );
}


void
puthex( unsigned char ch ) 
{
	int hi_nibble, lo_nibble;
	
	hi_nibble = ch >> 4;
	lo_nibble = ch & 0x0f;
	putchar( hex_chars[hi_nibble] );
	putchar( hex_chars[lo_nibble] );
}

	


/* Using a port: 
 
  - aquire a handle to the port 
	
      SERIAL_PORT *port_handle;
      port_handle = serial_get_handle( [UART_DEBUG | UART_ITLA] );
	
  - set filter type

      serial_config_port_filter( port_handle, [UART_FILTER_RAW | UART_FILTER_TERM] );
       	
  - set data handler callback function. The scheduler will call the serial 
    handler which will call this function

      serial_config_port_callback( port_handle, callback_fn );

  - use putc to send characters to the required port

      putc( int ch, int handle )
*/
int
serial_get_handle( uchar port_name ) 
{
	int i;
	
	for( i = 0 ; i < UART_PORT_COUNT; i++ ) {
		if( serial_port[i].port_name == port_name ) 
			return( i );
	}
	return( -1 );
}

/* 	
	serial_config_port_filter()

 set filter type
On success, ESUCCESS is returned.
On error, the function returns EOF.
 filter type = 	UART_FILTER_RAW | UART_FILTER_TERM
*/
int
serial_config_port_filter( 
	int port_handle, 
	unsigned filter_type )  	// [UART_FILTER_RAW | UART_FILTER_TERM] );
{
	// TODO Error checking
	serial_port[port_handle].filter_type = filter_type;
	return( ESUCCESS );
}       

/*	
  - set data handler callback function. The scheduler will call the serial 
    handler which will call this function
*/
int
serial_config_port_callback( 
	int port_handle, 
	void ( *callback_fn )( uchar * ) )
{					   
	//TODO error checking
	serial_port[port_handle].callback_fn = callback_fn;
	return( ESUCCESS );
}



