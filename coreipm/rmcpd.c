
/*
-------------------------------------------------------------------------------
coreIPM/rmcpd.c

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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "rmcp.h"

#define MYPORT PRIMARY_RMCP_PORT // the port users will be connecting to
#define MAXBUFLEN 100



void 
*get_in_addr( struct sockaddr *sa )
{
	if( sa->sa_family == AF_INET ) {
		return &( ( ( struct sockaddr_in * )sa )->sin_addr );
	}
	return &( ( ( struct sockaddr_in6 * )sa )->sin6_addr );
}

int 
rmcpd_init_listener( void )
{
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;
	struct sockaddr_storage their_addr;
	char buf[MAXBUFLEN];
	size_t addr_len;
	char s[INET6_ADDRSTRLEN];

	memset( &hints, 0, sizeof hints );
	hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; // use my IP
	if( ( rv = getaddrinfo( NULL, MYPORT, &hints, &servinfo ) ) != 0 ) {
		fprintf( stderr, "getaddrinfo: %s\n", gai_strerror( rv ) );	
		return 1;
	}
	
	// loop through all the results and bind to the first we can
	for( p = servinfo; p != NULL; p = p->ai_next ) {
		if( ( sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol ) ) == -1 ) {
			perror( "rmcpd: socket" );
			continue;
		}
		if( bind( sockfd, p->ai_addr, p->ai_addrlen ) == -1 ) {
			close( sockfd );
			perror( "rmcpd: bind" );
			continue;
		}
		break;
	}
	
	if( p == NULL ) {
		fprintf( stderr, "listener: failed to bind socket\n" );
		return 2;
	}

	freeaddrinfo( servinfo );
	printf( "rmcpd: waiting to recvfrom...\n" );
	addr_len = sizeof their_addr;
	if( ( numbytes = recvfrom( sockfd, buf, MAXBUFLEN - 1 , 0,
		( struct sockaddr * )&their_addr, &addr_len ) ) == -1 ) {
			perror( "recvfrom" );
			exit( 1 );
	}
	printf( "rmcpd: got packet from %s\n",
			inet_ntop( their_addr.ss_family,
				get_in_addr( ( struct sockaddr * )&their_addr ),
				s, sizeof s ) );

	printf( "rmcpd: packet is %d bytes long\n", numbytes );
	buf[numbytes] = '\0';
	printf( "rmcpd: packet contains \"%s\"\n", buf );
	close( sockfd );
	return 0;
}

