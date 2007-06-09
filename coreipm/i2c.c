/*
-------------------------------------------------------------------------------
coreIPM/i2c.c

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



/* based on Philips app note LPC214x_AN10369_1 */


/*
The LPC2141/2/4/6/8 I2C interfaces are byte oriented, and have four operating 
modes: master transmitter mode, master receiver mode, slave transmitter mode 
and slave receiver mode.

In a given application, the I2C block may operate as a master, a slave, or 
both. In the slave mode, the I2C hardware looks for its own slave address and 
the general call address. If one of these addresses is detected, an interrupt 
is requested. If the processor wishes to become the bus master, the hardware 
waits until the bus is free before the master mode is entered so that a 
possible slave operation is not interrupted. If bus arbitration is lost in the
master mode, the I2C block switches to the slave mode immediately and can 
detect its own slave address in the same serial transfer.

During a data transfer from a master transmitter to a slave receiver, the first
byte transmitted by the master is the slave address. Next follows a number of
data bytes. The slave returns an acknowledge bit after each received byte.

Before the master transmitter mode can be entered, the control register must 
be initialized. Control register bits are set by writing to I2C0CONSET/I2C1CONSET
and cleared by writing to I2CCONCLR/I2C1CONCLR registers.

- I2EN must be set to 1 to enable the I2C function. 
- If the AA bit is 0, the I2C interface will not acknowledge any address 
  when another device is master of the bus, so it can not enter slave mode. 
- The STA, STO and SI bits must be 0. The SI Bit is cleared by writing 1 to the
  SIC bit in the I2CONCLR register.

I2C0CONSET and I2C1CONSET used to configure Master mode
Bit     7   6   5   4   3   2   1   0
Symbol  - I2EN STA STO SI  AA   -   -
Value   -   1   0   0   0   0   -   -

The first byte transmitted contains the slave address of the receiving device 
(7 bits) and the data direction bit. In this mode the data direction bit (R/W)
should be 0 which means Write. The first byte transmitted contains the slave 
address and Write bit. Data is transmitted 8 bits at a time. After each byte 
is transmitted, an acknowledge bit is received. START and STOP conditions are
output to indicate the beginning and the end of a serial transfer.

The I2C interface will enter master transmitter mode when software sets the
STA bit. The I2C logic will send the START condition as soon as the bus is 
free. After the START condition is transmitted, the SI bit is set, and the 
status code in the I2STAT register is 0x08. This status code is used to vector
to a state service routine which will load the slave address and Write bit to 
the I2DAT register, and then clear the SI bit. SI is cleared by writing a 1 to
the SIC bit in the I2CONCLR register.

When the slave address and R/W bit have been transmitted and an acknowledgment 
bit has been received, the SI bit is set again, and the possible status codes 
now are 0x18, 0x20, or 0x38 for the master mode, or 0x68, 0x78, or 0xB0 if the
slave mode was enabled (by setting AA to 1). The appropriate actions to be 
taken
*/

#define ARCH LPC214x

#include <stdio.h>
#include "arch.h"
#include "timer.h"
#include "ipmi.h"
#include "ws.h"
#include "i2c.h"
#include "gpio.h"
#include "serial.h"
#include "debug.h"


/* keep track of channel specific information */
typedef struct i2c_context {
	unsigned state_transition_timer;	/* timer handle */
	unsigned char state;		/* current state */
	unsigned char op_type;		/* indicates master or slave op., used for buffer allocation */
	unsigned char channel;		/* which channel this context belongs to */
	unsigned error_count;
	unsigned master_xmit_count;
	unsigned slave_rcv_count;
	IPMI_WS *ws;		/* ptr to any buffers we are currently using */
} I2C_CONTEXT;

/*==============================================================*/
/* Local Variables						*/
/*==============================================================*/
unsigned int	i2c_lock;
I2C_CONTEXT	i2c_context[I2C_NUM_CHANNELS];
unsigned	i2c_channel_selection_policy = CH_POLICY_0_ONLY;
unsigned	i2c_last_channel_used = 1;
unsigned	i2c_enable_timeout = 1;
unsigned char 	retry_timer_handle;

/*==============================================================*/
/* Global Variables						*/
/*==============================================================*/
extern unsigned long lbolt;
unsigned int local_i2c_address;
unsigned int remote_i2c_address;	/* used for debugging only */

/*==============================================================*/
/* Local Function Prototypes					*/
/*==============================================================*/
void i2c_initialize(void);

/* I2C ISR */
void I2C_ISR_0(void) __irq;
void I2C_ISR_1(void) __irq;

void i2c_proc_stat( unsigned i2stat, unsigned channel );

/* DEBUGGING */
void master_write_test( void ); 

void i2c_timeout( unsigned char *arg );
void i2c_master_complete( IPMI_WS *ws, int status );
void i2c_slave_complete( IPMI_WS *ws, int status );
void i2c_retry_enable( unsigned char *arg );

/*==============================================================
 * main()
 *==============================================================*/
