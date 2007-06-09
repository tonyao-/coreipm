/*
-------------------------------------------------------------------------------
coreIPM/arch.h

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

#include <LPC214x.h>

// Interrupt Enable register (VICIntEnable) bit allocation
					// Bit   
#define IER_USB		0x400000	// [22]  
#define IER_AD1		0x200000	// [21]  
#define IER_BOD		0x100000	// [20]  
#define IER_I2C1	0x080000	// [19]  
#define IER_AD0		0x040000	// [18]  
#define IER_EINT3	0x020000	// [17]  
#define IER_EINT2	0x010000	// [16]  

#define IER_EINT1	0x008000	// [15]
#define IER_EINT0	0x004000	// [14]
#define IER_RTC		0x002000	// [13]
#define IER_PLL		0x001000	// [12]
#define IER_SPI1/SSP	0x000800	// [11]
#define IER_SPI0	0x000400	// [10]
#define IER_I2C0	0x000200	// [09]
#define IER_PWM0	0x000100	// [08]

#define IER_UART1	0x000080	// [07]
#define IER_UART0	0x000040	// [06]
#define IER_TIMER1	0x000020	// [05]
#define IER_TIMER0	0x000010	// [04]
#define IER_ARMCore1	0x000008	// [03]
#define IER_ARMCore0	0x000004	// [02]
#define IER_WDT		0x000001	// [00]

/* BOARD CLOCK */
#define	PCLK		12000000


/*======================================================================*/
/*			Controller Types				*/
/*======================================================================*/

#define CONTROLLER_TYPE_IPMC 0
#define CONTROLLER_TYPE_SHMC 1

/* change this to IPMI_CONTROLLER_TYPE_SHMC for testing as a shelf controller */
#define CONTROLLER_TYPE CONTROLLER_TYPE_IPMC

