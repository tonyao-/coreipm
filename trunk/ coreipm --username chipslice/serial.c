/*
-------------------------------------------------------------------------------
coreIPM/serial.c

Author: Gokhan Sozmen
-------------------------------------------------------------------------------
Copyright (C) 2007 Gokhan Sozmen
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

#define uchar unsigned char

#define CR     0x0D
#define LF     0x0A
#define CLI_PROMPT "BMC>"

uchar line_buf[160]; /* TODO: a single line_buf is not sufficient */
int buf_index = 0;
int data_ready = 0;

/*==============================================================*/
/* Local Function Prototypes					*/
/*==============================================================*/
/* UART ISR */
void UART_ISR_0( void ) __irq;
void UART_ISR_1( void ) __irq;
void term_process( uchar *buf );


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


void 
UART_ISR_1( void ) __irq 
{
	uchar int_reg;
	uchar rx_char;
	unsigned char temp;
	
	int_reg = U1IIR;	// read U1IIR to clear UART interrupt flag

	switch (int_reg)
	{
		case UARTINT_ERROR:		
	  		temp = U1LSR;		// clear interrupt
	   		break;							
		case UARTINT_RX_DATA_AVAIL:	// Rx char available
			rx_char = U1RBR;	// get char and reset int
			line_buf[buf_index++] = rx_char;
			U1THR = rx_char;	// echo character
			if (rx_char == CR) {
				U1THR = LF;	// echo character
				line_buf[buf_index] = 0;
				//printf( "%s\n", line_buf );
				//term_process( line_buf );
				data_ready = 1;
				buf_index = 0;
	   		}
			break;

		case UARTINT_CHAR_TIMEOUT:	// Character Time-out indication
			break;
		case UARTINT_THRE:		// Transmit Holding Register Empty
			break;			// U1IIR Read has reset interrupt
		case UARTINT_MODEM:
			temp = U1MSR;		// clear interrupt
			break;
	} 
	
	VICVectAddr = 0;          		// Acknowledge Interrupt
}

void 
UART_ISR_0( void ) __irq 
{
	uchar int_reg;
	uchar rx_char;
	unsigned char temp;
	
	int_reg = U0IIR;	// read U0IIR to clear UART interrupt flag

	switch (int_reg)
	{
		case UARTINT_ERROR:	
	  		temp = U0LSR;		// clear interrupt
	   		break;							
		case UARTINT_RX_DATA_AVAIL:	// Rx char available
			rx_char = U0RBR;	// get char and reset int
			U0THR = rx_char;	// echo character
			if (rx_char == CR) {
				U0THR = LF;	// echo character
	   		}
			break;

		case UARTINT_CHAR_TIMEOUT:	// Character Time-out indication
			break;
		case UARTINT_THRE:		// Transmit Holding Register Empty
			break;			// U1IIR Read has reset interrupt
		case UARTINT_MODEM:
			temp = U0MSR;		// clear interrupt
			break;
	} 
	
	VICVectAddr = 0;          		// Acknowledge Interrupt
}

/*==============================================================
 * uart_initialize()
 *==============================================================*/
void 
uart_initialize( void ) 
{
	/* Symbol  Function  PINSEL0 bit
	   P0.0  = TxD0      [1:0]
	   P0.1  = RxD0      [3:2] */
	PINSEL0 = PINSEL0 | 0x00000005;	/* Enable RxD0 and TxD0 */
	U0LCR = 0x83;		/* 8 bits, no Parity, 1 Stop bit */
	U0DLL = 98;		/* 9600 Baud Rate @ 12MHz VPB Clock */
	U0LCR = 0x03;		/* Disable access to Divisor Latches */
	U0IER = UARTINT_ENABLE_RX_DATA;   /* Enable the RDA interrupts */
	VICVectAddr5 = ( unsigned long )UART_ISR_0;	/* set interrupt vector in 5 */
	VICVectCntl5 = 0x20 | 7;			/* use it for UART1 interrupt */
	VICIntEnable = IER_UART0;			/* enable UART1 interrupt */
	   
	/* P0.8  = TxD1      [17:16]
	   P0.9  = RxD1      [19:18]
	   P0.10 = RTS1      [21:20] (NC on connector, avail on board)
	   P0.11 = CTS1      [23:22] (NC on connector, avail on board) */
	PINSEL0 = PINSEL0 | 0x00050000;	/* Enable RxD1 and TxD1 */
	U1LCR = 0x83;		/* 8 bits, no Parity, 1 Stop bit */
	U1DLL = 98;		/* 9600 Baud Rate @ 12MHz VPB Clock */
	U1LCR = 0x03;		/* Disable access to Divisor Latches */
	U1IER = UARTINT_ENABLE_RX_DATA;   /* Enable the RDA interrupts */
	VICVectAddr4 = ( unsigned long )UART_ISR_1;	/* set interrupt vector in 4 */
	VICVectCntl4 = 0x20 | 7;			/* use it for UART1 interrupt */
	VICIntEnable = IER_UART1;			/* enable UART1 interrupt */
}

