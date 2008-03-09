/*
-------------------------------------------------------------------------------
coreIPM/port_expander.h

Author: Gokhan Sozmen
-------------------------------------------------------------------------------
Copyright (C) 200a-2008 Gokhan Sozmen
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
This code allows us to talk to port expanders.

We support either an SC18IS600 SPI to i2c/gpio bridge or a coreEXP port expander
based on an LPC210x chip. coreEXP supports the SC18 protocol so the code is
mostly same for both devices. coreEXP has 2 x i2c and 20 GPIOs so it uses a
superset of the SC18 protocol.
*/
/*=============================================================================
 * EXPANSION PORT COMMANDS
 *=============================================================================*/
/*
Write N bytes to I2C-bus slave device
+---------+----------+------------+--------+-----+--------+
| 0x00    | NUMBER   | SLAVE      | DATA   | ... | DATA   |
| COMMAND | OF BYTES | ADDRESS +W | BYTE 1 |     | BYTE n |
+---------+----------+------------+--------+-----+--------+
The SPI host issues the write command by sending a 0x00 command followed by the total
number of bytes (maximum 96 bytes excluding the address) to send and an I2C-bus slave
device address followed by I2C-bus data bytes, beginning with the first byte (data byte 1)
and ending with the last byte (data byte N). Once the SPI host issues this command, the
expander1 will access the I2C-bus slave device and start sending the I2C-bus data
bytes.
When the I2C-bus write transaction has successfully finished, and interrupt is generated
on the INT pin, and the ‘transaction completed’ status can be read in I2CStat.
Note that the third byte sent by the host is the device I2C-bus slave address. The
expander will ignore the least significant bit so a write will always be performed
even if the least significant bit is a 1.
*/
#define EXP_CMD_I2C_WRITE		0x00

/*
Read N bytes from I2C-bus slave device
+---------+----------+------------+
| 0x01    | NUMBER   | SLAVE      |
| COMMAND | OF BYTES | ADDRESS +R |
+---------+----------+------------+
Once the host issues this command, the expander will start an I2C-bus read
transaction on the I2C-bus to the specified slave address. Once the data is received, the
expander will place this data in the receiver buffer, and will generate an interrupt on
the INT pin. The ‘transaction completed’ status can be read in the I2CStat. Note that the
data is not returned until a read buffer command is performed.
*/
#define EXP_CMD_I2C_READ		0x01

/*
I2C-bus read after write
+---------+----------+---------+---------+--------+-----+--------+---------+
| 0x02    | NUMBER   | NUMBER  | SLAVE   | DATA   | ... | DATA   | SLAVE   |
| COMMAND | OF WRITE | OF READ | ADDRESS | WRITE  |     | WRITE  | ADDRESS |
|         | BYTES    | BYTES   | +W      | BYTE 0 |     | BYTE n | +R      |
+---------+----------+---------+---------+--------+-----+--------+---------+
Once the host issues this command, the expander will start a write transaction on
the I2C-bus to the specified slave address. Once the data is written, the expander
will read data from the specified slave, place the data in the Receiver Buffer and generate
an interrupt on the INT pin. The ‘transaction completed’ status can be read in I2CStat.
Note that the data is not returned until a ‘read buffer’ command is performed.
*/
#define EXP_CMD_I2C_READ_AFTER_WRITE	0x02

/*
Read buffer
+---------+--------+-----+--------+
| 0x06    | DATA   | ... | DATA   |
| COMMAND | BYTE 1 |     | BYTE n |
+---------+--------+-----+--------+
When the host issues a Read Buffer command, the expander will return the data in
the read buffer on the MISO pin. Note that the Read Buffer will be overwritten if an
additional ‘Read N bytes’ or a ‘Read after write’ command is executed before the Read
Buffer command.
*/
#define EXP_CMD_READ_BUFFER		0x06

/*
I2C-bus write after write [ NOT SUPPORTED ]
+---------+-----+---
| 0x03    | ... |
| COMMAND |     |
+---------+-----+---
*/

/*
SPI configuration
When the host issues this command, the SC18IS600/601 will first write N data bytes to
the I2C-bus slave 1 device followed by a write of M data bytes to the I2C-bus slave 2
device.
+---------+---------------+
| 0x18    | SPI           |
| COMMAND | CONFIGURATION |
+---------+---------------+
The SPI configuration command can be used to change the order in which the bits of SPI
data byte are sent on the SPI bus. In the LSB first configuration (SPI configuration data is
0x42), bit 0 is the first bit sent of any SPI byte. In MSB first (SPI configuration data is
0x81), bit 7 is the first bit sent. Valid SPI configuration values are LSB_CONFIG_LSB_FIRST
and LSB_CONFIG_MSB_FIRST
*/
#define EXP_CMD_SPI_CONFIG		0x18

