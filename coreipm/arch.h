/*
-------------------------------------------------------------------------------
coreIPM/arch.h

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
#ifdef IPMC
#include "lpc23nn.h"
#elif
#include "lpc21nn.h"
#endif

// Interrupt Enable register (VICIntEnable) bit allocation
					// Bit   
#define IER_USB		0x400000	// [22]  
#define IER_ADC1	0x200000	// [21]  
#define IER_BOD		0x100000	// [20]  
#define IER_I2C1	0x080000	// [19]  
#define IER_ADC0	0x040000	// [18]  
#define IER_EINT3	0x020000	// [17]  
#define IER_EINT2	0x010000	// [16]  

#define IER_EINT1	0x008000	// [15]
#define IER_EINT0	0x004000	// [14]
#define IER_RTC		0x002000	// [13]
#define IER_PLL		0x001000	// [12]
#define IER_SPI1	0x000800	// [11]
#define IER_SSP 	0x000800
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

/* INTERRUPT SOURCES */
#define IS_USB		22  
#define IS_ADC1		21  
#define IS_BOD		20  
#define IS_I2C1		19  
#define IS_ADC0		18  
#define IS_EINT3	17  
#define IS_EINT2	16  

#define IS_EINT1	15
#define IS_EINT0	14
#define IS_RTC		13
#define IS_PLL		12
#define IS_SPI1		11
#define IS_SPI0		10
#define IS_I2C0		9
#define IS_PWM0		8

#define IS_UART1	7
#define IS_UART0	6
#define IS_TIMER1	5
#define IS_TIMER0	4
#define IS_ARMCore1	3
#define IS_ARMCore0	2
#define IS_WDT		0

/* BOARD CLOCK */
#define	PCLK		12000000
#define CCLK		60000000


/* INTERRUPT DISABLE/ENABLE MACROS
 * 
 * Usage:
 * 
 * 	unsigned int interrupt_mask = CURRENT_INTERRUPT_MASK;
 * 	DISABLE_INTERRUPTS;
 * 	.
 * 	.
 * 	ENABLE_INTERRUPTS( interrupt_mask );
 */
#define CURRENT_INTERRUPT_MASK	VICIntEnable
//#define DISABLE_INTERRUPTS	( VICIntEnClr = 0xFFFFFFFF )		// disable all interrupts handled by VIC
//#define ENABLE_INTERRUPTS(mask)	( VICIntEnable = mask ) 
#define DISABLE_INTERRUPTS	
#define ENABLE_INTERRUPTS(mask)

/*======================================================================*/
/*			Controller Types				*/
/*======================================================================*/

#define CONTROLLER_TYPE_IPMC 0
#define CONTROLLER_TYPE_SHMC 1

/* change this to IPMI_CONTROLLER_TYPE_SHMC for testing as a shelf controller */
#define CONTROLLER_TYPE CONTROLLER_TYPE_IPMC

