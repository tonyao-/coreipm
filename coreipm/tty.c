/*
-------------------------------------------------------------------------------
coreIPM/tty.c

Author: Gokhan Sozmen
-------------------------------------------------------------------------------
Copyright (C) 2007-2009 Gokhan Sozmen
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

#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/signal.h>
#include <sys/types.h>
#include "ipmi.h"

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

volatile int STOP=FALSE; 

void tty_signal_handler( int status );   /* definition of signal handler */
int wait_flag = TRUE;       /* TRUE while no signal received */
int fd;
struct termios oldtio;

void
tty_init( void )
{
	int c, res;
	struct termios newtio;
	struct sigaction saio;           /* definition of signal action */

	/* open the device to be non-blocking (read will return immediatly) */
	fd = open( MODEMDEVICE, O_RDWR | O_NOCTTY | O_NONBLOCK );
	if ( fd < 0 ) {
		perror( MODEMDEVICE ); 
		exit( -1 );
	}

	/* install the signal handler before making the device asynchronous */
	saio.sa_handler = tty_signal_handler;
	//saio.sa_mask = 0;
	saio.sa_flags = 0;
	saio.sa_restorer = NULL;
	sigaction( SIGIO, &saio, NULL );
  
	/* allow the process to receive SIGIO */
	fcntl( fd, F_SETOWN, getpid() );
	
	/* Make the file descriptor asynchronous (the manual page says only 
	   O_APPEND and O_NONBLOCK, will work with F_SETFL...) */
	fcntl( fd, F_SETFL, FASYNC );

	tcgetattr( fd, &oldtio ); /* save current port settings */

	/* set new port settings for canonical input processing */
	newtio.c_cflag = BAUDRATE | CRTSCTS | CS8 | CLOCAL | CREAD;
	newtio.c_iflag = IGNPAR | ICRNL;
	newtio.c_oflag = 0;
	newtio.c_cc[VMIN] = 1;
	newtio.c_cc[VTIME] = 0;
	tcflush( fd, TCIFLUSH );
	tcsetattr( fd, TCSANOW, &newtio );
}

/***************************************************************************
* signal handler. sets wait_flag to FALSE, to indicate above loop that     *
* characters have been received.                                           *
***************************************************************************/

void
tty_signal_handler( int status )
{
	int res;
	char buf[255];
	
 	/* after receiving SIGIO, set wait_flag = FALSE, input is available
	 * and can be read */
	printf( "received SIGIO signal.\n" );
	wait_flag = FALSE;
	res = read( fd, buf, 255 );
	buf[res] = 0;
	printf( ":%s:%d\n", buf, res );
	wait_flag = TRUE;      /* wait for new input */
}

void
tty_restore( void )
{
	/* restore old port settings */
	tcsetattr( fd, TCSANOW, &oldtio);
}

void 
serial_tm_send_debug( IPMI_WS *ws )
/*----------------------------------------------------------------------------*/
{
	int i;
	
	printf( "[" );
	for( i = 0; i < ws->len_out; i++ ) { 
		printf( "%2.2x", ws->pkt_out[i] );
		if( i < ( ws->len_out - 1 ) )
			printf( " " );
	}
	printf( "]\n" );
	ws_free( ws );
}

void 
serial_tm_send( IPMI_WS *ws )
/*----------------------------------------------------------------------------*/
{
	int i;
	char buf[128];
	char *ptr = buf;
	int pos = 0;
	
	pos = sprintf( ptr, "[" );
	ptr += pos;	
	for( i = 0; i < ws->len_out; i++ ) {
		pos = sprintf( ptr, "%2.2x", ws->pkt_out[i] );
		ptr += pos;	
		if( i < ( ws->len_out - 1 ) ) {
			pos = sprintf( ptr, " " );
			ptr += pos;
		}
	}
	sprintf( ptr, "]\n" );
	//printf( "%s", buf );
	write( fd, buf, strlen( buf ) );
	ws_free( ws );
}

