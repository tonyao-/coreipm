/*
-------------------------------------------------------------------------------
coreIPM/iopin.c

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
#include "lpc21nn.h"
#include "iopin.h"

void
iopin_set( unsigned long long bit )
{
	IOSET1 = ( unsigned )( bit >> 32 );
	IOSET0 = ( unsigned )bit;	
}

void
iopin_clear( unsigned long long bit )
{
	IOCLR1 = ( unsigned )( bit >> 32 );
	IOCLR0 = ( unsigned )bit;	
}

unsigned char
iopin_get( unsigned long long bit )
{
	unsigned char retval;

	if( bit >= 0x100000000 )
		retval = ( IOPIN1 & ( unsigned )( bit >> 32 ) ) ? 1: 0;
	else
		retval = ( IOPIN0 & ( unsigned )bit ) ? 1: 0 ;

	return retval;
}

/* set & reset IO bits simultaneously
 * Only bit positions which have a 1 in the mask will be changed */
void
iopin_assign( unsigned long long bit, unsigned long long mask )
{
	unsigned int reg0, reg1;

	reg0 = IOPIN0;
	reg1 = IOPIN1;

	IOPIN0 = ( reg0 & !mask ) | ( bit & mask );
	IOPIN1 = ( reg1 & !( mask >>32 ) ) | ( ( bit & mask ) >> 32 );
}
