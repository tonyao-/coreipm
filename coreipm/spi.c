/*
-------------------------------------------------------------------------------
coreIPM/spi.c

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
#include "arch.h"
#include "spi.h"
#include "error.h"
#include "gpio.h"
#include "debug.h"
#include "error.h"

/*==============================================================*/
/* Local Function Prototypes					*/
/*==============================================================*/
/* SPI ISR for channel 1 */
#if defined (__CA__) || defined (__CC_ARM)
void SPI_ISR_0( void ) __irq;
#elif defined (__GNUC__)
void SPI_ISR_0(void) __attribute__ ((interrupt));
#endif
int spi_out( unsigned char *wr_data, int wr_count );
int spi_in( unsigned char *rd_data, int rd_count );


/* configure SPI0 as master and SPI1 as slave */
void
spi_initialize( void )
{
	/*======================*/
	/*	SPI0		*/
	/*======================*/
	/*
	PINSEL0  Port    Value  Selected
	Bit      symbol         Function
	------------------------------------
	9:8      P0.4    01     SCK0 (SPI0)
	11:10    P0.5    01     MISO0 (SPI0)
	13:12    P0.6    01     MOSI0 (SPI0)
	15:14    P0.7    01	SSEL0 (SPI0)
                         00	GPIO Port 0.7
			 
	On the LPC2141/2/4/6/8 (unlike earlier Philips ARM devices) the SSEL0
       	pin can be used for a different function when the SPI0 interface is 
	only used in Master mode. For example, pin hosting the SSEL0 function
       	can be configured as an output digital GPIO pin and used to select one
       	of the SPI0 slaves.
	                       1 1  1 0
	                       4 2  0 8
	0b0000 0000 0000 0000 0001 0101 0000 0000 = 0x00001500 for master operation
						  = 0x00005500 for slave operation
	*/

	/* Set the SPI clock counter register */
	S0SPCCR = ( PCLK / SPI_CLK ) & 0xFE;

#ifdef MCB2140
	if( gpio_get_hardware_setting() == 1 ) { // master mode operation
		PINSEL0 |= 0x00001500;
		IO0DIR = 0x80; // pin P0.7 configured as output
		IOSET0 = 0x80; 	// drive the SSEL line high
	
		/* Set the SPI control register */
		S0SPCR = SPI_CTRL_MASTER_MODE_SELECT | SPI_CTRL_BITS_8;
	} else { // Slave mode operation
		PINSEL0 |= 0x00005500;
		IOSET0 = 0x80; 	// drive the SSEL line high

		VICIntEnable = IER_SPI0; /* enable SPI0 interrupts */

		VICVectCntl6 = 0x20 | IS_SPI0; /* This vectored IRQ slot is enabled | The 
					    number of the interrupt request assigned 
					    to this vectored IRQ slot.*/
		VICVectAddr6 = ( unsigned long )SPI_ISR_0;

		/* Set the SPI control register */
		S0SPCR = SPI_CTRL_INTERRUPT_ENABLE;
	}
#else
	IODIR1 = 0x03080000;	// pin P1.19, P1.24 & P1.25 configured as output
	IOSET1 = 0x03080000; 	// drive the Chip Select lines high
	PINSEL0 |= 0x00001500;	// master operation

	/* Set the SPI control register */
	S0SPCR = SPI_CTRL_MASTER_MODE_SELECT | SPI_CTRL_BITS_8;

#endif

	
	/* TODO: Fix - select an unused eint pin, this one is used by SPI1 SSEL
	Select an external interrupt pin EINT3 for SPI use

	PINSEL1  Port    Value  Selected
	Bit      symbol         Function
	------------------------------------
	9:8      P0.20   11     EINT3

	0b0000 0000 0000 0000 0000 0011 0000 0000 = 0x00000300
	*/
	//PINSEL1 |= 0x00000300;

	/*======================*/
	/*	SPI1		*/
	/*======================*/
	/*
	PINSEL1  Port    Value  Selected
	Bit      symbol         Function
	------------------------------------
	3:2      P0.17   10	SCK1 (SPI1)
	5:4      P0.18   10	MISO1 (SPI1)
	7:6      P0.19   10	MOSI1 (SPI1)
	9:8      P0.20   10	SSEL1 (SPI1)

	0b0000 0000 0000 0000 0000 0010 1010 1000 = 0x000002A8
	*/
	
	PINSEL1 |= 0x000002A0;
}

