/*
-------------------------------------------------------------------------------
coreIPM/debug.h

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

/* subsystem */
#define DBG_IPMI	0x0100
#define DBG_SERIAL	0x0200
#define DBG_I2C		0x0400
#define DBG_WD		0x0800
#define DBG_TIMER	0x1000
#define DBG_LAN		0x2000
#define DBG_GPIO	0x4000
#define DBG_WS		0x8000

/* qualifier */
#define DBG_INOUT	0x01	/* function ingress & egress */
#define DBG_ERR		0x02	/* error messages */
#define DBG_WARN	0x04	/* warning messages */
#define DBG_LVL1	0x08	/* operation information messages, mainly used for debugging */
#define DBG_LVL2	0x10

//#define USE_DPRINTF

#ifdef USE_DPRINF
void dprintf(unsigned flags, char *fmt, ...);
#endif
void putstr( char *str );
void dputstr( unsigned flags, char *str);

#ifndef USE_DPRINTF
#define dprintf
#endif

void puthex( unsigned char ch );
