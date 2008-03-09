/*
-------------------------------------------------------------------------------
coreIPM/spi.h

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
/*

SPI Registers
-------------
There are four registers that control the SPI peripheral.

1) SPI control register (S0SPCR)
contains a number of programmable bits used to control the function of the SPI
block. The settings for this register must be set up prior to a given data
transfer taking place. All values default to zero.

Bit Symbol    Value Description 
-----------------------------------------
1:0 - Reserved
2 BitEnable   0 The SPI controller sends and receives 8 bits of data per transfer.
              1 The SPI controller sends and receives the number of bits
	        selected by bits 11:8.
		
3 CPHA          Clock phase control determines the relationship between the data
                and the clock on SPI transfers, and controls when a slave transfer
	        is defined as starting and ending.
		
	      0 Data is sampled on the first clock edge of SCK. A transfer starts
	        and ends with activation and deactivation of the SSEL signal.
              1 Data is sampled on the second clock edge of the SCK. A transfer
	        starts with the first clock edge, and ends with the last sampling
	        edge when the SSEL signal is active.

4 CPOL          Clock polarity control.

              0 SCK is active high.
              1 SCK is active low.
	      
5 MSTR          Master mode select.

              0 The SPI operates in Slave mode.
              1 The SPI operates in Master mode.

6 LSBF          LSB First controls which direction each byte is shifted
                when transferred.
		
	      0 SPI data is transferred MSB (bit 7) first.
	      1 SPI data is transferred LSB (bit 0) first.
	      
7 SPIE          Serial peripheral interrupt enable.

              0 SPI interrupts are inhibited.
              1 A hardware interrupt is generated each time the SPIF or
                MODF bits are activated.
		
11:8 BITS       When bit 2 of the control register is 1, this field controls the
                number of bits per transfer:

           1000 8 bits per transfer
           1001 9 bits per transfer
           1010 10 bits per transfer
           1011 11 bits per transfer
           1100 12 bits per transfer
           1101 13 bits per transfer
           1110 14 bits per transfer
           1111 15 bits per transfer
           0000 16 bits per transfer

15:12 - Reserved
	      
*/
#define SPI_CTRL_BIT_ENABLE		0x004
#define SPI_CTRL_CLK_PHASE_SECOND_EDGE	0x008
#define SPI_CTRL_CLK_ACTIVE_LOW		0x010
#define SPI_CTRL_MASTER_MODE_SELECT	0x020
#define SPI_CTRL_LS_BIT_FIRST		0x040
#define SPI_CTRL_INTERRUPT_ENABLE	0x080
#define SPI_CTRL_BITS_8			0x800

/*
2) SPI status register (S0SPSR)
Contains read only bits that are used to monitor the status of the SPI interface,
including normal functions, and exception conditions. The primary purpose of this
register is to detect completion of a data transfer. This is indicated by the SPIF
bit. The remaining bits in the register are exception condition indicators.

2:0 - Reserved
3 	ABRT 	Slave abort. When 1, this bit indicates that a slave abort has
		occurred. This bit is cleared by reading this register.

4 	MODF 	Mode fault. when 1, this bit indicates that a Mode fault error has
		occurred. This bit is cleared by reading this register, then writing
		the SPI0 control register.

5 	ROVR 	Read overrun. When 1, this bit indicates that a read overrun has
		occurred. This bit is cleared by reading this register.

6 	WCOL 	Write collision. When 1, this bit indicates that a write collision
		has occurred. This bit is cleared by reading this register, then
		accessing the SPI data register.

7 	SPIF 	SPI transfer complete flag. When 1, this bit indicates when a SPI
		data transfer is complete. When a master, this bit is set at the
		end of the last cycle of the transfer. When a slave, this bit is set
		on the last data sampling edge of the SCK. This bit is cleared by
		first reading this register, then accessing the SPI data register.
		Note: this is not the SPI interrupt flag. This flag is found in the
		SPINT register.
*/

#define SPI_STAT_SLAVE_ABORT	0x08
#define SPI_STAT_MODE_FAULT	0x10
#define SPI_STAT_READ_OVERRUN	0x20
#define SPI_STAT_WRITE_COLL	0x40
#define SPI_STAT_XFER_COMPLETE	0x80
/*
3) SPI data register (S0SPDR)
Is used to provide the transmit and receive data bytes. An internal shift register
in the SPI block logic is used for the actual transmission and reception of the
serial data. Data is written to the SPI data register for the transmit case. 

There is no buffer between the data register and the internal shift register. 
A write to the data register goes directly into the internal shift register.
Therefore, data should only be written to this register when a transmit is not
currently in progress. Read data is buffered. When a transfer is complete, the
receive data is transferred to a single byte data buffer, where it is later read.
A read of the SPI data register returns the value of the read data buffer.

4) SPI clock counter register (S0SPCCR)
Controls the clock rate when the SPI block is in master mode (number of PCLK
cycles that make up an SPI clock). This needs to be set prior to a transfer
taking place, when the SPI block is a master. This register has no function when
the SPI block is a slave.

The value of this register must always be an even number. As a result, bit 0 must
always be 0. The value of the register must also always be greater than or equal
to 8. Violations of this can result in unpredictable behavior.

The SPI0 rate may be calculated as: PCLK / SPCCR0 value. The PCLK rate is
CCLK/VPB divider rate as determined by the VPBDIV register contents.

*/

#define SPI_CLK 1000000		/* 1 MHz */

typedef struct _sg_elem {
		unsigned len;
		unsigned char *ptr;
} SG_ELEM;

typedef struct _sg {
	unsigned char elements;
	SG_ELEM e[2];
} SG;

#define SPI_DEVICE_EXPANDER	0
#define SPI_DEVICE_PROM		1
void spi_initialize( void );
int spi_select( unsigned spi_device );
int spi_deselect( unsigned spi_device );
void spi_test( void );
void spi_signal_test( void );