/* SPI device 0 is by default the master I/F.
 * The device ID determines which chip select line is enabled
 */
int
spi_master( 
	unsigned spi_device,
	SG *sg, 		// scatter/gather list
	unsigned char *rcvd,  	// data returned from device copied here
	unsigned rcv_len,	// length of rcv_buffer, don't exceed this 
	unsigned *xmit_len )	// actual bytes transferred (either direction)
{
	int i, j, k = 0;
	int retval = ENOERR;
	unsigned char status, tmp;
	
	spi_select( spi_device );

	for( i = 0; i < sg->elements; i++ )
	{
		
	    for( j = 0 ; j < sg->e[i].len ; j++ ) {
		/* Write the data to transmitted to the SPI data register. 
		 * This write starts the SPI data transfer. */
		S0SPDR = sg->e[i].ptr[j];
		
		/* Wait for the SPIF bit in the SPI status register to be set to 1. */
		while( !( S0SPSR & SPI_STAT_XFER_COMPLETE ) );
		
		/* Read the SPI status register. */
		status = S0SPSR;

		/* Check for Slave Abort. A slave transfer is considered to be
		 * aborted, if the SSEL signal goes inactive before the transfer
		 * is complete. In the event of a slave abort, the transmit and
		 * receive data for the transfer that was in progress are lost,
		 * and the slave abort (ABRT) bit in the status register
		 * will be activated. */
		if( status & SPI_STAT_SLAVE_ABORT ) {
			retval = EIO;
		}

		/* Check for Mode Fault. The SSEL signal must always be inactive
		 * when the SPI block is a master. If the SSEL signal goes active,
		 * when the SPI block is a master, this indicates another master 
		 * has selected the device to be a slave. This condition is known
		 * as a mode fault. When a mode fault is detected, the mode fault
		 * (MODF) bit in the status register will be activated, the SPI
		 * signal drivers will be de-activated, and the SPI mode will be
		 * changed to be a slave. */
		if( status & SPI_STAT_MODE_FAULT ) {
			retval = EIO;
		}

		/* Check for a read overrun. A read overrun occurs when the SPI 
		 * block internal read buffer contains data that has not been
		 * read by the processor, and a new transfer has completed. The
		 * read buffer containing valid data is indicated by the SPIF 
		 * bit in the status register being active. When a transfer
		 * completes, the SPI block needs to move the received data to
		 * the read buffer. If the SPIF bit is active (the read buffer
		 * is full), the new receive data will be lost, and the read
		 * overrun (ROVR) bit in the status register will be activated.
		 */
		if( status & SPI_STAT_READ_OVERRUN ) {
			retval = EIO;
		}

		/* Check for Write collusion. There is no write buffer between
		 * the SPI block bus interface, and the internal shift register.
		 * As a result, data must not be written to the SPI data register
		 * when an SPI data transfer is currently in progress. The time
		 * frame where data cannot be written to the SPI data register
		 * is from when the transfer starts, until after the status 
		 * register has been read when the SPIF status is active. If 
		 * the SPI data register is written in this time frame, the 
		 * write data will be lost, and the write collision (WCOL) bit
		 * in the status register will be activated. */
		if( status & SPI_STAT_WRITE_COLL ) {
			retval = EIO;
		}

		/* Read the received data from the SPI data register. 
		   Note that a read or write of the SPI data register is required
		   in order to clear the SPIF status bit. Therefore, if the 
		   optional read of the SPI data register does not take place, a
		   write to this register is required in order to clear the SPIF
		   status bit. */
		if( k < rcv_len ) 
			rcvd[k++] = S0SPDR;
		else
			tmp = S0SPDR;
		
		if( retval != ESUCCESS )
			break;
	    }
	    if( retval != ESUCCESS )
		break;
	}
	spi_deselect( spi_device );
	*xmit_len = k;
	return( retval );
}

