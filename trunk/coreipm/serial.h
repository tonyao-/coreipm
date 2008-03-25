/*
-------------------------------------------------------------------------------
coreIPM/serial.h

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
#define UART_0	0
#define	UART_1	1

#define UART_ITLA	UART_1
#define UART_DEBUG	UART_0

#define UART_FILTER_RAW		0
#define UART_FILTER_TERM	1

#define UART_PORT_COUNT		2

#ifndef EOF
#define EOF -1
#endif


void uart_initialize( void );
#if defined (__CA__)
int putchar( int ch );
#elif defined (__CC_ARM)  
int sendchar( int ch );
#endif
//int putc( int ch, int handle );
//int fputs ( const char * str, int handle );
int putchar_0( int ch );
int putchar_1( int ch );
//int getchar( void );
int getchar_0( void );
int getchar_1( void );
void terminal_process_work_list( void );
void serial_tm_send( unsigned char *ws ); 
int serial_get_handle( unsigned char port_name );
//int fflush( int handle );
