/*
-------------------------------------------------------------------------------
coreIPM/ipmi_pkt.c

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
#include <string.h>
#include "ipmi.h"
#include "ws.h"
#include "strings.h"
#include "ipmi_pkt.h"

extern int g_bridging_enabled, g_channel_number, 
	g_outgoing_protocol, g_outgoing_medium, g_responder_i2c_address;

int fill_pkt( char *payload, int payload_len, char *wrapper, int netfn, int protocol );

/*------------------------------------------------------------------------------
	Fill in the outgoing packet according to protocol.
	If bridging is enabled, we will have to wrap the
	command in a send message command.
	
	Preconditions:
	Postconditions:
 *----------------------------------------------------------------------------*/
void fill_pkt_out( IPMI_WS *ws, IPMI_CMD_REQ *cmd_req, int data_len, int netfn )
/*----------------------------------------------------------------------------*/
{
	int size;

	ws->pkt.req = ws->pkt_out;

	if( g_bridging_enabled ) {
		SEND_MESSAGE_CMD_REQ send_message_req;
		int size;
		
		send_message_req.command = IPMI_CMD_SEND_MESSAGE;
		send_message_req.tracking = BRIDGE_TRACK_REQ;
		send_message_req.encryption = 0;
		send_message_req.authentication = 0;
		send_message_req.channel_number = g_channel_number;

		size = fill_pkt( ( char * )cmd_req, data_len + 1, &send_message_req.message_data, netfn, IPMI_CH_PROTOCOL_IPMB );
		size = fill_pkt( ( char * )&send_message_req, size, ws->pkt_out, NETFN_APP_REQ, g_outgoing_protocol );

		ws->pkt.hdr.netfn = netfn;
		ws->outgoing_protocol = g_outgoing_protocol;
		ws->outgoing_medium = g_outgoing_medium;
		ws->len_out = size;

	} else {
		size = fill_pkt( ( char * )cmd_req, data_len + 1, ws->pkt_out, netfn, g_outgoing_protocol );
		ws->pkt.hdr.netfn = netfn;
		ws->outgoing_protocol = g_outgoing_protocol;
		ws->outgoing_medium = g_outgoing_medium;
		ws->len_out = size;
	}
}

/*------------------------------------------------------------------------------
	Fill in the outgoing packet according to protocol.
	If bridging is enabled, we will have to wrap the
	command in a send message command.
	
	Preconditions:
	Postconditions:
 *----------------------------------------------------------------------------*/
void fill_pkt_out_orig( IPMI_WS *ws, IPMI_CMD_REQ *cmd_req, int data_len, int netfn )
/*----------------------------------------------------------------------------*/
{
	ws->pkt.req = ws->pkt_out;
	ws->pkt.hdr.req_data_len = data_len;
	ws->pkt.hdr.netfn = netfn;

	switch( g_outgoing_protocol ) {
		case IPMI_CH_PROTOCOL_TMODE:
		{
			printf( "fill_pkt_out: IPMI_CH_PROTOCOL_TMODE\n" );
			IPMI_TERMINAL_MODE_REQUEST *req;
			
			ws->outgoing_protocol = IPMI_CH_PROTOCOL_TMODE;
			ws->outgoing_medium = IPMI_CH_MEDIUM_SERIAL;
			ws->len_out = sizeof( IPMI_TERMINAL_MODE_REQUEST ) - TERM_MODE_REQ_MAX_DATA_LEN + ws->pkt.hdr.req_data_len;

			/* fill in the request */
			req = ( IPMI_IPMB_REQUEST * )ws->pkt_out;
			req->netfn = ws->pkt.hdr.netfn;
			req->responder_lun = 0;
			req->req_seq = 0;
			req->bridge = 0;
			memcpy( &req->command, cmd_req, data_len + 1 );
		}
			break;
		case IPMI_CH_PROTOCOL_IPMB:
		{	
			IPMI_IPMB_REQUEST *req;
			
			/* this portion is IPMB specific */
			ws->outgoing_protocol = IPMI_CH_PROTOCOL_IPMB;
			ws->outgoing_medium = IPMI_CH_MEDIUM_IPMB;
			ws->len_out = sizeof( IPMI_IPMB_REQUEST ) - IPMB_REQ_MAX_DATA_LEN + ws->pkt.hdr.req_data_len;
			ws->addr_out = g_responder_i2c_address;

			/* fill in the request */
			req = ( IPMI_IPMB_REQUEST * )ws->pkt_out;
	
			//req->responder_slave_addr = g_responder_i2c_address;
					/* This is the IPMB address of the device that
					   is expected to respond to the message. */

			req->netfn = ws->pkt.hdr.netfn;
					/* This contains the network function of 
                                           the message. For a request command, the 
                                           NetFn is always even. The IPMI and IPMB 
                                           specifications define the legal NetFns. 
                                           Of particular interest is NetFn 2Ch (Request) 
                                           and 2Dh (Response) for the commands stated 
                                           in the PICMG v3 specification. */

			req->responder_lun = 0;	
					/* The Responder LUN (Logical Unit Number) 
					   defines which unit is meant to respond to 
					   the message (see the IPMI specification for 
					   further definition). */

			req->header_checksum = 0;
					/* 2’s complement checksum of preceding 
					   bytes in the connection header. */

			req->requester_slave_addr = 0; /* TODO: FIX !!! */
					/* Requester Slave Address. This is the IPMB 
					   address of the requesting device. */

			req->req_seq = 0;	    /* TODO: FIX!! */
			req->requester_lun = 0;	/* TODO: FIX!! */
					/* Request Sequence/Requester LUN. The Request 
					   Sequence identifier is used by the device(s) 
					   to determine if duplicate requests/responses
					   are received. The Requester LUN provides 
					   the LUN that should receive the response. */

			memcpy( &req->command, cmd_req, data_len + 1 );

			/* data[25]; */ /* 7:N Data Bytes. The Command may be followed 
					   by zero or more data bytes that are command 
					   specific. The maximum N value by IPMI 
					   definition is 31; thus, 25 bytes of 
					   request data are allowed in each request. */
			req->data_checksum = 0;
					/* TODO: Data Checksum. 2’s complement checksum 
					   of preceeding bytes back to, but not 
					   including, the Header Checksum.
					   Note that this is a placeholder and the
					   real checksum needs to go in right after
					   the data */
		}
		break;
	}
}


