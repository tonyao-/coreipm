/*
-------------------------------------------------------------------------------
coreIPM/spi_prom.h

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
/* SPI EEPROM Access */

/*=============================================================================
 * SPI EEPROM ACCESS INSTRUCTIONS
 *=============================================================================*/
/*

The Write Enable Latch (WEL) bit must be set prior to each WRITE and WRSR 
instruction. The only way to do this is to send a Write Enable (WREN) instruction
to the device.

The Write Enable Latch (WEL) bit is returned to its reset state by the following
events:

  – Power-up
  – Write Disable (WRDI) instruction completion
  – Write Status Register (WRSR) instruction completion
  – Write (WRITE) instruction completion

NOTE: For any instruction to be accepted, and executed, Chip Select (S) must be
driven High after the rising edge of Serial Clock (C) for the last bit of the 
instruction, and before the next rising edge of Serial Clock (C).

Write Enable (WREN)
-------------------
To send this instruction to the device, Chip Select (S) is driven Low,
and the bits of the instruction byte are shifted in, on Serial Data Input (D). The device then
enters a wait state. It waits for a the device to be deselected, by Chip Select (S) being driven
High.

Read Status Register (RDSR)
---------------------------
The Read Status Register (RDSR) instruction allows the Status Register to be read. The
Status Register may be read at any time, even while a Write or Write Status Register cycle
is in progress. When one of these cycles is in progress, it is recommended to check the
Write In Progress (WIP) bit before sending a new instruction to the device. It is also possible
to read the Status Register continuously,

Write Status Register (WRSR)
----------------------------
The Write Status Register (WRSR) instruction allows new values to be written to
the Status Register. Before it can be accepted, a Write Enable (WREN) instruction
must previously have been executed. After the Write Enable (WREN) instruction has
been decoded and executed, the device sets the Write Enable Latch (WEL).

The Write Status Register (WRSR) instruction is entered by driving Chip Select 
(S) Low, followed by the instruction code and the data byte on Serial Data Input (D).

The Write Status Register (WRSR) instruction has no effect on b6, b5, b4, b1 
and b0 of the Status Register. b6, b5 and b4 are always read as 0.

Chip Select (S) must be driven High after the rising edge of Serial Clock (C)
that latches in the eighth bit of the data byte, and before the next rising edge 
of Serial Clock (C). Otherwise, the Write Status Register (WRSR) instruction is
not executed. As soon as Chip Select (S) is driven High, the self-timed Write 
Status Register cycle (whose duration is tW) is initiated. While the Write Status
Register cycle is in progress, the Status Register may still be read to check the
value of the Write In Progress (WIP) bit. The Write In Progress (WIP) bit is 1 
during the self-timed Write Status Register cycle, and is 0 when it is completed.
When the cycle is completed, the Write Enable Latch (WEL) is reset.

Read from Memory Array (READ)
-----------------------------
To send this instruction to the device, Chip Select (S) is first driven Low.
The bits of the instruction byte and address bytes are then shifted in, on
Serial Data Input (D). The address is loaded into an internal address register,
and the byte of data at that address is shifted out, on Serial Data Output (Q).

If Chip Select (S) continues to be driven Low, the internal address register is
automatically incremented, and the byte of data at the new address is shifted 
out.

When the highest address is reached, the address counter rolls over to zero, 
allowing the Read cycle to be continued indefinitely. The whole memory can, 
therefore, be read with a single READ instruction.

The Read cycle is terminated by driving Chip Select (S) High. The rising edge 
of the Chip Select (S) signal can occur at any time during the cycle.

The first byte addressed can be any byte within any page.

The instruction is not accepted, and is not executed, if a Write cycle is 
currently in progress.

Write to Memory Array (WRITE)
-----------------------------
To send this instruction to the device, Chip Select (S) is first driven Low.
The bits of the instruction byte, address byte, and at least one data byte are
then shifted in, on Serial Data Input (D).

The instruction is terminated by driving Chip Select (S) High at a byte boundary
of the input data. In the case of Figure 12, this occurs after the eighth bit 
of the data byte has been latched in, indicating that the instruction is being
used to write a single byte. The self-timed Write cycle starts, and continues
for a period tWC (as specified in Table 14), at the end of which the Write in
Progress (WIP) bit is reset to 0.

If, though, Chip Select (S) continues to be driven Low, as shown in Figure 13,
the next byte of input data is shifted in, so that more than a single byte, 
starting from the given address towards the end of the same page, can be written
in a single internal Write cycle. The selftimed Write cycle starts, and continues,
for a period tWC (as specified in Table 14), at the end of which the Write in
Progress (WIP) bit is reset to 0.

Each time a new data byte is shifted in, the least significant bits of the
internal address counter are incremented. If the number of data bytes sent to 
the device exceeds the page boundary, the internal address counter rolls over
to the beginning of the page, and the previous data there are overwritten with
the incoming data. (The page size of these devices is 128/256 bytes depending
on the device type).

The instruction is not accepted, and is not executed, under the following 
conditions:

  - if the Write Enable Latch (WEL) bit has not been set to 1 (by executing a
    Write Enable instruction just before)

  - if a Write cycle is already in progress

  - if the device has not been deselected, by Chip Select (S) being driven High,
    at a byte boundary (after the eighth bit, b0, of the last data byte that has 
    been latched in)

  - if the addressed page is in the region protected by the Block Protect (BP1 
    and BP0) bits.

Some devices offer an ECC (Error Correction Code) logic which compares each
4-byte word with its associated 6 EEPROM bits of ECC. So even if a single byte
has to be written, 4 bytes are internally modified (plus the ECC bits), that is,
the addressed byte is cycled together with the other three bytes making up the
word. It is therefore recommended to write by words of 4 bytes in order to
benefit from the larger amount of Write cycles.
*/

