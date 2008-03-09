/*
-------------------------------------------------------------------------------
coreIPM/debug.c

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


#include "stdio.h"
#include "stdarg.h"
#include "string.h"
#include "debug.h"
#include "ipmi.h"
#include "strings.h"

unsigned global_debug_setting = DBG_IPMI | DBG_SERIAL | DBG_I2C | DBG_WD | DBG_TIMER | DBG_LAN | DBG_GPIO | DBG_WS | DBG_ERR | DBG_LVL1;

#ifdef USE_DPRINTF
void
dprintf(unsigned flags, char *fmt, ...)
{
	va_list argp;

	if( ( ( flags & global_debug_setting ) >> 8 )  && ( ( flags & global_debug_setting ) & 0x0f ) ) {
		va_start( argp, fmt );
		vprintf( fmt, argp );
		va_end( argp );
	}
}
#endif

void
putstr( char *str )
{
	int i, len;
	
	len = strlen( str );
	for( i = 0; i < len; i++ ) {
		putchar( str[i] );
	}
}

void
dputstr( unsigned flags, char *str)
{

	if( ( ( flags & global_debug_setting ) >> 8 )  && ( ( flags & global_debug_setting ) & 0x0f ) ) {
		putstr( str );
	}
}