// Simple polled routines
int 
putchar( int ch )	// Write character to Serial Port    
{
	return ( putchar_1( ch ) );
}

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

int 
getchar( void )		// Read character from Serial Port 
{
	return ( getchar_1() );
}

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
void
term_process( uchar *buf )
{
	uchar *ptr = buf;
	int nibble[2];
	int nibble_count;
	int val;
	int count = 0;
	int buf_len;
	char *cmd_str;
	IPMI_TERMINAL_MODE_HDR tm_hdr = { 0, 0, 0 };
	uchar *req_ptr = (char *)&tm_hdr;
	IPMI_WS *ws;

	/* first character must be '[' */
	if( strncmp( ptr++, "[", 1 ) ) {
		putstr( "[ERR]\n" );
		return;
	}

	/* if not prefixed by SYS, check if this is an ipmi message */
	if( strncmp( ptr, "SYS", 3 ) && strncmp( ptr, "sys", 3 ) ) 
		goto message_process;

	if( ptr = strchr( ptr, ' ' ) ) {
		ptr++;
	}
	else {
		putstr( "[ERR]\n" );
		return;
	}

	if( ( strncmp( ptr, "POWER", 5 ) == 0 ) 
			|| ( strncmp( ptr, "power", 5 ) == 0 ) ) {

		if( ptr = strchr( ptr, ' ' ) ) {
			ptr++;
		}
		else
			return;

		if( ( strncmp( ptr, "OFF]", 4 ) == 0 ) 
				|| ( strncmp( ptr, "off]", 4 ) == 0 ) ) {
			putstr( "[OK]\n" );
			gpio_power_off();
		} else if( ( strncmp( ptr, "ON]", 4 ) == 0 ) 
				|| (strncmp( ptr, "on]", 3 ) == 0) ) {
			putstr( "[OK]\n" );
			gpio_power_on();
		} else {
			putstr( "[ERR]\n" );
		}
		return;
	}

	if( ( strncmp( ptr, "TMODE]", 6 ) == 0 ) 
			|| ( strncmp( ptr, "tmode]", 6 ) == 0 ) ) {
		putstr( "[OK TMODE]\n" );
		return;
	}

	if( ( strncmp( ptr, "RESET]", 6 ) == 0 ) 
			|| ( strncmp( ptr, "reset]", 6 ) == 0 ) ) {
		putstr( "[OK]\n" );
		return;
	}

	if( ( strncmp( ptr, "IDENTIFY]", 9 ) == 0 ) 
			|| ( strncmp( ptr, "identify]", 9 ) == 0 ) ) {
		gpio_led_blink( GPIO_IDENTIFY_LED, 5, 5, 0 );
		putstr( "[OK]\n" );
		return;
	}

	if( ( strncmp( ptr, "HEALTH QUERY]", 13 ) == 0 ) 
			|| ( strncmp( ptr, "health query]", 13 ) == 0 ) ) {

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

	putstr( "[ERR]\n" );
	return;

message_process:
	putstr( "Processing Message\n" );
	nibble_count = 0;
	buf_len = strlen( buf );
	while( ptr < buf + buf_len ) {
		if( ( ( *ptr >= 'A' ) && ( *ptr <= 'F' ) ) ||
		    ( ( *ptr >= 'a' ) && ( *ptr <= 'f' ) ) ||
		    ( ( *ptr >= '0' ) && ( *ptr <= '9' ) ) &&
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
	if( data_ready ) {
		data_ready = 0;
		term_process( line_buf );
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



