/*
-------------------------------------------------------------------------------
coreIPM/ws.h

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

/* Working Set states */
#define WS_FREE				0x1	/* ws free */
#define WS_PENDING			0x2	/* ws not in any queue, ready for use */
#define WS_ACTIVE_IN			0x3	/* ws in incoming queue, ready for ipmi processing */
#define WS_ACTIVE_IN_PENDING		0x4	/* ws in use by the ipmi layer */
#define WS_ACTIVE_MASTER_WRITE		0x5	/* ws in outgoing queue */
#define WS_ACTIVE_MASTER_WRITE_PENDING	0x6	/* outgoing request in progress */
#define WS_ACTIVE_MASTER_WRITE_SUCCESS  0x7
#define WS_ACTIVE_MASTER_READ		0x8
#define WS_ACTIVE_MASTER_READ_PENDING	0x9

#define WS_ARRAY_SIZE	16
#define WS_BUF_LEN 32

#define WS_FL_GENERAL_CALL	1
#define WS_FL_REPEATED_START	2

/* transport layer completion codes */
#define XPORT_REQ_NOERR 	0 
#define XPORT_REQ_ERR		1
#define XPORT_RESP_NOERR	2
#define XPORT_RESP_ERR		3

void ws_set_state( IPMI_WS *, unsigned );
void ws_init( void );
IPMI_WS *ws_alloc( void );
void ws_free( IPMI_WS *ws );
IPMI_WS *ws_get_elem( unsigned state );
void ws_set_state( IPMI_WS * ws, unsigned state );
void ws_process_work_list( void );
IPMI_WS *ws_get_elem_seq( uchar seq, IPMI_WS* ws_ignore );
void ws_process_incoming( IPMI_WS *ws );


