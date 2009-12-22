/*
-------------------------------------------------------------------------------
coreIPM/ipmcio.c

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
#include "timer.h"
#include "ipmcio.h"
#include "serial.h"
#include "debug.h"
#include "iopin.h"



void iopin_initialize( void ) 
{

	/* Initialize Pin Connect Block */
	PINSEL0 |=  
		PS0_P0_0_TXD_UART_0 |	
		PS0_P0_1_RXD_UART_0 |
		PS0_P0_2_SCL_I2C_0  |
		PS0_P0_3_SDA_I2C_0  |
		PS0_P0_4_SCK_0      |
		PS0_P0_5_MISO_0     |
		PS0_P0_6_MOSI_0     |
		PS0_P0_7_PWM_2      |
		PS0_P0_8_TDX_UART_1 |
		PS0_P0_9_RDX_UART_1 |
		PS0_P0_10_AD_1_2    |
		PS0_P0_11_SCL_I2C_1 |
		PS0_P0_12_GPIO      |
		PS0_P0_13_GPIO      |
		PS0_P0_14_SDA_I2C_1 |
		PS0_P0_15_AD_1_5;		
	
	PINSEL1 |=  
		PS1_P0_16_EINT_0    |
		PS1_P0_17_GPIO      |
		PS1_P0_18_GPIO      |
		PS1_P0_19_GPIO      |
		PS1_P0_20_GPIO      |
		PS1_P0_21_GPIO      |
		PS1_P0_22_GPIO      |
		PS1_P0_23_GPIO      |
		PS1_P0_25_GPIO      |
		PS1_P0_28_GPIO      |
		PS1_P0_29_GPIO      |
		PS1_P0_30_GPIO      |
		PS1_P0_31_GPIO;		

	PINSEL2 |= 
		PS2_P1_16_25_GPIO   |	// Pins P1.25-16 are used as GPIO pins.
		PS2_P1_26_36_DEBUG;	// Pins P1.36-26 are used as a Debug port.

	/* Set the direction of each GPIO port pin, setting the bit makes it an output */
	IODIR0 = ( unsigned int ) ( 0 );

	IODIR1 = ( unsigned int ) (
		GPIO_0		|	
		GPIO_1		|
		GPIO_2		|
		GPIO_3		|
		LED_1		|
		BLUE_LED ); 

	// Read backplane address

	// discover & initialize i2c interfaces

	// payload power-up
	
	// e-keying
		
	// determine if there are AMC cards to manage
	
	// AMC management signals handler
	
	
}

/* Module specific handlers
 *
 * - hot swap switch
 * - m-states
 *   event receiver for AMC modules
 */
void
module_led_on( unsigned led_state )
{
	long long iopin = 0;

	if( led_state & GPIO_LED_0 ) iopin |= LED_0;
	if( led_state & GPIO_LED_1 ) iopin |= LED_1;
	iopin_set( iopin );

}

void
module_led_off( unsigned led_state )
{
	long long iopin = 0;

	if( ~led_state & GPIO_LED_0 ) iopin |= LED_0;
	if( ~led_state & GPIO_LED_1 ) iopin |= LED_1;

	iopin_clear( iopin );
}

void
module_payload_on( void )
{
	iopin_set( PAYLOAD_POWER );
}

void
module_payload_off( void )
{
	iopin_clear( PAYLOAD_POWER );

}