int main()
{
	unsigned long time;

	/* Initialize system */
	ws_init();
	gpio_initialize();
	timer_initialize();
	i2c_initialize();
	uart_initialize();
	ipmi_initialize();

	dprintf( DBG_I2C | DBG_LVL1, "Hello World");
	
	time = lbolt;
	
	/* Do forever */
	while( 1 )
	{
		/* Blink system activity LEDs once every second */
		if( ( time + 2 ) < lbolt ) {
			time = lbolt;
			gpio_toggle_activity_led();
		}
		ws_process_work_list();
		terminal_process_work_list();
		timer_process_callout_queue();
	}
}


/*==============================================================
 * i2c_initialize()
 *==============================================================*/
void i2c_initialize( void )
{
	int channel;
	
	/* get the i2c addresses */
	local_i2c_address = gpio_get_i2c_address( I2C_ADDRESS_LOCAL );
	remote_i2c_address = gpio_get_i2c_address( I2C_ADDRESS_REMOTE );
	
	for( channel = 0 ; channel < I2C_NUM_CHANNELS; channel++ ) {
		i2c_context[channel].state = I2STAT_NADDR_SLAVE_MODE;
		i2c_context[channel].channel = channel;
		i2c_context[channel].error_count = 0;
		i2c_context[channel].master_xmit_count = 0;
		i2c_context[channel].slave_rcv_count = 0;
	}

	/* ===========================
	 * Initialize I2C channel 0
	 * ===========================*/
	I2C0CONCLR = I2C_CTRL_FL_AA | I2C_CTRL_FL_SI | 
		I2C_CTRL_FL_STA | I2C_CTRL_FL_I2EN; /* clearing all flags */

	/* I2C_CLOCK_RATE = PCLK / I2CSCLH + I2CSCLL */
	//I2C0SCLH = 50; /* For 120 KHz i2c clock using a 12 MHz chip clock */
	//I2C0SCLL = 50;
	I2C0SCLH = PCLK / I2C_CLOCK_RATE;
	I2C0SCLL = I2C0SCLH;


	/* Set our slave address. The LSB of I2ADR is the general call bit.
	 * We set this bit so that the general call address (0x00) is recognized. */
	I2C0ADR = ( local_i2c_address << 1 ) | 1;

	/* Initialize VIC for I2C use */
	/* Interrupt Select register (VICIntSelect) is a read/write accessible 
	 * register. This register classifies each of the 32 interrupt requests
	 * as contributing to FIQ or IRQ.
	 * 0 The interrupt request with this bit number is assigned to the IRQ category.
	 * 1 The interrupt request with this bit number is assigned to the FIQ category. */
	VICIntSelect = 0x0; /* assign all interrupt reqs to the IRQ category */
	 	
	/* When this register is written, ones enable interrupt requests or software
	 * interrupts to contribute to FIQ or IRQ, zeroes have no effect.  */
	VICIntEnable = IER_I2C0; /* enable I2C interrupts */

	VICVectCntl0 = 0x29; /* highest priority and enabled */
	VICVectAddr0 = (unsigned long)I2C_ISR_0;

	/* ISR address written to the respective address register*/
 	I2C0CONSET = I2C_CTRL_FL_I2EN | I2C_CTRL_FL_AA; /* enabling I2C */
	
	/* ===========================
	 * Initialize I2C channel 1
	 * ===========================*/
	I2C1CONCLR = I2C_CTRL_FL_AA | I2C_CTRL_FL_SI | 
		I2C_CTRL_FL_STA | I2C_CTRL_FL_I2EN; /* clearing all flags */

	I2C1SCLH = I2C0SCLH;
	I2C1SCLL = I2C1SCLH;


	/* Set our slave address. The LSB of I2ADR is the general call bit.
	 * We set this bit so that the general call address (0x00) is recognized. */
	I2C1ADR = ( local_i2c_address << 1 ) | 1;
	 	
	/* When this register is written, ones enable interrupt requests or software
	 * interrupts to contribute to FIQ or IRQ, zeroes have no effect.  */
	VICIntEnable = IER_I2C1; /* enable I2C interrupts */

	VICVectCntl1 = 0x29; /* highest priority and enabled */
	VICVectAddr1 = (unsigned long)I2C_ISR_1;

	/* ISR address written to the respective address register*/
 	I2C1CONSET = I2C_CTRL_FL_I2EN | I2C_CTRL_FL_AA; /* enabling I2C */

	/* start channel retry timer */
	timer_add_callout_queue( (void *)&retry_timer_handle,
		       	30*HZ, i2c_retry_enable, 0 ); /* 30 sec timeout */

}


/*==============================================================
 * INTERRUPT SERVICE ROUTINES
 *==============================================================*/
void I2C_ISR_0( void ) __irq
{
	unsigned int i2c_stat = 0;
	
	i2c_stat = I2C0STAT;
	i2c_proc_stat( i2c_stat, 0 );
	
	VICVectAddr = 0xFF;	/* - update priority hardware -
				 * the value written here is irrelevant */
}