/*------------------------------------------------------------------------------
	Fill in the outgoing packet according to protocol.
	
payload
------->+-------+ ------+  -
	|cmd	|	|  ^
	+-------+	|  |
	|data	|	| payload_len
	|	|	|  |
	|	|	|  v
	+-------+	|  -
			.
wrapper		     Protocol dependent req.
------->+-------+<-- IPMI_TERMINAL_MODE_REQUEST/IPMI_IPMB_REQUEST
	|	|	.
	|	|<-- initialize protocol dependent request fields
	|	|    netfn, luns, bridge, checksums
	|	|	.
	+-------+ <-----+ copy here
	|cmd	|
	+-------+
	|data	|
	|	|
	+-------+
	|	|
	+-------+
	
	Preconditions:
	Postconditions:
	- returns size of filled area in wrapper
 *----------------------------------------------------------------------------*/
int fill_pkt( char *payload, int payload_len, char *wrapper, int netfn, int protocol )
/*----------------------------------------------------------------------------*/
{

	switch( protocol ) {
		case IPMI_CH_PROTOCOL_TMODE:
		{
			printf( "fill_pkt: IPMI_CH_PROTOCOL_TMODE\n" );
			IPMI_TERMINAL_MODE_REQUEST *req;
			
			/* fill in the request */
			req = ( IPMI_TERMINAL_MODE_REQUEST * )wrapper;
			req->netfn = netfn;
			req->responder_lun = 0;
			req->req_seq = 0;
			req->bridge = 0;
			memcpy( &req->command, payload, payload_len );
		}
			return( sizeof( IPMI_TERMINAL_MODE_REQUEST ) - TERM_MODE_REQ_MAX_DATA_LEN + payload_len - 1 );
			break;
		case IPMI_CH_PROTOCOL_IPMB:
		{	
			IPMI_IPMB_REQUEST *req;
			
			/* fill in the request */
			req = ( IPMI_IPMB_REQUEST * )wrapper;
	
			//req->responder_slave_addr = g_responder_i2c_address;
					/* This is the IPMB address of the device that
					   is expected to respond to the message. */

			req->netfn = netfn;
					/* This contains the network function of 
                                           the message. For a request command, the 
                                           NetFn is always even. The IPMI and IPMB 
                                           specifications define the legal NetFns. 
                                           Of particular interest is NetFn 2Ch (Request) 
                                           and 2Dh (Response) for the commands stated 
                                           in the PICMG v3 specification. */

			req->responder_lun = 0;	
					/* The Responder LUN (Logical Unit Number) 
					   defines which unit is meant to respond to 
					   the message (see the IPMI specification for 
					   further definition). */

			req->header_checksum = calculate_checksum( ( char * )req, 2 );
					/* 2’s complement checksum of preceding 
					   bytes in the connection header. */

			req->requester_slave_addr = 0; /* TODO: FIX !!! */
					/* Requester Slave Address. This is the IPMB 
					   address of the requesting device. */

			req->req_seq = 0;	    /* TODO: FIX!! */
			req->requester_lun = 0;	/* TODO: FIX!! */
					/* Request Sequence/Requester LUN. The Request 
					   Sequence identifier is used by the device(s) 
					   to determine if duplicate requests/responses
					   are received. The Requester LUN provides 
					   the LUN that should receive the response. */

			memcpy( &req->command, payload, payload_len );

			/* data[25]; */ /* 7:N Data Bytes. The Command may be followed 
					   by zero or more data bytes that are command 
					   specific. The maximum N value by IPMI 
					   definition is 31; thus, 25 bytes of 
					   request data are allowed in each request. */
			req->data_checksum = calculate_checksum( &(req->requester_slave_addr),
				       sizeof( IPMI_IPMB_REQUEST ) - IPMB_REQ_MAX_DATA_LEN + payload_len - 4 );
					/* TODO: Data Checksum. 2’s complement checksum 
					   of preceeding bytes back to, but not 
					   including, the Header Checksum.
					   Note that this is a placeholder and the
					   real checksum needs to go in right after
					   the data */
		}
		return( sizeof( IPMI_IPMB_REQUEST ) - IPMB_REQ_MAX_DATA_LEN + payload_len - 1);
		break;
	}
}

/* calculate_checksum()
 * 
 * 	8-bit checksum algorithm: Initialize checksum to 0. For each byte,
 * 	checksum = (checksum + byte) modulo 256. Then checksum = - checksum. 
 * 	When the checksum and the bytes are added together, modulo 256, 
 * 	the result should be 0.
 */
unsigned char
calculate_checksum( char *ptr, int numchar )
{
	char checksum = 0;
	int i;
	
	for( i = 0; i < numchar; i++ ) {
		checksum += *ptr++;
	}
	
	return( -checksum );
}

