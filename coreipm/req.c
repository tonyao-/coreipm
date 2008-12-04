/*
-------------------------------------------------------------------------------
coreIPM/req.c

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


#include <string.h>
#include "ipmi.h"
#include "ws.h"
#include "req.h"

void fill_pkt_out( IPMI_WS *ws, GENERIC_CMD_REQ *cmd_req, REQ_PARAMS *params );
int fill_pkt( char *payload, int payload_len, char *wrapper, int netfn, int protocol, unsigned char dev_addr );



int 
req_send( GENERIC_CMD_REQ *cmd_req, REQ_PARAMS *params )
/*----------------------------------------------------------------------------*/
{
	IPMI_WS *ws;
	
	ws = ws_alloc();
	
	if( !ws ) {
		return -1;
	}

	fill_pkt_out( ws, cmd_req, params );

	/* the completion function will be called by the transport layer
	 * completion routine after the xfer has completed. It is up to the 
	 * test framework to keep track of request/response pairs using the 
	 * sequence numbers */
	ws->ipmi_completion_function = params->completion_function; 

	/* change ws state, work list processing will do the rest */
	ws_set_state( ws, WS_ACTIVE_MASTER_WRITE );

	return 0;
}


/*------------------------------------------------------------------------------
	Fill in the outgoing packet according to protocol.
	If bridging is required, we will have to wrap the
	command in a send message command.
	
	Preconditions:
	Postconditions:
 *----------------------------------------------------------------------------*/
void fill_pkt_out( IPMI_WS *ws, GENERIC_CMD_REQ *cmd_req, REQ_PARAMS *params )
/*----------------------------------------------------------------------------*/
{
	int size;

	ws->pkt.req = ( IPMI_CMD_REQ* )ws->pkt_out;
	ws->pkt.hdr.ws = ( char * )ws;

	if( params->bridged ) {
		SEND_MESSAGE_CMD_REQ send_message_req;
		int size;
		
		send_message_req.command = IPMI_CMD_SEND_MESSAGE;
		send_message_req.tracking = BRIDGE_TRACK_REQ;
		send_message_req.encryption = 0;
		send_message_req.authentication = 0;
		send_message_req.channel_number = params->bridge_addr[0];

		size = fill_pkt( ( char * )cmd_req, 
				 params->req_data_len + 1,
				 &send_message_req.message_data, 
				 params->netfn, 
				 params->bridge_protocol,
				 params->addr_out[0] );
		ws->pkt.hdr.req_data_len = size - 1;
		
		size = fill_pkt( ( char * )&send_message_req,
			         size,
				 ws->pkt_out, 
				 NETFN_APP_REQ, 
				 params->outgoing_protocol,
				 params->bridge_addr[0] );
		
		ws->pkt.hdr.netfn = NETFN_APP_REQ;
		ws->outgoing_protocol = params->bridge_protocol;
		ws->outgoing_medium = params->bridge_medium;
		ws->len_out = size;

	} else {
		size = fill_pkt( ( char * )cmd_req, 
				 params->req_data_len + 1,
				 ws->pkt_out,
				 params->netfn,
				 params->outgoing_protocol,
				 params->addr_out[0] );
		
		ws->pkt.hdr.req_data_len = size - 1;
		ws->pkt.hdr.netfn = params->netfn;
		ws->outgoing_protocol = params->outgoing_protocol;
		ws->outgoing_medium = params->outgoing_medium;
		ws->len_out = size;
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
int fill_pkt( char *payload, int payload_len, char *wrapper, int netfn, int protocol, unsigned char dev_addr )
/*----------------------------------------------------------------------------*/
{

	switch( protocol ) {
		case IPMI_CH_PROTOCOL_TMODE:
		{
			// printf( "fill_pkt: IPMI_CH_PROTOCOL_TMODE\n" );
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
			IPMI_IPMB_REQUEST *ipmb_req;
			
			/* fill in the request */
			ipmb_req = ( IPMI_IPMB_REQUEST * )wrapper;
			ipmb_req->netfn = netfn;
			ipmb_req->responder_lun = 0;	
			ipmb_req->header_checksum = -( *( char * )ipmb_req + dev_addr );
			ipmb_req->requester_slave_addr = 0; /* TODO: FIX !!! */
			ipmb_req->req_seq = 0;	    /* TODO: FIX!! */
			ipmb_req->requester_lun = 0;	/* TODO: FIX!! */
			memcpy( &ipmb_req->command, payload, payload_len );
			ipmb_req->data_checksum = ipmi_calculate_checksum( &(ipmb_req->requester_slave_addr),
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
	return 0;
}