void I2C_ISR_1( void ) __irq
{
	unsigned int i2c_stat = 0;
	
	i2c_stat = I2C1STAT;
	i2c_proc_stat( i2c_stat, 1 );
	
	VICVectAddr = 0xFF;
}

/* General Call Handling
According to the i2c specification, the meaning of the general call address is
always specified in the second byte (first data byte after the address).

There are two cases to consider:
· When the least significant bit B is a ‘zero’.
· When the least significant bit B is a ‘one’.

When bit B is a ‘zero’; the second byte has the following definition:

· 00000110 (H‘06’). Reset and write programmable part of slave address by hardware.
On receiving this 2-byte sequence, all devices designed to respond to the general
call address will reset and take in the programmable part of their address. 

· 00000100 (H‘04’). Write programmable part of slave address by hardware. All 
devices which define the programmable part of their address by hardware (and
which respond to the general call address) will latch this programmable part
at the reception of this two byte sequence. The device will not reset.

· 00000000 (H‘00’). This code is not allowed to be used as the second byte.

The remaining codes have not been fixed and devices must ignore them.

We ignore the bit B is a ‘zero’ case.

When bit B is a ‘one’; the 2-byte sequence is a ‘hardware general call’. This
means that the sequence is transmitted by a hardware master device, such as a
keyboard scanner, which cannot be programmed to transmit a desired slave 
address. Since a hardware master doesn’t know in advance to which device the
message has to be transferred, it can only generate this hardware general call
and its own address - identifying itself to the system.

The seven bits remaining in the second byte contain the address of the hardware
master. This address is recognized by an intelligent device (e.g. a microcontroller)
connected to the bus which will then direct the information from the hardware 
master. If the hardware master can also act as a slave, the slave address is 
identical to the master address.

The IPMI spec indicates that the IPMB message format for the Broadcast Get Device
ID request exactly matches that for the Get Device ID command, with the exception
that the IPMB message is prefixed with the 00h broadcast address.

The question is: Is the first data byte to be handled as defined in the i2c spec.
or as a slave address as implied in IPMI ?

*/
/*==============================================================
 * i2c_proc_stat()
 * 	i2c state machine
 * 	We can only be in OP_MODE_MASTER or OP_MODE_SLAVE.
 *==============================================================*/
