/*
-------------------------------------------------------------------------------
coreIPM/i2c.h

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

/*==============================================================*/
/* I2C 7-BIT ADDRESSING 					*/
/*==============================================================*/
/*
SLAVE	R/W
ADDRESS	BIT DESCRIPTION
------- --- -------------------------------------
0000 000 0 General call address
0000 000 1 START byte(1)
0000 001 X CBUS address(2)
0000 010 X Reserved for different bus format(3)
0000 011 X Reserved for future purposes
0000 1XX X Hs-mode master code
1111 1XX X Reserved for future purposes
1111 0XX X 10-bit slave addressing

Available addresses are therefore:
0x08
0x10, 0x18
0x20, 0x28
 ....
0x60, 0x68
0x70
*/

#define I2C_ADDRESS_LOCAL	0
#define I2C_ADDRESS_REMOTE	1

#define I2C_NUM_CHANNELS	2
#define I2C_CLOCK_RATE		100000

/*==============================================================*/
/* I2C control flags						*/
/*==============================================================*/

#define I2C_CTRL_FL_AA		0x04	/* Assert acknowledge flag. */
#define I2C_CTRL_FL_SI		0x08	/* I2C serial interrupt flag. */
#define I2C_CTRL_FL_STO 	0x10	/* STOP flag. */
#define I2C_CTRL_FL_STA 	0x20	/* START flag.*/
#define	I2C_CTRL_FL_I2EN 	0x40	/* I2C interface enable. */

/*==============================================================*/
/* I2CSTAT Codes						*/
/*==============================================================*/

/* Slave standby */
#define	I2STAT_NADDR_SLAVE_MODE			0xFF
#define I2STAT_START_MASTER			0xFE

/* Master transmitter/receiver mode common */
#define	I2STAT_START_SENT			0x08
#define	I2STAT_REP_START_SENT			0x10
#define	I2STAT_ARBITRATION_LOST			0x38

/* Master transmitter mode */
#define	I2STAT_SLAW_SENT_ACKED			0x18
#define	I2STAT_SLAW_SENT_NOT_ACKED		0x20
#define	I2STAT_MASTER_DATA_SENT_ACKED		0x28
#define	I2STAT_MASTER_DATA_SENT_NOT_ACKED	0x30

/* Master receiver mode */
#define	I2STAT_SLAR_SENT_ACKED			0x40
#define	I2STAT_SLAR_SENT_NOT_ACKED		0x48
#define	I2STAT_MASTER_DATA_RCVD_ACKED		0x50
#define	I2STAT_MASTER_DATA_RCVD_NOT_ACKED	0x58

/* Slave receiver mode*/
#define	I2STAT_SLAW_RCVD_ACKED			0x60	/* Own SLA+W has been received; 
							   ACK has been returned. */
#define	I2STAT_ARB_LOST_SLAW_RCVD_ACKED		0x68
#define	I2STAT_GENERAL_CALL_RCVD_ACKED		0x70
#define	I2STAT_ARB_LOST_GENERAL_CALL_RCVD_ACKED	0x78
#define	I2STAT_SLAVE_DATA_RCVD_ACKED		0x80
#define	I2STAT_SLAVE_DATA_RCVD_NOT_ACKED	0x88	/* Previously addressed with own SLV
							   address; DATA has been received; 
							   ACK has been returned. */
#define	I2STAT_GENERAL_CALL_DATA_RCVD_ACKED	0x90
#define	I2STAT_GENERAL_CALL_DATA_RCVD_NOT_ACKED	0x98
#define	I2STAT_STOP_START_RCVD			0xA0

/* Slave transmitter mode */
#define	I2STAT_SLAR_RCVD_ACKED			0xA8
#define	I2STAT_ARB_LOST_SLAR_RCVD_ACKED		0xB0
#define	I2STAT_SLAVE_DATA_SENT_ACKED		0xB8
#define	I2STAT_SLAVE_DATA_SENT_NOT_ACKED	0xC0
#define	I2STAT_LAST_BYTE_SENT_ACKED		0xC8

#define	I2STAT_NO_INFO				0xF8
#define	I2STAT_BUS_ERROR			0x00

/* Channel utilization policies */
#define CH_POLICY_0_ONLY	0x0
#define CH_POLICY_1_ONLY	0x1
#define CH_POLICY_ALL		0x2

/* Channel health */	
#define I2C_CH_STATE_DISABLED		0x0
#define I2C_CH_STATE_ENABLED_FUNCTIONAL	0x1
#define I2C_CH_STATE_ENABLED_DEGRADED	0x2
#define I2C_CH_STATE_ENABLED_DEAD	0x3

/* Error codes */
#define I2ERR_NOERR			0x0
#define I2ERR_STATE_TRANSITION		0x1
#define I2ERR_ARBITRATION_LOST		0x2
#define	I2ERR_SLARW_SENT_NOT_ACKED	0x3
#define I2ERR_NAK_RCVD			0x4
#define I2ERR_TIMEOUT			0x5
#define I2ERR_BUFFER_OVERFLOW		0x6

/* Data direction */
#define DATA_DIRECTION_WRITE	0x0
#define DATA_DIRECTION_READ	0x1

/* Operation mode */
#define OP_MODE_MASTER_XMIT	0x0
#define OP_MODE_MASTER_RCV	0x1
#define OP_MODE_SLAVE		0x2
#define OP_MODE_SLAVE_ALLOC	0x3

/* Queue management macros */
#define REMLIST(buf_ptr) 		\
{ 					\
	buf_ptr = buf_ptr->next; 	\
} 					\

#define INSLIST_END(list, buf_ptr)	\
{					\
	LIST_HDR *ptr = list;		\
	while( ptr->next ) 		\
		ptr = ptr->next;	\
					\
	ptr->next = buf_ptr;		\
}					\

/* Concurrency protection primitives */
/* TODO */
#define LOCK( lock )
#define UNLOCK( lock )

/* Conrol bit manipulation macros */
#define	I2CCONSET( flags, channel )	\
{					\
	if(channel == 0 )		\
		I2C0CONSET = flags;	\
	else				\
		I2C1CONSET = flags;	\
}

#define I2CCONCLR( flags, channel ) 	\
{					\
	if(channel == 0 )		\
		I2C0CONCLR = flags;	\
	else				\
		I2C1CONCLR = flags;	\
}

#define I2CDAT_WRITE( val, channel )	\
{							\
	if(channel == 0 )		\
		I2C0DAT = val;		\
	else					\
		I2C1DAT = val;	  	\
}
#define I2CDAT_READ( channel )	(channel?I2C1DAT:I2C0DAT)
/*==============================================================*/
/* Data Structures						*/
/*==============================================================*/


/*==============================================================*/
/* Function Prototypes						*/
/*==============================================================*/
void i2c_initialize( void );
void i2c_send( void *ws );
void i2c_interface_enable_local_control( unsigned char channel, unsigned char link_id );
void i2c_interface_disable( unsigned char channel, unsigned char link_id );
void i2c_master_write( IPMI_WS *ws );
void i2c_master_read( IPMI_WS *ws );
void i2c_test_read( void );
void i2c_test_write( void );
void i2c_set_slave_receive_callback( void ( *callback_fn )( void *, int ) );
void i2c_set_read_buffer( unsigned char *buf, unsigned buf_len );
