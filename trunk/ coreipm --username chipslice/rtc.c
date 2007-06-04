/*
-------------------------------------------------------------------------------
coreIPM/rtc.c

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

#include "rtc.h"

/*
Time is an unsigned 32-bit value representing the local time as the number of 
seconds from 00:00:00, January 1, 1970. 
The timestamps used for SDR and SEL records are assumed to be specified in 
relative local time. 
0xFFFFFFFF indicates an invalid or unspecified time value.
0x00000000 through 0x20000000 are used for timestamping events that occur after
the initialization of the System Event Log device up to the time that the 
timestamp is set with the system time value. Thus, these timestamp values
are relative to the completion of the SEL device’s initialization, not 
January 1, 1970.
*/
unsigned int
rtc_get_timestamp( void )
{
	return 0;
}