void
i2c_proc_stat(unsigned i2stat, unsigned channel)
{
	I2C_CONTEXT *context = &i2c_context[channel];
	unsigned start_timer = 1;
	
	/* remove the state transition timer from the callout queue */
	timer_remove_callout_queue( (void *)&(context->state_transition_timer) );
	
	switch( i2stat ) {
		/* Master transmitter/receiver mode common */
		case I2STAT_START_SENT:
			/* A Start condition has been transmitted as a result of 
			 * i2c_master_read/write. The slave address + R/W bit will 
			 * be transmitted, an ACK bit received is expected next.
			 * context->ws points to a send request
			 */
			
			if( ( !context->ws ) || ( context->state != I2STAT_START_MASTER ) || 
					( context->op_type != OP_MODE_MASTER_XMIT ) &&
			 		( context->op_type != OP_MODE_MASTER_RCV ) ) 
			{
				/* error condition */
				if( context->ws ) {
					(*context->ws->xport_completion_function)( (void *)context->ws, 
							I2ERR_STATE_TRANSITION );
					context->ws = 0;
				}
				context->state = I2STAT_NADDR_SLAVE_MODE;
				start_timer = 0;
				context->op_type = OP_MODE_SLAVE;
				I2CCONSET( I2C_CTRL_FL_AA | I2C_CTRL_FL_STO, channel );
				I2CCONCLR( I2C_CTRL_FL_SI, channel ); 
			} else {
				context->state = I2STAT_START_SENT;
			
				/* load I2DAT with the slave address and the data 
				 * direction bit (SLA+W). The SI bit in I2CON must 
				 * then be reset before the serial transfer can continue.
				 */
				if( context->op_type == OP_MODE_MASTER_XMIT ) {
					I2CDAT_WRITE( context->ws->addr_out << 1 | 
							DATA_DIRECTION_WRITE, channel );
				}
				else {
					I2CDAT_WRITE( context->ws->addr_out << 1 | 
							DATA_DIRECTION_READ, channel );
				}
				I2CCONCLR( I2C_CTRL_FL_SI | I2C_CTRL_FL_STA, channel ); 
			}
			break;
			
		case I2STAT_REP_START_SENT:
			/* not supported - we should not be here */
			if( context->ws ) {
				(*context->ws->xport_completion_function)( (void *)context->ws, 
						I2ERR_STATE_TRANSITION );
				context->ws = 0;
			}
			context->state = I2STAT_NADDR_SLAVE_MODE;
			start_timer = 0;
			context->op_type = OP_MODE_SLAVE;
			I2CCONSET( I2C_CTRL_FL_STO, channel );
			I2CCONCLR( I2C_CTRL_FL_SI, channel ); 
			break;
			
		case I2STAT_ARBITRATION_LOST:
			/* Arbitration has been lost during either Slave 
			 * Address + Write or data. The bus has been released.
			 * 
			 * Due to the nature of the i2c design, we may not know
			 * that we may lose the arbitration until the first
			 * different bit of either the SLA+RW or data. 
			 * 
			 * call ws->xport_completion function & enter not adressed slave mode.
			 *
			 * The ws will be retried later.
			 */
			
			if( context->ws ) {
				(*context->ws->xport_completion_function)( context->ws, I2ERR_ARBITRATION_LOST );
				context->ws = 0;
			}
			/* We don't set the start bit so I2C-bus will be released; 
			 * the I2C block will enter a slave mode. */
			
			context->state = I2STAT_NADDR_SLAVE_MODE;
			start_timer = 0;
			context->op_type = OP_MODE_SLAVE;
			I2CCONCLR( I2C_CTRL_FL_SI, channel ); 
			break;

		/* Master transmitter mode */
		case I2STAT_SLAW_SENT_ACKED:
			/* Previous state was State I2STAT_START_SENT or 
			 * State I2STAT_REP_START_SENT. Slave Address + 
			 * Write has been transmitted, ACK has been received.
			 * The first data byte will be transmitted, an ACK 
			 * bit will be received.
			 * context->ws still points to an outgoing ws
			 */
			
			if( (!context->ws ) || 
			    ( ( context->state != I2STAT_START_SENT ) && 
				  ( context->state != I2STAT_REP_START_SENT ) ) || 
			    ( context->op_type != OP_MODE_MASTER_XMIT ) ) {
				if( context->ws ) {
					(*context->ws->xport_completion_function)( context->ws, 
							I2ERR_STATE_TRANSITION );
					context->ws = 0;
				}
				context->state = I2STAT_NADDR_SLAVE_MODE;
				start_timer = 0;
				context->op_type = OP_MODE_SLAVE;
				I2CCONSET( I2C_CTRL_FL_STO, channel );
				I2CCONCLR( I2C_CTRL_FL_SI, channel ); 
			} else {
				context->state = I2STAT_SLAW_SENT_ACKED;
				/* write first byte of data */
				context->ws->len_sent = 1;
				I2CDAT_WRITE( context->ws->pkt_out[0], channel );
				I2CCONCLR( I2C_CTRL_FL_SI | I2C_CTRL_FL_STA, channel ); 
			}
			break;

		case I2STAT_SLAW_SENT_NOT_ACKED:
			/* Slave Address + Write has been transmitted, NOT ACK
			 * has been received. A Stop condition will be transmitted.
			 * Release current ws & enter not adressed slave mode. */
			
			/* We will end up here if there are no listeners/open circuit
			 * on the bus so increment the channel error count. */
			context->error_count++;
			
			if( context->ws ) {
				(*context->ws->xport_completion_function)( context->ws, I2ERR_SLARW_SENT_NOT_ACKED );
				context->ws = 0;
			}
			context->state = I2STAT_NADDR_SLAVE_MODE;
			start_timer = 0;
			context->op_type = OP_MODE_SLAVE;
			I2CCONSET( I2C_CTRL_FL_STO, channel );
			I2CCONCLR( I2C_CTRL_FL_SI, channel ); 
			break;
			
		case I2STAT_MASTER_DATA_SENT_ACKED:
			/* Data has been transmitted, ACK has been received.
			 * If the transmitted data was the last data byte 
			 * then transmit a Stop condition, otherwise transmit
			 * the next data byte. */
			
			if( (!context->ws ) || 
				( ( context->state != I2STAT_MASTER_DATA_SENT_ACKED ) && 
			          ( context->state != I2STAT_SLAW_SENT_ACKED ) ) || 
			        ( context->op_type != OP_MODE_MASTER_XMIT ) ) 
			{
				if( context->ws ) {
					(*context->ws->xport_completion_function)( context->ws, 
							I2ERR_STATE_TRANSITION );
					context->ws = 0;
				}
				context->state = I2STAT_NADDR_SLAVE_MODE;
				start_timer = 0;
				context->op_type = OP_MODE_SLAVE;
				I2CCONSET( I2C_CTRL_FL_AA | I2C_CTRL_FL_STO, channel );
				I2CCONCLR( I2C_CTRL_FL_SI, channel ); 
			} else {
				context->state = I2STAT_MASTER_DATA_SENT_ACKED;
				if (context->ws->len_sent >= context->ws->len_out ) {
					/* we've sent all the data requested of us */
					(*context->ws->xport_completion_function)( context->ws, I2ERR_NOERR );
					context->ws = 0;
					context->state = I2STAT_NADDR_SLAVE_MODE;
					start_timer = 0;
					context->op_type = OP_MODE_SLAVE;
					I2CCONSET( I2C_CTRL_FL_AA | I2C_CTRL_FL_STO , channel );
					I2CCONCLR( I2C_CTRL_FL_SI, channel );
				} else {
					I2CDAT_WRITE( context->ws->pkt_out[context->ws->len_sent], channel );
					context->ws->len_sent++;	/* this is the actual count of bytes sent */
					I2CCONSET( I2C_CTRL_FL_AA, channel );
					I2CCONCLR( I2C_CTRL_FL_SI, channel );
				}
			}
			break;
			
		case I2STAT_MASTER_DATA_SENT_NOT_ACKED:
			/* Data has been transmitted, NOT ACK received. 
			 * Send a STOP condition & enter not adressed slave mode.
			 */
			if( context->ws ) {
				(*context->ws->xport_completion_function)( context->ws, I2ERR_NAK_RCVD );
			}
			context->state = I2STAT_NADDR_SLAVE_MODE;
			start_timer = 0;
			context->op_type = OP_MODE_SLAVE;
			I2CCONSET( I2C_CTRL_FL_AA | I2C_CTRL_FL_STO , channel );
			I2CCONCLR( I2C_CTRL_FL_SI, channel );
			break;

		/* Master receiver mode */
		case I2STAT_SLAR_SENT_ACKED:			
		case I2STAT_SLAR_SENT_NOT_ACKED:
		case I2STAT_MASTER_DATA_RCVD_ACKED:
		case I2STAT_MASTER_DATA_RCVD_NOT_ACKED:
			/* Not suppported right now - we will need this for local i2c bus support
			 * For the tlme being we should not be here, send a STOP 
			 * & enter not adressed slave mode.
			 */
			if( context->ws ) {
				(*context->ws->xport_completion_function)( context->ws, I2ERR_STATE_TRANSITION );
				context->ws = 0;
			}

			context->state = I2STAT_NADDR_SLAVE_MODE;
			start_timer = 0;
			context->op_type = OP_MODE_SLAVE;
			I2CCONSET( I2C_CTRL_FL_AA | I2C_CTRL_FL_STO , channel );
			I2CCONCLR( I2C_CTRL_FL_SI, channel );
			break;
			
		/* Slave receiver mode*/
		case I2STAT_SLAW_RCVD_ACKED: 
		case I2STAT_GENERAL_CALL_RCVD_ACKED:
			/* Own SLA+W has been rcvd; ACK has been returned. */

			/* illegal state transition handling */
			if( context->state != I2STAT_NADDR_SLAVE_MODE ) {
				if( context->ws ) {
					(*context->ws->xport_completion_function)( context->ws, I2ERR_STATE_TRANSITION );
					context->ws = 0;
				}
				context->op_type = OP_MODE_SLAVE;
				/* Return NAK */
				I2CCONCLR( I2C_CTRL_FL_SI | I2C_CTRL_FL_AA, channel ); 
				return;
			}
			
			/* set up data buffer */
			if( !( context->ws = ws_alloc() ) ) {
				/* Buffer allocation failed handling: return NAK */
				I2CCONCLR( I2C_CTRL_FL_SI | I2C_CTRL_FL_AA, channel ); 
				return;
			}

			context->ws->incoming_protocol = IPMI_CH_PROTOCOL_IPMB;
			context->ws->incoming_medium = IPMI_CH_MEDIUM_IPMB;
			context->ws->incoming_channel = IPMI_CH_NUM_PRIMARY_IPMB;
			context->op_type = OP_MODE_SLAVE_ALLOC;
			context->ws->xport_completion_function = i2c_slave_complete; 
			context->ws->len_in = 0; /* reset data counter */
			context->state = I2STAT_SLAW_RCVD_ACKED;
			context->slave_rcv_count++;
			if( i2stat == I2STAT_GENERAL_CALL_RCVD_ACKED )
				context->ws->flags = WS_FL_GENERAL_CALL;
			else
				context->ws->flags = 0;
			
			/* set AA flag to get data byte & clear SI */
			I2CCONSET( I2C_CTRL_FL_AA, channel );
			I2CCONCLR( I2C_CTRL_FL_SI, channel ); 

			break;


		case I2STAT_SLAVE_DATA_RCVD_ACKED:
		case I2STAT_GENERAL_CALL_DATA_RCVD_ACKED:
			/* Previously addressed with own SLV address; 
			 * DATA has been received; ACK has been returned. */
			if( ( context->state != I2STAT_SLAVE_DATA_RCVD_ACKED ) ||
				( context->state != I2STAT_SLAW_RCVD_ACKED ) ||
				( context->state != I2STAT_GENERAL_CALL_RCVD_ACKED ) ||
				( context->state != I2STAT_GENERAL_CALL_DATA_RCVD_ACKED ) ||
				( context->state != I2STAT_ARB_LOST_SLAW_RCVD_ACKED ) ) {

				/* illegal state transition handling */
				if( context->ws ) {
					(*context->ws->xport_completion_function)( context->ws, I2ERR_STATE_TRANSITION );
					context->ws = 0;
				}
				context->op_type = OP_MODE_SLAVE;
				/* Return NAK */
				I2CCONCLR( I2C_CTRL_FL_SI | I2C_CTRL_FL_AA, channel ); 
				return;
			}
				
			if( context->ws->len_in > WS_BUF_LEN ) {
				/* clear AA flag to send a no-ack */
				I2CCONCLR( I2C_CTRL_FL_AA, channel );

				ws_free( context->ws );
				context->ws = 0;
			} else {
				context->ws->pkt_in[context->ws->len_in] = I2CDAT_READ( channel );
				context->ws->len_in++;
			
				/* set AA flag to get next data byte */
				I2CCONSET( I2C_CTRL_FL_AA, channel );
			}
			context->state = I2STAT_SLAVE_DATA_RCVD_ACKED;
			I2CCONCLR( I2C_CTRL_FL_SI, channel );

			break;

		case I2STAT_ARB_LOST_SLAW_RCVD_ACKED:
		case I2STAT_ARB_LOST_GENERAL_CALL_RCVD_ACKED:
			if( context->ws ) {
				(*context->ws->xport_completion_function)( context->ws, I2ERR_ARBITRATION_LOST );
				context->ws = 0;
			}
			
			/* set up data buffer */
			if( !( context->ws = ws_alloc() ) ) {
				/* Buffer allocation failed handling: return NAK */
				I2CCONCLR( I2C_CTRL_FL_SI | I2C_CTRL_FL_AA, channel ); 
				return;
			}

			context->ws->incoming_protocol = IPMI_CH_PROTOCOL_IPMB;
			context->ws->incoming_medium = IPMI_CH_MEDIUM_IPMB;
			context->ws->incoming_channel = IPMI_CH_NUM_PRIMARY_IPMB;
			context->op_type = OP_MODE_SLAVE_ALLOC;
			context->ws->xport_completion_function = i2c_slave_complete; 
			context->ws->len_in = 0; /* reset data counter */
			context->state = i2stat;
			I2CCONSET( I2C_CTRL_FL_AA, channel );
			I2CCONCLR( I2C_CTRL_FL_SI, channel );
			break;

		case I2STAT_GENERAL_CALL_DATA_RCVD_NOT_ACKED:
		case I2STAT_SLAVE_DATA_RCVD_NOT_ACKED:
			/* Previously addressed with own Slave Address. Data 
			 * has been received and NOT ACK has been returned. 
			 * Received data will not be saved. Not addressed 
			 * Slave mode is entered.
			 */
			context->state = I2STAT_NADDR_SLAVE_MODE;
			start_timer = 0;
			I2CCONSET( I2C_CTRL_FL_AA, channel );
			I2CCONCLR( I2C_CTRL_FL_SI, channel );
			break;

		case I2STAT_STOP_START_RCVD:
			if( ( context->state != I2STAT_SLAVE_DATA_RCVD_ACKED ) ||
			    ( context->state != I2STAT_GENERAL_CALL_DATA_RCVD_ACKED ) ) {
				/* illegal state transition */
				if( context->ws ) {
					(*context->ws->xport_completion_function)( context->ws, I2ERR_STATE_TRANSITION );
				}
			} else {
				(*context->ws->xport_completion_function)( context->ws, I2ERR_NOERR );
			}				
						
			context->state = I2STAT_NADDR_SLAVE_MODE;
			start_timer = 0;

			context->ws = 0;

			/* Set AA flag & clear SI */
			I2CCONSET( I2C_CTRL_FL_AA, channel );
			I2CCONCLR( I2C_CTRL_FL_SI, channel );
			
			break;
			
		/* Slave transmitter mode */
		case I2STAT_SLAR_RCVD_ACKED:
			if( context->ws ) {
				(*context->ws->xport_completion_function)( context->ws, I2ERR_STATE_TRANSITION );
				context->ws = 0;
			}
			I2CCONCLR( I2C_CTRL_FL_SI | I2C_CTRL_FL_AA , channel );
			break;
			
		case I2STAT_ARB_LOST_SLAR_RCVD_ACKED:
			if( context->ws ) {
				(*context->ws->xport_completion_function)( context->ws, I2ERR_STATE_TRANSITION );
				context->ws = 0;
			}
			I2CCONCLR( I2C_CTRL_FL_SI | I2C_CTRL_FL_AA , channel );
			break;

		case I2STAT_SLAVE_DATA_SENT_ACKED:
			/* Not supposed to get here. Set AA flag & clear SI */
			if( context->ws ) {
				(*context->ws->xport_completion_function)( context->ws, I2ERR_STATE_TRANSITION );
				context->ws = 0;
			}
			I2CCONSET( I2C_CTRL_FL_AA, channel );
			I2CCONCLR( I2C_CTRL_FL_SI, channel );
			break;
			
		case I2STAT_SLAVE_DATA_SENT_NOT_ACKED:
			if( context->ws ) {
				(*context->ws->xport_completion_function)( context->ws, I2ERR_STATE_TRANSITION );
				context->ws = 0;
			}
			I2CCONSET( I2C_CTRL_FL_AA, channel );
			I2CCONCLR( I2C_CTRL_FL_SI, channel );
			break;

		case I2STAT_LAST_BYTE_SENT_ACKED:
			if( context->ws ) {
				(*context->ws->xport_completion_function)( context->ws, I2ERR_STATE_TRANSITION );
				context->ws = 0;
			}
			I2CCONSET( I2C_CTRL_FL_AA, channel );
			I2CCONCLR( I2C_CTRL_FL_SI, channel );
			break;

		case I2STAT_NO_INFO:
		case I2STAT_BUS_ERROR:
			if( context->ws ) {
				(*context->ws->xport_completion_function)( context->ws, I2ERR_STATE_TRANSITION );
				context->ws = 0;
			}
			I2CCONSET( I2C_CTRL_FL_AA | I2C_CTRL_FL_STO, channel );
			I2CCONCLR( I2C_CTRL_FL_SI, channel );
			break;
			
		default:
			break;
	}

	if( start_timer && i2c_enable_timeout ) {
		timer_add_callout_queue( (void *)&context->state_transition_timer,
		       	10*HZ, i2c_timeout, context ); /* 10 sec timeout */
	}
}

