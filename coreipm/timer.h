/*
-------------------------------------------------------------------------------
coreIPM/timer.h

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

#define HZ 10	/* number of lbolts in a second */
#define CQE_FREE	0
#define CQE_ACTIVE	1
#define CQE_PENDING	3

extern void timer_initialize(void);
extern void timer_process_callout_queue( void );
extern int timer_add_callout_queue( 
	void *handle,
	unsigned long ticks, 
	void(*func)( unsigned char *), 
	unsigned char *arg );
extern void timer_remove_callout_queue( void *handle );
unsigned long timer_get_expiration_time( void *handle );
void timer_reset_callout_queue( void *handle, unsigned long ticks );