#define SPIPROM_CMD_WREN	0x06	/* Write Enable */
#define SPIPROM_CMD_WRDI	0x04	/* Write Disable */
#define SPIPROM_CMD_RDSR	0x05	/* Read Status Register */
#define SPIPROM_CMD_WRSR	0x01	/* Write Status Register */
#define SPIPROM_CMD_READ	0x03	/* Read from Memory Array */
#define SPIPROM_CMD_WRITE	0x02	/* Write to Memory Array */
/*
Address bits and Page sizes
Addresses for Read/Write array cammands are either 16 or 24 bits depending on the device.
Obviously devices above 512 Kbits require more than 16 bits of addressing. the page sizes are
also 128 or 256 bytes depending on the device. 1+ Mbit devices have 256 byte page sizes.
*/


/*=============================================================================
 * SPI EEPROM REGISTERS
 *=============================================================================*/
/* 

The status and control bits of the Status Register are as follows:

WIP bit
-------
The Write In Progress (WIP) bit indicates whether the memory is busy with a Write or Write
Status Register cycle. When set to 1, such a cycle is in progress, when reset to 0 no such
cycle is in progress.

WEL bit
-------
The Write Enable Latch (WEL) bit indicates the status of the internal Write Enable Latch.
When set to 1 the internal Write Enable Latch is set, when set to 0 the internal Write Enable
Latch is reset and no Write or Write Status Register instruction is accepted.

BP1, BP0 bits
-------------
The Block Protect (BP1, BP0) bits are non-volatile. They define the size of the area to be
software protected against Write instructions. These bits are written with the Write Status
Register (WRSR) instruction. When one or both of the Block Protect (BP1, BP0) bits is set to
1, the relevant memory area (as defined in Table 4) becomes protected against Write
(WRITE) instructions. The Block Protect (BP1, BP0) bits can be written provided that the
Hardware Protected mode has not been set.

SRWD bit
--------
The Status Register Write Disable (SRWD) bit is operated in conjunction with the Write
Protect (W) signal. The Status Register Write Disable (SRWD) bit and Write Protect (W)
signal allow the device to be put in the Hardware Protected mode (when the Status Register
Write Disable (SRWD) bit is set to 1, and Write Protect (W) is driven Low). In this mode, the
non-volatile bits of the Status Register (SRWD, BP1, BP0) become read-only bits and the
Write Status Register (WRSR) instruction is no longer accepted for execution.
*/

#define SPIPROM_SCR_WIP		0x01	/* Write in progress bit */
#define SPIPROM_SRC_WEL		0x02	/* Write enable latch bit */
#define SPIPROM_SRC_BP0		0x04	/* Block protect bits */
#define SPIPROM_SRC_BP1		0x08
#define SPIPROM_SRC_SRWD	0x10	/* Status register write protect */

void
prom_init()
{
	// TODO make sure write protection is disabled
}


void
prom_write( addr, uchar *data, int len )
{
	Sg sg;
	uchar buf[4], spi_device = SPI_DEVICE_PROM;
	uchar rcvd[128];
	int rcvd_len, xmit_len;
	
	/* The Write Enable Latch (WEL) bit must be set prior to each WRITE and
	 * WRSR instruction. The only way to do this is to send a Write Enable
	 * (WREN) instruction to the device. */
	sg.elements = 1;
	sg.e[0].ptr = buf;
	sg.e[0].len = 1;
	buf[0] = SPIPROM_CMD_WREN;

	/* now send the data */
	sg.elements = 2;
	sg.e[0].ptr = buf;
	buf[0] = SPIPROM_CMD_WRITE;
	if( two_byte_address ) {
		sg.e[0].len = 3;
		buf[1] = ( addr >> 8 ) & 0xff;
		buf[2] = addr & 0xff;
	} else {
		sg.e[0].len = 3;
		buf[1] = ( addr >> 16 ) & 0xff;
		buf[2] = ( addr >> 8 ) & 0xff;
		buf[3] = addr & 0xff;
	}
	sg.e[1].len = len;
	sg.e[1].ptr = data;

	spi_master( spi_device, &sg, rcvd, &xmit_len, &rcvd_len );
}
	
void
prom_read( addr, uchar *data, int len )
{
	Sg sg;
	uchar buf[4], spi_device = SPI_DEVICE_PROM;
	int rcvd_len, xmit_len;
	
	/* read data */
	sg.elements = 1;
	sg.e[0].ptr = buf;
	buf[0] = SPIPROM_CMD_READ;
	if( two_byte_address ) {
		sg.e[0].len = 3;
		buf[1] = ( addr >> 8 ) & 0xff;
		buf[2] = addr & 0xff;
	} else {
		sg.e[0].len = 3;
		buf[1] = ( addr >> 16 ) & 0xff;
		buf[2] = ( addr >> 8 ) & 0xff;
		buf[3] = addr & 0xff;
	}

	spi_master( spi_device, &sg, data, &xmit_len, &rcvd_len );
}