void
i2c_retry_enable( unsigned char *arg )
{
	unsigned char channel;

	/* reset error counts so failed channels get another chance to try */
	for( channel = 0 ; channel < I2C_NUM_CHANNELS; channel++ ) {
		i2c_context[channel].error_count = 0;
	}

	/* restart channel retry timer to reset the error counts again in 30 secs */
	timer_add_callout_queue( (void *)&retry_timer_handle,
		       	30*HZ, i2c_retry_enable, 0 ); 
}

void
i2c_timeout( unsigned char *arg )
{
	I2C_CONTEXT *context = ( I2C_CONTEXT * )arg;
	
	/* state transition timeout handling */
	dputstr( DBG_I2C | DBG_ERR, "i2c_timeout: \n" );

	/* increment the channel error count. */
	context->error_count++;
	
	if( context->ws ) {
		(*context->ws->xport_completion_function)( context->ws, I2ERR_TIMEOUT );
		context->ws = 0;
	}

	/* If an uncontrolled source generates a superfluous START or masks a 
	 * STOP condition, then the I2C-bus stays busy indefinitely. If the STA
	 * flag is set and bus access is not obtained within a reasonable amount 
	 * of time, then a forced access to the I2C-bus is possible. This is 
	 * achieved by setting the STO flag while the STA flag is still set. No
	 * STOP condition is transmitted. The I2C hardware behaves as if a STOP 
	 * condition was received and is able to transmit a START condition. 
	 * The STO flag is cleared by hardware.
	 *
	 * For the stuck bus try a STOP as well  */
	
	context->state = I2STAT_NADDR_SLAVE_MODE;
	context->op_type = OP_MODE_SLAVE;
	I2CCONSET( I2C_CTRL_FL_STO | I2C_CTRL_FL_AA, context->channel );
}

