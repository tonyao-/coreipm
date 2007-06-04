/*
-------------------------------------------------------------------------------
coreIPM/gpio.c

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


#include "arch.h"
#include "gpio.h"
#include "ipmi.h"
#include "ws.h"
#include "i2c.h"
#include "timer.h"

#define DEBUG_I2C_ADDRESS_1	0x20
#define DEBUG_I2C_ADDRESS_2	0x28

unsigned led_state = 0;
unsigned activity_led_state = 0;
unsigned power_state = 0;

typedef struct led_blink {
	unsigned on_period;	/* in 100 ms */
	unsigned off_period;	/* in 100 ms */
	unsigned duration;	/* in 100 ms */
	unsigned led_mask;
	unsigned state;
} LED_BLINK;

LED_BLINK led_blink;
void gpio_led_blink_callback( char *led_mask );

/* LEDs, switches, backplane address detection, etc, etc.. */

void gpio_initialize( void ) 
{
	/* Remap interrupt vectors to SRAM */
	/* MEMMAP=0x2;	startup code should take care of this ?		  */

	/* Initialize GPIO ports to be used as indicators */
	IODIR1 = 0x00FF0000;  /* on MBC2140 & MBC2103 boards P1.16..23 connected to LEDs  */
	IOSET1 = 0x00FF0000;  /* turn on all LEDs */

	/* Initialize Pin Connect Block */
	/*
	 * LPC2148
	 * Pin function Select register 0 (PINSEL0) bit description
	 * Bit   Symbol  Value Function
	 * 5:4   P0.2    01    SCL0 (I2C0)
	 * 7:6   P0.3    01    SDA0 (I2C0)
	 * 23:22 P0.11   11    SCL1 (I2C1)
	 * 29:28 P0.14   11    SDA1 (I2C1)
	 *
	 * LPC2101/02/03
	 * Pin function Select register 0 (PINSEL0) bit description
	 * Bit   Symbol  Value Function
	 * 5:4   P0.2    01    SCL0 (I2C0)
	 * 7:6   P0.3    01    SDA0 (I2C0)
	 * Pin function select register 1 (PINSEL1) bit description
	 * 3:2   P0.17   01    SCL1 (I2C1)
	 * 5:4   P0.18   01    SDA1 (I2C1)
	 * 
	 */ 
	
	PINSEL0 = 0x50;		/* Setup i2c pins: P0.2 = SCL0, P0.3 = SDA0
				 * All others are GPIO. This will work for both
				 * 2103 & 2148 */
	led_state = 0;
	gpio_led_off( GPIO_LED_ALL );
}

int gpio_get_i2c_address( int address_type )
{
	switch( address_type ) {
		case I2C_ADDRESS_LOCAL:
			if( CONTROLLER_TYPE == CONTROLLER_TYPE_IPMC )
				return DEBUG_I2C_ADDRESS_1;
			else
				return DEBUG_I2C_ADDRESS_2;
			break;
		case I2C_ADDRESS_REMOTE:
			if( CONTROLLER_TYPE == CONTROLLER_TYPE_IPMC )
				return DEBUG_I2C_ADDRESS_2;
			else
				return DEBUG_I2C_ADDRESS_1;
			break;
		default:
			return DEBUG_I2C_ADDRESS_1;
			break;
	}
}


/* On LPC2140 board P1.16..23 connected to LEDs  */
void gpio_led_on( unsigned led_mask )
{
	led_state |= led_mask;
	IOSET1 = ( 0xff & led_state ) << 16;
}

void gpio_led_off( unsigned led_mask )
{
	led_state &= ( ~led_mask );
	IOCLR1 = ( 0xff & ( ~led_state ) ) << 16;
}

void gpio_all_leds_on( void )
{
	IOSET1 = 0x00FF0000;
}

void gpio_all_leds_off( void )
{
	IOCLR1 = 0x00FF0000;
}

void gpio_toggle_activity_led( void )
{
	if( activity_led_state ) {
		gpio_led_on( GPIO_ACTIVITY_LED );
		activity_led_state = 0;
	} else {
		gpio_led_off( GPIO_ACTIVITY_LED );
		activity_led_state = 1;
	}
}

void gpio_led_blink ( unsigned led_mask, 
		unsigned on_period, 	/* in 100ms */
		unsigned off_period, 	/* in 100ms */
		unsigned duration )		/* in 100ms - length of time we'll blink, 0 = forever */
{
	gpio_led_on( led_mask );
	led_blink.state = 1;

	if( on_period )
		led_blink.on_period = on_period;
	else 
		led_blink.on_period = 1;

	if( off_period )
		led_blink.off_period = off_period;
	else
		led_blink.off_period = 1;

	led_blink.led_mask = led_mask;
	led_blink.duration = duration;
	timer_add_callout_queue( &led_blink, on_period, gpio_led_blink_callback, 0 ); 
}

void gpio_led_blink_callback( unsigned char *arg)
{
	if( led_blink.state ) {
		gpio_led_off( led_blink.led_mask );
		led_blink.state = 0;
		if( !led_blink.duration ) {
			timer_add_callout_queue( &led_blink, led_blink.off_period, gpio_led_blink_callback, 0 ); 
		} else if( led_blink.duration > led_blink.on_period ) {
			led_blink.duration -= led_blink.on_period;
			timer_add_callout_queue( &led_blink, led_blink.off_period, gpio_led_blink_callback, 0 ); 
		}
	} else {
		gpio_led_on( led_blink.led_mask );
		led_blink.state = 1;
		if( !led_blink.duration ) {
			timer_add_callout_queue( &led_blink, led_blink.on_period, gpio_led_blink_callback, 0 ); 
		} else if( led_blink.duration > led_blink.off_period ) {
			led_blink.duration -= led_blink.on_period;
			timer_add_callout_queue( &led_blink, led_blink.on_period, gpio_led_blink_callback, 0 ); 
		} else {
			gpio_led_off( led_blink.led_mask );
			led_blink.state = 0;
		}
	}
}

void gpio_toggle_led( unsigned led_mask )
{

}

void gpio_power_off( void )
{
	power_state = POWER_STATE_OFF;
}

void gpio_power_on( void )
{
	power_state = POWER_STATE_ON;
}

void gpio_warm_reset( void )
{

}

void gpio_cold_reset( void )
{

}

unsigned gpio_get_power_state( void )
{
	return power_state;
}

gpio_get_handle_switch_state( void )
{
	return HANDLE_SWITCH_CLOSED;
}