/*
Write to expander internal registers
+---------+----------+------+
| 0x20    | REGISTER | DATA |
| COMMAND |     X    | BYTE |
+---------+----------+------+
A Write Register function is initiated by sending a 0x20 command followed by an internal
register address to be written. The register data byte follows the register
address. Only one register can be accessed in a single transaction. There is no
auto-incrementing of the register address.
*/
#define EXP_CMD_WRITE_REGISTER		0x20

/*
+---------+----------+-------+
| 0x21    | REGISTER | DUMMY |
| COMMAND |     X    | BYTE  |
+---------+----------+-------+
A Read Register function is initiated by sending a 0x21 command followed by an internal
register address to be read and a dummy byte. The data byte of the
read register is returned by the expander on the MISO pin. Only one register can be
accessed in a single transaction. There is no auto-incrementing of the register address.
Note that write and read from internal registers are processed immediately as soon as the
expander determines the intended register.
*/
#define EXP_CMD_READ_REGISTER		0x21

/*
Power-down mode [NOT SUPPORTED ]
+---------+-----+---
| 0x30    | ... |
| COMMAND |     |
+---------+-----+---
*/

// SPI CONFIGURATION
#define LSB_CONFIG_LSB_FIRST	0x42	// LSB first
#define LSB_CONFIG_MSB_FIRST	0x81 	// MSB first (default)


/*=============================================================================
 * EXPANSION PORT REGISTERS
 *=============================================================================*/
/*
Register
address
     Register  Bit7   Bit6   Bit5   Bit4   Bit3    Bit2    Bit1    Bit0    R/W  Default
                                                                                value
--------------------------------------------------------------------------------------
0x00 IO0Config IO3.1  IO3.0  IO2.1  IO2.0  IO1.1   IO1.0   IO0.1   IO0.0   R/W  0x00
0x01 IO0State  0      0      GPIO5  GPIO4  GPIO3   GPIO2   GPIO1   GPIO0   R/W  0x3F
0x02 I2C0Clock CR7    CR6    CR5    CR4    CR3     CR2     CR1     CR0     R/W  0x19
0x03 I2C0TO    TO6    TO5    TO4    TO3    TO2     TO1     TO0     TE      R/W  0xFE
0x04 I2C0Stat  1      1      1      1      I2C0ST3 I2C0ST2 I2C0ST1 I2C0ST0 R    0xF0
0x05 I2C0Adr   ADR7   ADR6   ADR5   ADR4   ADR3    ADR2    ADR1    X       R/W  0x00

0x06 IO1Config IO9.1  IO9.0  IO8.1  IO8.0  IO7.1   IO7.0   IO6.1   IO6.0   R/W  0x00
0x07 IO1State  0      0      0      0      GPIO9   GPIO8   GPIO7   GPIO6   R/W  0x0F

0x08 IO2Config IO13.1 IO13.0 IO12.1 IO12.0 IO11.1  IO11.0  IO10.1  IO10.0  R/W  0x00
0x09 IO2State  0     0       0      0      GPIO13  GPIO12  GPIO11  GPIO10  R/W  0x0F

0x0A IO2Config IO17.1 IO17.0 IO16.1 IO16.0 IO15.1  IO15.0  IO14.1  IO14.0  R/W  0x00
0x0B IO2State  0     0       0      0      GPIO17  GPIO16  GPIO15  GPIO14  R/W  0x0F

0x0C IO2Config IO21.1 IO21.0 IO20.1 IO20.0 IO19.1  IO19.0  IO18.1  IO18.0  R/W  0x00
0x0D IO2State  0     0       0      0      GPIO21  GPIO20  GPIO19  GPIO18  R/W  0x0F

0x0E I2C1Clock CR7    CR6    CR5    CR4    CR3     CR2     CR1     CR0     R/W  0x19
0x0F I2C1TO    TO6    TO5    TO4    TO3    TO2     TO1     TO0     TE      R/W  0xFE
0x10 I2C1Stat  1      1      1      1      I2C1ST3 I2C1ST2 I2C1ST1 I2C1ST0 R    0xF0
0x11 I2C1Adr   ADR7   ADR6   ADR5   ADR4   ADR3    ADR2    ADR1    X       R/W  0x00

*/
#define	EXP_IO0Config	0x00
#define	EXP_IO0State	0x01
#define	EXP_I2C0Clock	0x02
#define	EXP_I2C0TO	0x03
#define	EXP_I2C0Stat	0x04
#define	EXP_I2C0Adr	0x05
#define EXP_IO1Config	0x06
#define	EXP_IO1State	0x07
#define	EXP_IO2Config	0x08
#define	EXP_IO2State	0x09
#define	EXP_IO2Config	0x0A
#define	EXP_IO2State	0x0B
#define	EXP_IO2Config	0x0C
#define	EXP_IO2State	0x0D
#define	EXP_I2C1Clock	0x0E
#define	EXP_I2C1TO	0x0F
#define	EXP_I2C1Stat	0x10
#define	EXP_I2C1Adr	0x11