void
i2c_master_read( IPMI_WS *ws )
{
	unsigned channel;
	I2C_CONTEXT *context;

	/* select channel */
	switch( i2c_channel_selection_policy ) {
		case CH_POLICY_0_ONLY:
			channel = i2c_last_channel_used = 0;
			break;
		case CH_POLICY_1_ONLY:
			channel = i2c_last_channel_used = 1;
			break;
		case CH_POLICY_ALL:
			/* Per ATCA spec. An IPM Controller will try to 
			 * alternate the transmission of messages between
			 * IPMB-A and IPMB-B. If an IPM Controller is unable
			 * to transmit on the desired IPMB then it will try
			 * to send the message on the alternate IPMB. */
			/* use the channel with least errors */
			if( i2c_context[0].error_count == i2c_context[1].error_count ) {
				if( i2c_last_channel_used == 0)
					channel = i2c_last_channel_used = 1;
				else
					channel = i2c_last_channel_used = 0;
			} else if ( i2c_context[0].error_count > i2c_context[1].error_count ) {
				channel = i2c_last_channel_used = 1;
			} else {
				channel = i2c_last_channel_used = 0;
			}
			break;
	} 
	
	context = &i2c_context[channel];
	
	if( context->state == I2STAT_NADDR_SLAVE_MODE ) {
		context->state = I2STAT_START_MASTER;
		context->op_type = OP_MODE_MASTER_RCV;
		context->ws = ws;
		/* Set start bit */
		/* interrupt service routines will take care of the rest */
		/* The master transmitter mode is entered by setting
		 * the STA bit. The I2C logic will now test the I2C-bus and 
		 * generate a start condition as soon as the bus becomes free.
		 * When a START condition is transmitted, the serial interrupt 
		 * flag (SI) is set, and the status code in the status register 
		 * (I2STAT) will be I2STAT_START_SENT (0x08). */
		
		I2CCONSET( I2C_CTRL_FL_STA | I2C_CTRL_FL_I2EN, channel );

		/* we will wait until a START can be sent, need to start timeout
		 * here so we can recover and try a different channel */
		if( i2c_enable_timeout ) {
			timer_add_callout_queue( (void *)&context->state_transition_timer,
		       		10*HZ, i2c_timeout, 0 ); /* 10 sec timeout */
		}
	} else {
		/* back to the queue */
		ws_set_state( ws, WS_ACTIVE_MASTER_READ );
	}		

}

