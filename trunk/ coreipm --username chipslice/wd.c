/*
-------------------------------------------------------------------------------
coreIPM/wd.c

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


/*
The purpose of the watchdog is to reset the microcontroller within a reasonable
amount of time if it enters an erroneous state. When enabled, the watchdog will
generate a system reset if the user program fails to "feed" (or reload) the 
watchdog within a predetermined amount of time.

*/

#include "arch.h"

#define WDMOD_BIT_WDEN		0x1
#define WDMOD_BIT_WDRESET	0x2
#define	PCLK		12000000
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
