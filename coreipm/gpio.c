/*
-------------------------------------------------------------------------------
coreIPM/gpio.c

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
#include "gpio.h"
#include "ipmi.h"
#include "ws.h"
#include "i2c.h"
#include "timer.h"
#include "iopin.h"
//#include "mmcio.h"
#include "module.h"

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
void gpio_led_blink_callback( unsigned char *led_mask );
void gpio_led_on_noreset( unsigned led_mask );
void gpio_led_off_noreset( unsigned led_mask );

/* LEDs, switches, backplane address detection, etc, etc.. */

void gpio_initialize( void ) 
{
	led_state = 0;
	gpio_led_off( GPIO_LED_ALL );
}

/* gpio_get_hardware_setting()
 * 
 * 	Returns a value that is a mapping of configuration switches.
 * 	Currently:
 * 		1 if jumper J8 is unjumpered, and
 * 		0 if jumper J8 is jumpered
 *
 * 	NOTE: MCB2140 dependent
 */ 
int gpio_get_hardware_setting( void )
{
	// get P1.20 (J8) setting
	if( IOPIN1 & 0x00100000 ) {	// unjumpered, logic HIGH
		return 1;
	} else { 			// jumpered, logic LOW
		return 0;
	}
}
/* used for debugging only - call module_get_i2c_address for the real address */
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


/* LED control  */
void gpio_led_on( unsigned led_mask )
{
	long long iopin = 0;

	/* turn off timers if any */
	timer_remove_callout_queue( &led_blink );
	
	led_state |= led_mask;
/*	
	if( led_state & GPIO_LED_0 ) iopin |= LED_0;
	if( led_state & GPIO_LED_1 ) iopin |= LED_1;
	if( led_state & GPIO_LED_2 ) iopin |= LED_2;
	if( led_state & GPIO_LED_3 ) iopin |= LED_3;
	if( led_state & GPIO_LED_4 ) iopin |= LED_4;
	if( led_state & GPIO_LED_5 ) iopin |= LED_5;
	if( led_state & GPIO_LED_6 ) iopin |= LED_6;
	if( led_state & GPIO_LED_7 ) iopin |= LED_7;
	
	iopin_set( iopin );
*/
	module_led_on( led_state );
}


/* LED control  */
void gpio_led_on_noreset( unsigned led_mask )
{
	long long iopin = 0;

	led_state |= led_mask;
/*	
	if( led_state & GPIO_LED_0 ) iopin |= LED_0;
	if( led_state & GPIO_LED_1 ) iopin |= LED_1;
	if( led_state & GPIO_LED_2 ) iopin |= LED_2;
	if( led_state & GPIO_LED_3 ) iopin |= LED_3;
	if( led_state & GPIO_LED_4 ) iopin |= LED_4;
	if( led_state & GPIO_LED_5 ) iopin |= LED_5;
	if( led_state & GPIO_LED_6 ) iopin |= LED_6;
	if( led_state & GPIO_LED_7 ) iopin |= LED_7;
	
	iopin_set( iopin );
*/
	module_led_on( led_state );
}

void gpio_led_off( unsigned led_mask )
{
	long long iopin = 0;

	/* turn off timers if any */
	timer_remove_callout_queue( &led_blink );

	led_state &= ( ~led_mask );
/*
	if( ~led_state & GPIO_LED_0 ) iopin |= LED_0;
	if( ~led_state & GPIO_LED_1 ) iopin |= LED_1;
	if( ~led_state & GPIO_LED_2 ) iopin |= LED_2;
	if( ~led_state & GPIO_LED_3 ) iopin |= LED_3;
	if( ~led_state & GPIO_LED_4 ) iopin |= LED_4;
	if( ~led_state & GPIO_LED_5 ) iopin |= LED_5;
	if( ~led_state & GPIO_LED_6 ) iopin |= LED_6;
	if( ~led_state & GPIO_LED_7 ) iopin |= LED_7;

	iopin_clear( iopin );
*/
	module_led_off( led_state );
}


void gpio_led_off_noreset( unsigned led_mask )
{
	long long iopin = 0;

	led_state &= ( ~led_mask );

/*
	if( ~led_state & GPIO_LED_0 ) iopin |= LED_0;
	if( ~led_state & GPIO_LED_1 ) iopin |= LED_1;
	if( ~led_state & GPIO_LED_2 ) iopin |= LED_2;
	if( ~led_state & GPIO_LED_3 ) iopin |= LED_3;
	if( ~led_state & GPIO_LED_4 ) iopin |= LED_4;
	if( ~led_state & GPIO_LED_5 ) iopin |= LED_5;
	if( ~led_state & GPIO_LED_6 ) iopin |= LED_6;
	if( ~led_state & GPIO_LED_7 ) iopin |= LED_7;

	iopin_clear( iopin );
*/
	module_led_off( led_state );

}

void gpio_all_leds_on( void )
{
//	iopin_set( LED_0 | LED_1 | LED_2 | LED_3 | LED_4 | LED_5 | LED_6 | LED_7  );
}

void gpio_all_leds_off( void )
{
//	iopin_clr( LED_0 | LED_1 | LED_2 | LED_3 | LED_4 | LED_5 | LED_6 | LED_7  );
}

void gpio_toggle_activity_led( void )
{
	if( activity_led_state ) {
		gpio_led_on_noreset( GPIO_ACTIVITY_LED );
		activity_led_state = 0;
	} else {
		gpio_led_off_noreset( GPIO_ACTIVITY_LED );
		activity_led_state = 1;
	}
}

void gpio_led_blink ( unsigned led_mask, 
		unsigned on_period, 	/* in 100ms */
		unsigned off_period, 	/* in 100ms */
		unsigned duration )		/* in 100ms - length of time we'll blink, 0 = forever */
{
	/* turn off any timers before starting a new blink sequence */
	timer_remove_callout_queue( &led_blink );
	gpio_led_on_noreset( led_mask );
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

void
gpio_led_blink_off( unsigned led_mask )
{
	led_blink.led_mask &= ( ~led_mask );

	if( !led_blink.led_mask )
		timer_remove_callout_queue( &led_blink );
}

void gpio_led_blink_callback( unsigned char *arg)
{
	if( led_blink.state ) {
		gpio_led_off_noreset( led_blink.led_mask );
		led_blink.state = 0;
		if( !led_blink.duration ) {
			timer_add_callout_queue( &led_blink, led_blink.off_period, gpio_led_blink_callback, 0 ); 
		} else if( led_blink.duration > led_blink.on_period ) {
			led_blink.duration -= led_blink.on_period;
			timer_add_callout_queue( &led_blink, led_blink.off_period, gpio_led_blink_callback, 0 ); 
		}
	} else {
		gpio_led_on_noreset( led_blink.led_mask );
		led_blink.state = 1;
		if( !led_blink.duration ) {
			timer_add_callout_queue( &led_blink, led_blink.on_period, gpio_led_blink_callback, 0 ); 
		} else if( led_blink.duration > led_blink.off_period ) {
			led_blink.duration -= led_blink.on_period;
			timer_add_callout_queue( &led_blink, led_blink.on_period, gpio_led_blink_callback, 0 ); 
		} else {
			gpio_led_off_noreset( led_blink.led_mask );
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

int
gpio_get_handle_switch_state( void )
{
	// TODO depends on the config
	return HANDLE_SWITCH_CLOSED;
}