/*
 * i2c_master_write()
 * 	Gets called by ws_process_work_list() when we find a ws with state 
 * 	WS_ACTIVE_MASTER_WRITE_PENDING in the work list.
 */
void 
i2c_master_write( IPMI_WS *ws )
{
	unsigned channel;
	I2C_CONTEXT *context;

	ws->xport_completion_function = i2c_master_complete; 
	
	/* select channel */
	switch( i2c_channel_selection_policy ) {
		case CH_POLICY_0_ONLY:
			channel = i2c_last_channel_used = 0;
			break;
		case CH_POLICY_1_ONLY:
			channel = i2c_last_channel_used = 1;
			break;
		case CH_POLICY_ALL:
			/* Per ATCA spec. An IPM Controller will try to 
			 * alternate the transmission of messages between
			 * IPMB-A and IPMB-B. If an IPM Controller is unable
			 * to transmit on the desired IPMB then it will try
			 * to send the message on the alternate IPMB. */
			/* we use the channel with least errors */
			if( i2c_context[0].error_count == i2c_context[1].error_count ) {
				if( i2c_last_channel_used == 0)
					channel = i2c_last_channel_used = 1;
				else
					channel = i2c_last_channel_used = 0;
			} else if ( i2c_context[0].error_count > i2c_context[1].error_count ) {
				channel = i2c_last_channel_used = 1;
			} else {
				channel = i2c_last_channel_used = 0;
			}
			break;
	} /* end of switch */
	
	context = &i2c_context[channel];
	
	if( context->state == I2STAT_NADDR_SLAVE_MODE ) {
		context->state = I2STAT_START_MASTER;
		context->op_type = OP_MODE_MASTER_XMIT;
		context->ws = ws;
		/* Set start bit */
		/* interrupt service routines will take care of the rest */
		/* The master transmitter mode is entered by setting
		 * the STA bit. The I2C logic will now test the I2C-bus and 
		 * generate a start condition as soon as the bus becomes free.
		 * When a START condition is transmitted, the serial interrupt 
		 * flag (SI) is set, and the status code in the status register 
		 * (I2STAT) will be I2STAT_START_SENT (0x08). */
		
		dputstr( DBG_I2C | DBG_LVL1, "i2c_master_write: sending START bit\n" );

		I2CCONSET( I2C_CTRL_FL_STA | I2C_CTRL_FL_I2EN, channel );
		
		/* we will wait until a START can be sent, need to start timeout
		 * here so we can recover and try a different channel */
		if( i2c_enable_timeout ) {
			timer_add_callout_queue( (void *)&context->state_transition_timer,
		       		10*HZ, i2c_timeout, context ); /* 10 sec timeout */
		}
	} else {
		/* back to the queue */
		ws_set_state( ws, WS_ACTIVE_MASTER_WRITE );
	}		
}