#define EXP_I2CST0	0x01
#define EXP_I2CST1	0x02
#define EXP_I2CST2	0x04
#define EXP_I2CST3	0x08


/*
Programmable IO port configuration register (IOConfig)
------------------------------------------------------
Pins GPIO0 to GPIO3 may be configured by software to one of two types. These are: 
push-pull, and input-only. Two configuration bits per pin, located in the
IOConfig register, select the IO type for each pin. For SC18IS601, GPIO3 is
non-existent.

Configurations for the programmable I/O pins. IOx.1 and IOx.0 correspond to GPIOx.

IOx.1	IOx.0 	Pin configuration
---------------------------------
0 	1 	input-only configuration
1 	0 	push-pull output configuration
*/

/*
I/O pins state register (IOState)
---------------------------------
When read, this register returns the actual state of all programmable and
quasi-bidirectional I/O pins. When written, each register bit will be 
transferred to the corresponding I/O pin programmed as output.

IOState - I/O pins state register (address 0x01) bit description

Bit 	Symbol 	Description
---------------------------
7:6 	- 	reserved
5 	IO5 	Set the logic level on the output pins.
4	IO4	Write to this register:
3 	GPIO3	logic 0 = set output pin to zero
		logic 1 = set output pin to one
2 	GPIO2
1 	GPIO1
0 	GPIO0
*/

/*
I2C-bus address register (I2CAdr)
---------------------------------
The contents of the register represents the device’s own I2C-bus address. The most
significant bit corresponds to the first bit received from the I2C-bus after a START
condition. The least significant bit is not used, but should be programmed with a ‘0’.
I2CAdr is not needed for device operation, but should be configured so that its address
does not conflict with an I2C-bus device address used by the bus master.
*/

/*
I2C-bus clock rates register (I2CClk) ( 5 is the minimum value )
-------------------------------------
This register determines the I2C-bus clock frequency. The frequency can be
determined using the following formula:

I2C-bus clock frequency (Hz) = 7.3728 x 10^6 / 4 x I2CClk

The I2C-bus clock frequency for the SC18IS601 can be determined using the following
formula:

I2C-bus clock frequency (Hz) = CLKIN / 4 x I2CClk
*/

/*
I2C-bus time-out register (I2CTO)
---------------------------------
The time-out register is used to determine the maximum time that the I2C-bus master is
allowed to complete a transfer before setting an I2C-bus time-out interrupt.

Bit	Symbol		Description
7:1 	TO[7:1] 	Time-out value
0 	TE 		Enable/disable time-out function
			logic 0 = disable
			logic 1 = enable

The least significant bit of I2CTO (TE bit) is used as a time-out enable/disable. A logic 1
will enable the time-out function.

On the SC18IS600 the time-out oscillator operates at 57.6 kHz. For the SC18IS601 the
time-out oscillator frequency can be determined using the following formula:

frequency = CLKIN / 128

This oscillator is fed into a 16-bit down counter. The down counter’s lower nine bits are
loaded with ‘1’, while the upper seven bits are loaded with the contents of I2CTO.
*/

/*
I2C-bus status register (I2CStat)
---------------------------------
This register reports the results of I2C-bus transmit and receive transaction between
the expander and an I2C-bus slave device.
*/

#define EXP_I2C_XMIT_SUCCESS	0xF0 /* Transmission successful. The expander
					has successfully completed and I2C-bus
					read or write transaction. An interrupt
					is generated on INT. This is also the
					default status after reset. No interrupt
					is generated after reset. */
#define EXP_I2C_ADD_NACK	0xF1 /* I2C-bus device address not acknowledged.
				       	No I2C-bus slave device has acknowledged
				       	the slave address that has been sent out
				       	in an I2C-bus read or write transaction.
				       	An interrupt is generated on INT. */
#define EXP_I2C_DATA_NACK	0xF2 /* I2C-bus data not acknowledged. An I2C-bus
				        slave has not acknowledged the byte that
					has just been transmitted by the expander.
					An interrupt is generated on INT. */
#define EXP_I2C_BUSY		0xF3 /* I2C-bus busy. The expander is busy 
					performing an I2C-bus transaction, no
				       	new transaction should be initiated by
				       	the host. No interrupt is generated. */
#define EXP_I2C_TIMEOUT		0xF8 /* I2C-bus time-out. Check the I2C-bus
					time-out register I2CTO. The expander
					has started an I2C-bus transaction that
				       	has taken longer than the time programmed
				       	in I2CTO register. This could happen
				       	after a period of unsuccessful arbitration
				       	or when an I2C-bus slave is (continuously)
				       	pulling the SCL clock LOW. An interrupt
				       	is generated. */
#define EXP_I2C_INV_DATA_CNT	0xF9 /* I2C-bus invalid data count. The number
				       	of bytes specified in a read or write
				       	command to the expander. An interrupt
				       	is generated on INT. */

