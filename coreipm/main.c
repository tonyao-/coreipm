/*
-------------------------------------------------------------------------------
coreIPM/main.c

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
#include "ipmi.h"
#include "debug.h"
#include "ws.h"
#include "timer.h"
#include "gpio.h"
#include "module.h"
#include "serial.h"
#include "i2c.h"
#include "iopin.h"

extern unsigned long lbolt;
/*==============================================================
 * main()
 *==============================================================*/
int main()
{
	unsigned long time;

	/* Initialize system */
	ws_init();
	iopin_initialize();
	gpio_initialize();
	timer_initialize();
	i2c_initialize();
	uart_initialize();
	ipmi_initialize();
	module_init();

	dprintf( DBG_I2C | DBG_LVL1, "Hello World");
	
	time = lbolt;
	
	/* Do forever */
	while( 1 )
	{
		/* Blink system activity LEDs once every second */
		if( ( time + 2 ) < lbolt ) {
			time = lbolt;
			gpio_toggle_activity_led();
		}
		ws_process_work_list();
		terminal_process_work_list();
		timer_process_callout_queue();
	}
}