#if defined (__CA__) || defined (__CC_ARM)
void SPI_ISR_0( void ) __irq
#elif defined (__GNUC__)
void SPI_ISR_0( void )
#endif
{
	unsigned char data_in, error = 0;
	unsigned spin_count = 0;
	unsigned char status;
	
	/* In slave mode, how we respond is dependent on what the master 
	 * expects from us, this is usually indicated by a command frame
	 * that is transferred from the master at the begining of the 
	 * transaction. The command can indicate that this is a write 
	 * operation where we should be accepting and processing incoming
	 * bytes or a read operation where we should be responding by writing
	 * data to the data register. */

	/* Set the SPI control register to disable SPI0 interrupts */
	S0SPCR = 0;
	
	while( !error ) {
		while( !( S0SPSR & SPI_STAT_XFER_COMPLETE ) ) {
			if( spin_count++ > 1000000 ) {
				error = 1;
				break;
			}
		}	

		/* Read the SPI status register. */
		status = S0SPSR;

		if( status & 
			( SPI_STAT_SLAVE_ABORT | SPI_STAT_MODE_FAULT 
			  | SPI_STAT_READ_OVERRUN | SPI_STAT_WRITE_COLL ) )
			error = 1;

		/* For testing purposes we take the incoming data, do a bitwise 
		 * invert and return it */
		data_in = S0SPDR;
		S0SPDR = ~data_in;
	}
	
	/* Set the SPI control register to enable SPI0 interrupts */
	S0SPCR = SPI_CTRL_INTERRUPT_ENABLE;

	S0SPINT = 1;		/* Clear SPI interrupt flag. */
	VICVectAddr = 0;	/* Acknowledge Interrupt */
}


int
spi_select( unsigned spi_device )
{
#ifdef MCB2140
	IOCLR0 = 0x80; 	// drive the SSEL line low
#else
#endif
	return ENOERR;
}

int
spi_deselect( unsigned spi_device )
{
#ifdef MCB2140
	IOSET0 = 0x80; 	// drive the SSEL line high
#else

#endif
	return ENOERR;
}

void
spi_test( void )
{
	SG sg;
	unsigned char rcv_buf[128], snd_buf[128] = {1, 2, 3, 4};
	unsigned int xmit_len = 0;

	sg.elements = 1;
	sg.e[0].ptr = snd_buf;
	sg.e[0].len = 4;

	spi_master( 0, &sg, rcv_buf, 4, &xmit_len );
}

/* send a large number of writes to test out clk & data signal lines */
void
spi_signal_test( void )
{
	unsigned i;
	unsigned char status;

	spi_select( 0 );
	
	for( i = 0; i < 10000000 ; i++ ) {
		/* Write the data to transmitted to the SPI data register. 
		 * This write starts the SPI data transfer. */
		S0SPDR = 55;
		
		/* Wait for the SPIF bit in the SPI status register to be set to 1. */
		while( !( S0SPSR & SPI_STAT_XFER_COMPLETE ) );
		
		/* Read the SPI status register. */
		status = S0SPSR;

		if( status & ( SPI_STAT_SLAVE_ABORT | SPI_STAT_MODE_FAULT 
			  | SPI_STAT_READ_OVERRUN | SPI_STAT_WRITE_COLL ) ) {
			putstr( "spi_signal_test: status error\n" );
			break;
		}
	}
	spi_deselect( 0 );
}