/* Transport completion routine */
void
i2c_master_complete( IPMI_WS *ws, int status )
{
	switch( status ) {
		case I2ERR_NOERR:
			dputstr( DBG_I2C | DBG_LVL1, "i2c_master_complete: completed with I2ERR_NOERR\n" );
			ws_set_state( ws, WS_ACTIVE_MASTER_WRITE_SUCCESS );
			if( ws->ipmi_completion_function ) {
				(ws->ipmi_completion_function)( (void *)ws, 
							XPORT_REQ_NOERR );
			} else { 
				ws_free( ws );
			}
			break;
			
		case I2ERR_STATE_TRANSITION:
		case I2ERR_ARBITRATION_LOST:
		case I2ERR_SLARW_SENT_NOT_ACKED:
		case I2ERR_NAK_RCVD:
		case I2ERR_TIMEOUT:
		default:
			dputstr( DBG_I2C | DBG_ERR, "i2c_master_complete: completed with I2ERR\n" );
			/* if status indicates a retryable error, try again */
			/* back to the queue */
			if ( WS_ACTIVE_MASTER_WRITE_PENDING == ws->ws_state ) {
				ws_set_state( ws, WS_ACTIVE_MASTER_WRITE );
			}
			else if ( WS_ACTIVE_MASTER_READ_PENDING == ws->ws_state ) {
				ws_set_state( ws, WS_ACTIVE_MASTER_READ );
			} else {
				/* invalid state drop it */
				if( ws->ipmi_completion_function ) {
					(ws->ipmi_completion_function)( (void *)ws, 
							XPORT_REQ_ERR );
				} else { 
					ws_free( ws );
				}
			}	
			break;
	
	}
}

/* Transport completion routine */
void
i2c_slave_complete( IPMI_WS *ws, int status )
{
	switch( status ) {
		case I2ERR_NOERR:
			ws_set_state( ws, WS_ACTIVE_IN );
			break;
		default:
			ws_free( ws );
			break;
	}			

}


/* Handlers for ATCA_CMD_SET_IPMB_STATE */
void
i2c_interface_enable_local_control( uchar channel, uchar link_id )
{
	I2CCONSET( I2C_CTRL_FL_I2EN, channel );
}

void
i2c_interface_disable( uchar channel, uchar link_id )
{
	I2CCONCLR( I2C_CTRL_FL_I2EN, channel );
}


