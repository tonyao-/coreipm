/*
-------------------------------------------------------------------------------
coreIPM/gpio.h

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

/* LED bit masks */
#define GPIO_LED_0	0x01	
#define GPIO_LED_1	0x02
#define GPIO_LED_2	0x04
#define GPIO_LED_3	0x08
#define GPIO_LED_4	0x10
#define GPIO_LED_5	0x20
#define GPIO_LED_6	0x40
#define GPIO_LED_7	0x80
#define GPIO_LED_ALL	0xff

#define GPIO_ACTIVITY_LED	GPIO_LED_1
#define GPIO_IDENTIFY_LED	GPIO_LED_1

/* do mapping from ATCA FRU LEDs to physical LEDs */
#define GPIO_FRU_LED_BLUE	GPIO_LED_0
#define GPIO_FRU_LED1		GPIO_LED_1
#define GPIO_FRU_LED2		GPIO_LED_2
#define GPIO_FRU_LED3		GPIO_LED_3

void gpio_initialize( void );
int gpio_get_i2c_address( int address_type );
void gpio_led_on( unsigned led_mask );
void gpio_led_off( unsigned led_mask );
void gpio_all_leds_on( void );
void gpio_all_leds_off( void );
void gpio_toggle_activity_led( void );
void gpio_led_blink( unsigned led_mask, 
	unsigned on_period, unsigned off_period, unsigned duration );
void gpio_power_off( void );
void gpio_power_on( void );
void gpio_warm_reset( void );
void gpio_cold_reset( void );
unsigned gpio_get_power_state( void );
int gpio_get_handle_switch_state( void );
int gpio_get_hardware_setting( void );

