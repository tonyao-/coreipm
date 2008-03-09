/*
-------------------------------------------------------------------------------
coreIPM/wd.c

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
The purpose of the watchdog is to reset the microcontroller within a reasonable
amount of time if it enters an erroneous state. When enabled, the watchdog will
generate a system reset if the user program fails to "feed" (or reload) the 
watchdog within a predetermined amount of time.

The watchdog consists of a divide by 4 fixed pre-scaler (of the VPB clock) 
and a 32-bit counter. The clock isfed to the timer via a pre-scaler. The timer 
decrements when clocked. The minimum value from which the counter decrements 
is 0xFF. Setting a value lower than 0xFF causes 0xFF to be loaded in the counter.
Hence the minimum watchdog interval is (TPCLK x 256 x 4) and the maximum watchdog
interval is (TPCLK x 2^32 x 4) in multiples of (TPCLK x 4). The watchdog should 
be used in the following manner:
• Set the watchdog timer constant reload value in WDTC register.
• Setup mode in WDMOD register.
• Start the watchdog by writing 0xAA followed by 0x55 to the WDFEED register.
• Watchdog should be fed again before the watchdog counter underflows to prevent
reset/interrupt.
When the Watchdog counter underflows, the program counter will start from 
0x0000 0000 as in the case of external reset. The Watchdog Time-Out Flag (WDTOF) 
can be examined to determine if the watchdog has caused the reset condition. The 
WDTOF flag must be cleared by software.

			Watchdog register map
Name	Description
------------------------------------------------------------------------
WDMOD	Watchdog Mode register. This register contains
	the basic mode and status of the Watchdog Timer.
WDTC 	Watchdog Timer Constant register. This register
	determines the time-out value.
WDFEED 	Watchdog Feed sequence register. Writing 0xAA
	followed by 0x55 to this register reloads the
	Watchdog timer to its preset value.
WDTV 	Watchdog Timer Value register. This register reads
	out the current value of the Watchdog timer.

The WDMOD register controls the operation of the watchdog as per the combination
of WDEN and RESET bits.

Once the WDEN and/or WDRESET bits are set they can not be cleared by software. 
Both flags are cleared by an external reset or a watchdog timer underflow.
WDTOF The Watchdog Time-Out Flag is set when the watchdog times out. This flag 
is cleared by software.
WDINT The Watchdog Interrupt Flag is set when the watchdog times out. This flag 
is cleared when any reset occurs. Once the watchdog interrupt is serviced, it 
can be disabled in the VIC or the watchdog interrupt request will be generated.

Watchdog operating modes selection

WDEN 	WDRESET 	Mode of Operation
0 	X (0 or 1) 	Debug/Operate without the watchdog running.
1 	0 		Watchdog Interrupt Mode: debug with the Watchdog interrupt but no
			WDRESET enabled.
			When this mode is selected, a watchdog counter underflow will set the
			WDINT flag and the watchdog interrupt request will be generated.
1 	1 		Watchdog Reset Mode: operate with the watchdog interrupt and
			WDRESET enabled.
			When this mode is selected, a watchdog counter underflow will reset
			the microcontroller. While the watchdog interrupt is also enabled in
			this case (WDEN = 1) it will not be recognized since the watchdog
			reset will clear the WDINT flag.

		Watchdog Mode register WDMOD bit description
Bit 	Symbol 	Description 					Reset value
------------------------------------------------------------------------------------------
0 	WDEN	WDEN Watchdog interrupt Enable bit (Set Only). 	0
1 	WDRESET	WDRESET Watchdog Reset Enable bit (Set Only). 	0
2 	WDTOF 	WDTOF Watchdog Time-Out Flag. 			0 (Only after
								external reset)
3 	WDINT 	WDINT Watchdog interrupt Flag (Read Only). 	0
7:4 - Reserved, user software should not write ones to reserved
bits. The value read from a reserved bit is not defined.

Setting the WDEN bit in the WDMOD register is not sufficient to enable the watchdog. A
valid feed sequence must first be completed before the Watchdog is capable of generating
an interrupt/reset. Until then, the watchdog will ignore feed errors. Once 0xAA is written to
the WDFEED register the next operation in the Watchdog register space should be a
WRITE (0x55) to the WDFFED register otherwise the watchdog is triggered. The
interrupt/reset will be generated during the second PCLK following an incorrect access to
a watchdog timer register during a feed sequence.
*/

#include "arch.h"

#define WDMOD_BIT_WDEN		0x1
#define WDMOD_BIT_WDRESET	0x2
#define	WD_TIMEOUT	10 /* secs */
#define WD_DIVIDER	((WD_TIMEOUT * PCLK)/4)

void
wd_init()
{
	WDTC = WD_DIVIDER;
	WDMOD = WDMOD_BIT_WDEN | WDMOD_BIT_WDRESET;
	WDFEED = 0xAA;
	WDFEED = 0x55;
}

void
wd_feed()
{
	WDFEED = 0xAA;
	WDFEED = 0x55;
}
