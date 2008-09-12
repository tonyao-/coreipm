/*
-------------------------------------------------------------------------------
coreIPM/ipmi.c

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

#include "fan.h"
#include "gpio.h"
#include "ipmi.h"
#include "ws.h"
#include "i2c.h"
#include "timer.h"
#include "debug.h"
#include "wd.h"
#include "stdio.h"
#include "string.h"
#include "stdarg.h"
#include "debug.h"
#include "picmg.h"
#include "event.h"
#include "sensor.h"
#include "module.h"
#include <string.h>

#define FRU_INVENTORY_CACHE_ARRAY_SIZE	4

/*==============================================================*/
/* Local Variables						*/
/*==============================================================*/
#define WD_STATE_UNINITIALIZED					0
#define WD_STATE_INITIALIZED_TIMER_STOPPED			1
#define WD_STATE_TIMER_RUNNING					2
#define WD_STATE_TIMER_RUNNING_POST_PRE_TIMEOUT_INTERRUPT	3

#define FRU_LOCATOR_TABLE_SIZE	16
#define DUMP_RESPONSE


typedef struct watchdog_info {
	unsigned timer_handle;
	uchar state;
	uchar timer_running;
	uchar dont_log; /* cleared after every system hard reset or timer timeout. */
	uchar timer_use;
	uchar pre_timeout_intr;
	uchar timeout_action;
	uchar pre_timeout_interval;
	unsigned long pre_timeout_interval_ticks;
	/* The timeout use expiration flags retain their state across system
	 * resets and power cycles, as long as the BMC remains powered. */
	uchar timer_use_exp_fl;  
	uchar init_countdown_lsb;
	uchar init_countdown_msb;
	unsigned long init_countdown_ticks;
	uchar pre_timeout_enabled;
	uchar wd_initialized;
	uchar timer_use_exp_fl_clr;
} WATCHDOG_INFO;

	
struct {
	uchar fru_dev_id;
	uchar i2c_address;
} fru_locator_table[FRU_LOCATOR_TABLE_SIZE] = { {0, 0} };

WATCHDOG_INFO wd_timer;
FRU_CACHE fru_inventory_cache[FRU_INVENTORY_CACHE_ARRAY_SIZE];
CHANNEL channel_table[16] = { {0, 0} };
unsigned device_status = DEV_STATUS_READY;
/*==============================================================*/
/* Local Function Prototypes					*/
/*==============================================================*/

int  ipmi_verify_checksum( IPMI_PKT *pkt );
void ipmi_process_response( IPMI_PKT *pkt, unsigned char completion_code );
void ipmi_process_request( IPMI_PKT *pkt );
void ipmi_default_response( IPMI_PKT *pkt );
void ipmi_process_app_req( IPMI_PKT *pkt );
void ipmi_process_event_req( IPMI_PKT *pkt );
void ipmi_process_fw_req( IPMI_PKT *pkt );
void ipmi_get_fru_inventory_area_info( IPMI_PKT *pkt );
void ipmi_read_fru_data( IPMI_PKT *pkt );
void fru_read_complete( void *fru_ws, int status );
void ipmi_write_fru_data( IPMI_PKT *pkt );
void ipmi_wd_expired( uchar *arg );
void ipmi_process_nvstore_req( IPMI_PKT *pkt );
void ipmi_send_message_cmd_complete( void *ws, int status );
void ipmi_send_message_cmd( IPMI_PKT *pkt );
void ipmi_seq_free( uchar seq );
void init_fru_cache( void );

/*==============================================================*/
/* Functions							*/
/*==============================================================*/

void
ipmi_initialize( void )
{
	/* Initialize the channel tables */
	channel_table[IPMI_CH_NUM_PRIMARY_IPMB].protocol = IPMI_CH_PROTOCOL_IPMB;
	channel_table[IPMI_CH_NUM_PRIMARY_IPMB].medium = IPMI_CH_MEDIUM_IPMB;
	
	channel_table[IPMI_CH_NUM_CONSOLE].protocol = IPMI_CH_PROTOCOL_TMODE;
	channel_table[IPMI_CH_NUM_CONSOLE].medium = IPMI_CH_MEDIUM_SERIAL;

	channel_table[IPMI_CH_NUM_SYS_INTERFACE].protocol = IPMI_CH_PROTOCOL_TMODE;
	channel_table[IPMI_CH_NUM_SYS_INTERFACE].medium = IPMI_CH_MEDIUM_SERIAL;

	init_fru_cache();
}

uchar seq_array[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
/* sequence number generator */
uchar
ipmi_get_next_seq( uchar *seq )
{
	unsigned short i;

	/* return the first free sequence number */
	for( i = 0; i < 16; i++ ) {
		if( !seq_array[i] ) { 
			seq_array[i] = 1;
			*seq = i;
			return( 1 );
		}
	}
	return( 0 );   	
}

void
ipmi_seq_free( uchar seq )
{
	seq_array[seq] = 0;
}


/* 
 * ipmi_process_pkt()
 * 
 * 	We received a packet from an IPMI interface, this could be
 * 	a request or a response to a request we sent.
 * 	
 * 	For a request:  
 * 	1) check formatting and checksums.
 * 	2) check responder slave address to determine if this request
 * 	needs to be routed. 
 * 	3) If the request is local, check if the LUN is valid. 
 * 	A management controller that gets a request to an invalid 
 * 	(unimplemented) LUN must return an error completion code using 
 * 	that LUN as the responder’s LUN (RsLUN) in the response.
 * 	4) If everything OK call ipmi_process_request()
 * 	5) If ipmi_process_request() has a response ready, set ws state.
 * 	Otherwise if this is a delayed response ( completion code ==
 * 	CC_DELAYED_COMPLETION ), a completion function will do this.
 *
 * 	For a response:
 * 	1) check formatting and checksums.
 * 	2) call ipmi_process_response().
 * 	3) free the ws
 *
 * 	NOTE: ws->len_in integrity should be checked by the caller.
 * 
 */
void
ipmi_process_pkt( IPMI_WS * ws ) 
{
	IPMI_PKT	*pkt;
	uchar		cksum, completion_code = CC_NORMAL;
	uchar		responder_slave_addr, requester_slave_addr;
	
	IPMI_IPMB_HDR *ipmb_hdr = ( IPMI_IPMB_HDR * )&( ws->pkt_in );

	dputstr( DBG_IPMI | DBG_INOUT, "ipmi_process_pkt: ingress\n" );

	pkt = &ws->pkt;
	pkt->hdr.ws = (char *)ws;

	switch( ws->incoming_protocol ) {
		case IPMI_CH_PROTOCOL_IPMB:	/* used for IPMB, serial/modem Basic Mode, and LAN */
			dputstr( DBG_IPMI | DBG_LVL1, "ipmi_process_pkt: IPMB protocol\n" );

			if( ws->flags & WS_FL_GENERAL_CALL ) { 
				/* this is a broadcast request */

				/* Per IPMI spec:
				 * 1) The device that has the matching physical 
				 * slave address information shall respond with
				 * the same data it would return from a ‘regular’
				 * (non-broadcast) ‘Get Device ID’ command.
				 * 2) In order to speed the discovery process on
				 * the IPMB, a controller should drop off the bus
				 * as soon as it sees that the rsSA in the command
				 * doesn’t match its rsSA. */

				/* ignore if not our address */
				if( module_get_i2c_address( I2C_ADDRESS_LOCAL ) != ws->pkt_in[0] ) {
					dputstr( DBG_IPMI | DBG_LVL1, "ipmi_process_pkt: General Call: not our address\n" );
					ws_free( ws );
					return;
				}
				
			} 
			
			/* this is an interface that includes checksums -> verify payload integrity */
			/* need to check both header_checksum & data_checksum for requests. A response
			 * is directly bridged an we let the original requester worry about checksums */

			if( !( ipmb_hdr->netfn % 2 ) ) {
				/* an even netfn indicates a request */
				responder_slave_addr = module_get_i2c_address( I2C_ADDRESS_LOCAL );
				cksum = -( *( ws->pkt_in ) + responder_slave_addr );;
				if( ws->pkt_in[1] != cksum ) { /* header checksum is the second byte */
					dputstr( DBG_IPMI | DBG_ERR, "ipmi_process_pkt: Faulty header checksum\n" );
					completion_code = CC_INVALID_DATA_IN_REQ;
					break;
				}

				cksum = ipmi_calculate_checksum( &(((IPMI_IPMB_REQUEST *)(ws->pkt_in))->requester_slave_addr), 
						ws->len_in - 3 );
				if( ws->pkt_in[ws->len_in - 1] != cksum ) { /* data checksum is the last byte */
					dputstr( DBG_IPMI | DBG_ERR, "ipmi_process_pkt: Faulty data checksum\n" );
					completion_code = CC_INVALID_DATA_IN_REQ;
					break;
				}
			}
			
			/* TODO: check responder_slave_address to route this request if required */
			/* NOTE: BMC LUN 10b is used for delivering messages to 
			 * the System Interface. The BMC automatically routes any
			 * messages it receives via LUN 10b to the Receive Message Queue.
			 */

			/* fill in the IPMI_PKT structure */
			{
				IPMI_IPMB_REQUEST *ipmb_req;
				IPMI_IPMB_RESPONSE *ipmb_resp;

				if( ipmb_hdr->lun == 2 ) {
					/* route this to the system interface without processing ) */
				}
				
				if( ipmb_hdr->netfn % 2 ) {
					/* an odd netfn indicates a response */
					ipmb_req = NULL;
					ipmb_resp = ( IPMI_IPMB_RESPONSE * )&( ws->pkt_in );
					pkt->hdr.resp_data_len = ws->len_in - 8;
				} else {
					/* an even netfn is a request */
					ipmb_req = ( IPMI_IPMB_REQUEST * )&( ws->pkt_in );
					ipmb_resp = ( IPMI_IPMB_RESPONSE * )&( ws->pkt_out );
					pkt->hdr.responder_lun = ipmb_req->responder_lun;
					pkt->req = ( IPMI_CMD_REQ * )&( ipmb_req->command );
					pkt->hdr.req_data_len = ws->len_in - 7;
				}
				
				pkt->resp = ( IPMI_CMD_RESP * )&( ipmb_resp->completion_code );
				pkt->hdr.netfn = ipmb_hdr->netfn;
	
				/* check if responder_lun is valid */
				if( ipmb_req->responder_lun + 1 > NUM_LUN )
					completion_code = CC_INVALID_CMD;
			}
			
			break;
			
		case IPMI_CH_PROTOCOL_TMODE:		/* Terminal Mode */
			dputstr( DBG_IPMI | DBG_LVL1, "ipmi_process_pkt: Terminal Mode protocol\n" );
			pkt->req = ( IPMI_CMD_REQ * )&( ( ( IPMI_TERMINAL_MODE_REQUEST * )( ws->pkt_in ) )->command );
			pkt->resp = ( IPMI_CMD_RESP * )&( ( ( IPMI_TERMINAL_MODE_RESPONSE * )( ws->pkt_out ) )->completion_code );
			pkt->hdr.netfn = ( ( IPMI_TERMINAL_MODE_REQUEST * )( ws->pkt_in ) )->netfn;
			pkt->hdr.responder_lun = ( ( IPMI_TERMINAL_MODE_REQUEST * )( ws->pkt_in ) )->responder_lun;
			pkt->hdr.req_data_len = ws->len_in - 3;

			/* check if responder_lun is valid */
			if( pkt->hdr.responder_lun + 1 > NUM_LUN )
				completion_code = CC_INVALID_CMD;

			break;
			
		case IPMI_CH_PROTOCOL_ICMB:		/* ICMB v1.0 */
		case IPMI_CH_PROTOCOL_SMB:		/* IPMI on SMSBus */
		case IPMI_CH_PROTOCOL_KCS:		/* KCS System Interface Format */
		case IPMI_CH_PROTOCOL_SMIC:		/* SMIC System Interface Format */
		case IPMI_CH_PROTOCOL_BT10:		/* BT System Interface Format, IPMI v1.0 */
		case IPMI_CH_PROTOCOL_BT15:		/* BT System Interface Format, IPMI v1.5 */
		case IPMI_CH_PROTOCOL_NONE:
		default:
			dputstr( DBG_IPMI | DBG_ERR, "ipmi_process_pkt: unsupported protocol\n" );
			completion_code = CC_NOT_SUPPORTED;
			break;
	} /* end switch(incoming_protocol) */
	
	if( pkt->hdr.netfn % 2 ) {
		/* an odd netfn indicates a response */
		ipmi_process_response( pkt, completion_code );

		dputstr( DBG_IPMI | DBG_INOUT, "ipmi_process_pkt: egress\n" );

		return;
	}
	
	if( completion_code == CC_NORMAL )
		ipmi_process_request( pkt );

	/* ipmi_process_request fills in the |completion_code|data| portion 
	 * and also sets pkt->hdr.resp_data_len */
	
	/* send back response */
	if( completion_code != CC_DELAYED_COMPLETION ) {
		ws->outgoing_protocol = ws->incoming_protocol;
		ws->outgoing_medium = ws->incoming_medium;
		switch( ws->outgoing_protocol ) {
			case IPMI_CH_PROTOCOL_IPMB: {
				IPMI_IPMB_RESPONSE *ipmb_resp = ( IPMI_IPMB_RESPONSE * )&( ws->pkt_out );
				IPMI_IPMB_REQUEST *ipmb_req = ( IPMI_IPMB_REQUEST * )&( ws->pkt_in );
				
//				ipmb_resp->requester_slave_addr = ipmb_req->requester_slave_addr;
				requester_slave_addr = ipmb_req->requester_slave_addr;
				ipmb_resp->netfn = ipmb_req->netfn + 1;
				ipmb_resp->requester_lun = ipmb_req->requester_lun;
				ipmb_resp->header_checksum = -( *( char * )ipmb_resp + requester_slave_addr );
				ipmb_resp->responder_slave_addr = module_get_i2c_address( I2C_ADDRESS_LOCAL );
				ipmb_resp->req_seq = ipmb_req->req_seq;
				ipmb_resp->responder_lun = ipmb_req->responder_lun;
				ipmb_resp->command = ipmb_req->command;
				/* The location of data_checksum field is bogus.
				 * It's used as a placeholder to indicate that a checksum follows the data field.
				 * The location of the data_checksum depends on the size of the data preceeding it.*/
				ipmb_resp->data_checksum = 
					ipmi_calculate_checksum( &ipmb_resp->responder_slave_addr, 
						pkt->hdr.resp_data_len + 4 ); 
				ws->len_out = sizeof(IPMI_IPMB_RESPONSE) 
					- IPMB_RESP_MAX_DATA_LEN  +  pkt->hdr.resp_data_len;
				/* Assign the checksum to it's proper location */
				*( (uchar *)ipmb_resp + ws->len_out - 1 ) = ipmb_resp->data_checksum;
				}			
				break;
			
			case IPMI_CH_PROTOCOL_TMODE: {		/* Terminal Mode */
				IPMI_TERMINAL_MODE_RESPONSE *tm_resp = ( IPMI_TERMINAL_MODE_RESPONSE * )&( ws->pkt_out );
				IPMI_TERMINAL_MODE_REQUEST *tm_req = ( IPMI_TERMINAL_MODE_REQUEST * )&( ws->pkt_in );
				tm_resp->netfn = tm_req->netfn + 1;
				tm_resp->responder_lun = tm_req->responder_lun;
				tm_resp->req_seq = tm_req->req_seq;
				tm_resp->bridge = tm_req->bridge; /* TODO check */
				tm_resp->command = tm_req->command;
				ws->len_out = sizeof(IPMI_TERMINAL_MODE_RESPONSE)
					- TERM_MODE_RESP_MAX_DATA_LEN + pkt->hdr.resp_data_len;
				}
				break;
			
			case IPMI_CH_PROTOCOL_ICMB:		/* ICMB v1.0 */
			case IPMI_CH_PROTOCOL_SMB:		/* IPMI on SMSBus */
			case IPMI_CH_PROTOCOL_KCS:		/* KCS System Interface Format */
			case IPMI_CH_PROTOCOL_SMIC:		/* SMIC System Interface Format */
			case IPMI_CH_PROTOCOL_BT10:		/* BT System Interface Format, IPMI v1.0 */
			case IPMI_CH_PROTOCOL_BT15:		/* BT System Interface Format, IPMI v1.5 */
				/* we should not be here  */
				dputstr( DBG_IPMI | DBG_ERR, "ipmi_process_pkt: unsupported protocol\n" );
				break;
		}
		ws_set_state( ws, WS_ACTIVE_MASTER_WRITE );
	}

	/* If completion code is CC_DELAYED_COMPLETION do nothing */

	dputstr( DBG_IPMI | DBG_INOUT, "ipmi_process_pkt: egress\n" );
}

/*
 * ipmi_calculate_checksum()
 * 
 * 	8-bit checksum algorithm: Initialize checksum to 0. For each byte,
 * 	checksum = (checksum + byte) modulo 256. Then checksum = - checksum. 
 * 	When the checksum and the bytes are added together, modulo 256, 
 * 	the result should be 0.
 */
uchar
ipmi_calculate_checksum( unsigned char *ptr, int numchar )
{
	char checksum = 0;
	int i;
	
	for( i = 0; i < numchar; i++ ) {
		checksum += *ptr++;
	}
	
	return( -checksum );
}


void
ipmi_process_response( IPMI_PKT *pkt, unsigned char completion_code )
{
	IPMI_WS *req_ws = 0, *target_ws = 0, *resp_ws = 0;
	uchar seq = 0;
	uchar requester_slave_addr;
	int i;

	dputstr( DBG_IPMI | DBG_INOUT, "ipmi_process_response: ingress\n" );

	resp_ws = ( IPMI_WS * )pkt->hdr.ws;
	if( resp_ws->incoming_protocol == IPMI_CH_PROTOCOL_IPMB ) {
		seq = ((IPMI_IPMB_REQUEST *)(resp_ws->pkt_in))->req_seq;
	
		/* using the seq#, check to see if there is an outstanding target request ws
		 * corresponding to this response */
		target_ws = ws_get_elem_seq( seq, resp_ws ); 
	} else {
		/* currently unsupported */
		/* in instances where seq number is not used then the interface is waiting
		 * for the command to complete and there is a single outstanding ws */
		// target_ws = bridging_ws;
	}
	
	if( !target_ws ) {
		//call module response handler here
		module_process_response( req_ws, seq, completion_code );
#ifdef DUMP_RESPONSE
		putstr( "\n[" );
		for( i = 0; i < resp_ws->len_in; i++ ) {
			puthex( resp_ws->pkt_in[i] );
			putchar( ' ' );
		}
		putstr( "]\n" );
#endif
		ws_free( resp_ws );
		return;
	} else {
		req_ws = target_ws->bridged_ws;
	}

	if( !req_ws ) {
		ws_free( resp_ws );
		ws_free( target_ws );
		return;
	}
	
	memcpy( req_ws->pkt.resp, target_ws->pkt.resp, WS_BUF_LEN ); 
							   // TODO: make sure pkt-> pointers are set properly
	req_ws->len_out = target_ws->len_in;
	
	ws_free( resp_ws );
	ws_free( target_ws );
	
	/* send back response */
	req_ws->outgoing_protocol = req_ws->incoming_protocol;
	req_ws->outgoing_medium = req_ws->incoming_medium;
	
	switch( req_ws->outgoing_protocol ) {
		case IPMI_CH_PROTOCOL_IPMB: {
			IPMI_IPMB_RESPONSE *ipmb_resp = ( IPMI_IPMB_RESPONSE * )&( req_ws->pkt_out );
			IPMI_IPMB_REQUEST *ipmb_req = ( IPMI_IPMB_REQUEST * )&( req_ws->pkt_in );
			
//			ipmb_resp->requester_slave_addr = ipmb_req->requester_slave_addr;
			requester_slave_addr = ipmb_req->requester_slave_addr;			
			ipmb_resp->netfn = ipmb_req->netfn + 1;
			ipmb_resp->requester_lun = ipmb_req->requester_lun;
			ipmb_resp->header_checksum = -( *( char * )ipmb_resp + requester_slave_addr );
			ipmb_resp->responder_slave_addr = module_get_i2c_address( I2C_ADDRESS_LOCAL );
			ipmb_resp->req_seq = ipmb_req->req_seq;
			ipmb_resp->responder_lun = ipmb_req->responder_lun;
			ipmb_resp->command = ipmb_req->command;
			/* The location of data_checksum field is bogus.
			 * It's used as a placeholder to indicate that a checksum follows the data field.
			 * The location of the data_checksum depends on the size of the data preceeding it.*/
			ipmb_resp->data_checksum = 
				ipmi_calculate_checksum( &ipmb_resp->responder_slave_addr, 
					pkt->hdr.resp_data_len + 4 ); 
			req_ws->len_out = sizeof(IPMI_IPMB_RESPONSE) 
				- IPMB_RESP_MAX_DATA_LEN  +  pkt->hdr.resp_data_len;
			/* Assign the checksum to it's proper location */
			*( (uchar *)ipmb_resp + req_ws->len_out) = ipmb_resp->data_checksum; 
			}			
			break;
		
		case IPMI_CH_PROTOCOL_TMODE: {		/* Terminal Mode */
			IPMI_TERMINAL_MODE_RESPONSE *tm_resp = ( IPMI_TERMINAL_MODE_RESPONSE * )&( req_ws->pkt_out );
			IPMI_TERMINAL_MODE_REQUEST *tm_req = ( IPMI_TERMINAL_MODE_REQUEST * )&( req_ws->pkt_in );
			tm_resp->netfn = tm_req->netfn + 1;
			tm_resp->responder_lun = tm_req->responder_lun;
			tm_resp->req_seq = tm_req->req_seq;
			tm_resp->bridge = tm_req->bridge; /* TODO check */
			tm_resp->command = tm_req->command;
			req_ws->len_out = sizeof(IPMI_TERMINAL_MODE_RESPONSE)
				- TERM_MODE_RESP_MAX_DATA_LEN + pkt->hdr.resp_data_len;
			}
			break;
		
		case IPMI_CH_PROTOCOL_ICMB:		/* ICMB v1.0 */
		case IPMI_CH_PROTOCOL_SMB:		/* IPMI on SMSBus */
		case IPMI_CH_PROTOCOL_KCS:		/* KCS System Interface Format */
		case IPMI_CH_PROTOCOL_SMIC:		/* SMIC System Interface Format */
		case IPMI_CH_PROTOCOL_BT10:		/* BT System Interface Format, IPMI v1.0 */
		case IPMI_CH_PROTOCOL_BT15:		/* BT System Interface Format, IPMI v1.5 */
			/* Unsupported protocol */
			dputstr( DBG_IPMI | DBG_ERR, "ipmi_process_pkt: unsupported protocol\n" );
			break;
	}
	
	ws_set_state( req_ws, WS_ACTIVE_MASTER_WRITE );
}

void
ipmi_process_request( IPMI_PKT *pkt )
{
	dputstr( DBG_IPMI | DBG_INOUT, "ipmi_process_request: ingress\n" );

	switch( pkt->hdr.netfn ) {
		case NETFN_GROUP_EXTENSION_REQ:
			dputstr( DBG_IPMI | DBG_LVL1, "ipmi_process_request: NETFN_GROUP_EXTENSION_REQ\n" );
			picmg_process_command( pkt );
			break;
		case NETFN_APP_REQ:
			dputstr( DBG_IPMI | DBG_LVL1, "ipmi_process_request: NETFN_APP_REQ\n" );
			ipmi_process_app_req( pkt );
			break;
		case NETFN_EVENT_REQ:
			dputstr( DBG_IPMI | DBG_LVL1, "ipmi_process_request: NETFN_EVENT_REQ\n" );
			ipmi_process_event_req( pkt );
			break;
		case NETFN_FW_REQ:
			dputstr( DBG_IPMI | DBG_LVL1, "ipmi_process_request: NETFN_FW_REQ\n" );
			ipmi_process_fw_req( pkt );
			break;
		case NETFN_NVSTORE_REQ:
			dputstr( DBG_IPMI | DBG_LVL1, "ipmi_process_request: NETFN_NVSTORE_REQ\n" );
			ipmi_process_nvstore_req( pkt );
			break;
		default:
			dputstr( DBG_IPMI | DBG_LVL1, "ipmi_process_request: default\n" );
			ipmi_default_response( pkt );
			break;
	}
	
	dputstr( DBG_IPMI | DBG_INOUT, "ipmi_process_request: egress\n" );
}

/* firmware transfer request */
void
ipmi_process_fw_req( IPMI_PKT *pkt )
{
	dputstr( DBG_IPMI | DBG_INOUT, "ipmi_process_fw_req: ingress\n" );
	pkt->resp->completion_code = CC_INVALID_CMD;
	pkt->hdr.resp_data_len = 0;
}

void
ipmi_default_response( IPMI_PKT *pkt )
{
	dputstr( DBG_IPMI | DBG_INOUT, "ipmi_default_response: ingress\n" );
	pkt->resp->completion_code = CC_INVALID_CMD;
	pkt->hdr.resp_data_len = 0;
}

/*======================================================================*/
/*======================================================================*/
/*			NETFN_APP_REQ commands
 */
/*======================================================================*/
/*======================================================================*/
/*======================================================================*/
/*
 *   IPM Device “Global” Commands
 *
 *   Mandatory Commands
 *   	Get Device ID
 *   	Get Self Test Results
 *   	Broadcast ‘Get Device ID’ 
 */
/*======================================================================*/
/*======================================================================*/
/*
 *  BMC Watchdog Timer Mandatory Commands
 * 	Reset Watchdog Timer
 * 	Set Watchdog Timer
 * 	Get Watchdog Timer
 */
/*======================================================================*/

void
ipmi_process_app_req( IPMI_PKT *pkt )
{
	int err;

	dputstr( DBG_IPMI | DBG_INOUT, "ipmi_process_app_req: ingress\n" );

	switch( pkt->req->command )
	{
		case IPMI_CMD_GET_DEVICE_ID: 
		    {
			/* Broadcast Get Device ID is over IPMB channels only.
			 * Request is formatted as an entire IPMB application
			 * request message, from the RsSA field through the 
			 * second checksum, with the message prefixed with the
			 * broadcast slave address, 00h. Response format is 
			 * same as the regular ‘Get Device ID’ response. */
			    
			GET_DEVICE_ID_CMD_RESP *gdi_resp = 
				(GET_DEVICE_ID_CMD_RESP *)(pkt->resp);
			
			dputstr( DBG_IPMI | DBG_LVL1, "ipmi_process_app_req: IPMI_CMD_GET_DEVICE_ID\n" );
			
			gdi_resp->completion_code = CC_NORMAL;
			gdi_resp->device_id = 0x0;
			gdi_resp->device_sdr_provided = 1; 	/* 1 = device provides Device SDRs */
			gdi_resp->device_revision = 0;		/* 4 bit field, binary encoded */
			gdi_resp->device_available = 0;
			gdi_resp->major_fw_rev = 0x01;
			gdi_resp->minor_fw_rev = 0x00;
			gdi_resp->ipmi_version = 0x02;
			gdi_resp->add_dev_support = 
				DEV_SUP_IPMB_EVENT_GEN |
				DEV_SUP_FRU_INVENTORY |
				DEV_SUP_SDR_REPOSITORY |
				DEV_SUP_SENSOR;
			gdi_resp->manuf_id[0] = 0xbe;
			gdi_resp->manuf_id[1] = 0x12;
			gdi_resp->manuf_id[2] = 0x0;
			gdi_resp->product_id[0] = 0x00;
			gdi_resp->product_id[1] = 0x80;
			gdi_resp->aux_fw_rev[0] = 0x0;
			gdi_resp->aux_fw_rev[1] = 0x0;
			gdi_resp->aux_fw_rev[2] = 0x0;
			gdi_resp->aux_fw_rev[3] = 0x0;
		    }
		    pkt->hdr.resp_data_len = 15;
		    break;
		    
		case IPMI_CMD_COLD_RESET:
    		    {
			IPMI_CMD_RESP *generic_resp = ( IPMI_CMD_RESP *)(pkt->resp);

			dputstr( DBG_IPMI | DBG_LVL1, "ipmi_process_app_req: IPMI_CMD_COLD_RESET\n" );

			generic_resp->completion_code = CC_NORMAL;
			pkt->hdr.resp_data_len = 0;
		    }

		    break;

		case IPMI_CMD_WARM_RESET:
		    {
			IPMI_CMD_RESP *generic_resp = ( IPMI_CMD_RESP *)(pkt->resp);

			dputstr( DBG_IPMI | DBG_LVL1, "ipmi_process_app_req: IPMI_CMD_WARM_RESET\n" );

			generic_resp->completion_code = CC_NORMAL;
			pkt->hdr.resp_data_len = 0;
		    }
		    break;

		case IPMI_CMD_GET_SELF_TEST_RESULTS: 
		    {
			GET_SELF_TEST_RESULTS_CMD_RESP *gstr_resp =
				(GET_SELF_TEST_RESULTS_CMD_RESP *)(pkt->resp);				

			dputstr( DBG_IPMI | DBG_LVL1, "ipmi_process_app_req: IPMI_CMD_GET_SELF_TEST_RESULTS\n" );

			gstr_resp->completion_code = CC_NORMAL;
			/* Self Test function not implemented in this controller. */
			gstr_resp->result1 = SELFTEST_RESULT_NOT_IMPLEMENTED;
			gstr_resp->result2 = 0;
			gstr_resp->result3 = 0;
		    }	
		    pkt->hdr.resp_data_len = 3;
		    break;
		    
		case IPMI_CMD_RESET_WATCHDOG_TIMER: 
		    {
		 	/* The Reset Watchdog Timer command is used for starting
			 * and restarting the Watchdog Timer from the initial
			 * countdown value that was specified in the Set Watchdog
			 * Timer command. If a pre-timeout interrupt has been 
			 * configured, the Reset Watchdog Timer command will not
			 * restart the timer once the pre-timeout interrupt 
			 * interval has been reached. The only way to stop the
			 * timer once it has reached this point is via the Set
			 * Watchdog Timer command. 
			 * If the counter is loaded with zero and the Reset
			 * Watchdog command is issued to start the timer, the
			 * associated timer events occur immediately. */

			RESET_WATCHDOG_TIMER_CMD_RESP *resp = 
				(RESET_WATCHDOG_TIMER_CMD_RESP *)(pkt->resp);

			dputstr( DBG_IPMI | DBG_LVL1, "ipmi_process_app_req: IPMI_CMD_RESET_WATCHDOG_TIMER\n" );

			switch ( wd_timer.state ) {
				case WD_STATE_TIMER_RUNNING_POST_PRE_TIMEOUT_INTERRUPT:
					/* can't reset if we're post pre-timeout interrupt */
					resp->completion_code = CC_NORMAL;
					break;
				case WD_STATE_UNINITIALIZED:
					resp->completion_code = 0x80;
					break;
				case WD_STATE_INITIALIZED_TIMER_STOPPED:
				case WD_STATE_TIMER_RUNNING:
				default:
					if( wd_timer.pre_timeout_intr != WD_PRE_TIMEOUT_INTR_NONE ) {
						timer_reset_callout_queue( (void *)&wd_timer,
							wd_timer.init_countdown_ticks - wd_timer.pre_timeout_interval_ticks );
						wd_timer.pre_timeout_enabled = 1;
					} else {
						timer_reset_callout_queue( (void *)&wd_timer,
							wd_timer.init_countdown_ticks );
						wd_timer.pre_timeout_enabled = 0;
					}
					resp->completion_code = CC_NORMAL;
					break;
			}
			pkt->hdr.resp_data_len = 0;
		    }			
		    break;

		case IPMI_CMD_SET_WATCHDOG_TIMER: 
		    {
			/* The Set Watchdog Timer command is used for initializing
			 * and configuring the watchdog timer. The command is
			 * also used for stopping the timer.
			 * If the timer is already running, the Set Watchdog Timer
			 * command stops the timer (unless the “don’t stop” bit is
			 * set) and clears the Watchdog pre-timeout interrupt flag
			 * (see Get Message Flags command). BMC hard resets,
			 * system hard resets, and the Cold Reset command also
			 * stop the timer and clear the flag. */ 
			SET_WATCHDOG_TIMER_CMD_REQ *swd_req = 
				(SET_WATCHDOG_TIMER_CMD_REQ *)(pkt->req);
			SET_WATCHDOG_TIMER_CMD_RESP *resp = 
				(SET_WATCHDOG_TIMER_CMD_RESP *)(pkt->resp);

			unsigned long init_countdown_ticks = ( swd_req->init_countdown_msb << 8 ) |
				swd_req->init_countdown_lsb;
			unsigned long pre_timeout_interval_ticks = swd_req->pre_timeout_interval * 10;

			dputstr( DBG_IPMI | DBG_LVL1, "ipmi_process_app_req: IPMI_CMD_SET_WATCHDOG_TIMER\n" );

			/* parameter checking */
			if( swd_req->timeout_action > WD_TIMEOUT_ACTION_POWER_CYCLE )
				err++;

			if( swd_req->pre_timeout_intr &&
				( pre_timeout_interval_ticks > init_countdown_ticks ) )
				err++;

			/* stop wd timer */
			timer_remove_callout_queue( (void *)&wd_timer);

			/* set new values  */
			wd_timer.dont_log = swd_req->dont_log;
			wd_timer.timeout_action = swd_req->timeout_action;

			wd_timer.pre_timeout_intr = swd_req->pre_timeout_intr;
			wd_timer.pre_timeout_interval = swd_req->pre_timeout_interval;
			wd_timer.pre_timeout_interval_ticks = pre_timeout_interval_ticks;

			wd_timer.timer_use_exp_fl_clr = swd_req->timer_use_exp_fl_clr;

			wd_timer.init_countdown_lsb = swd_req->init_countdown_lsb;	/* (100 ms/count) */
			wd_timer.init_countdown_msb = swd_req->init_countdown_msb;
			wd_timer.init_countdown_ticks = init_countdown_ticks;

			wd_timer.timer_use = swd_req->timer_use;
			wd_timer.wd_initialized = 1;

			if( ( swd_req->dont_stop_timer ) && ( ( wd_timer.state == WD_STATE_TIMER_RUNNING ) ||
			      ( wd_timer.state == WD_STATE_TIMER_RUNNING_POST_PRE_TIMEOUT_INTERRUPT ) ) ) {
				if( wd_timer.pre_timeout_intr != WD_PRE_TIMEOUT_INTR_NONE ) {
					/* pre-timeout interrupt required */
					timer_add_callout_queue( (void *)&wd_timer,
						init_countdown_ticks - pre_timeout_interval_ticks,
						ipmi_wd_expired, 0 );
					wd_timer.pre_timeout_enabled = 1;
				} else {
					timer_add_callout_queue( (void *)&wd_timer,
						init_countdown_ticks,
						ipmi_wd_expired, 0 );
					wd_timer.pre_timeout_enabled = 0;
				}
				wd_timer.state = WD_STATE_TIMER_RUNNING;
		  	} else {
				wd_timer.state = WD_STATE_INITIALIZED_TIMER_STOPPED;
		  	}
			pkt->hdr.resp_data_len = 0;
		    }
		    break;

		case IPMI_CMD_GET_WATCHDOG_TIMER: 
		    {
			GET_WATCHDOG_TIMER_CMD_RESP *gwd_resp =
				( GET_WATCHDOG_TIMER_CMD_RESP *)(pkt->resp);	
			unsigned long ticks;			

			dputstr( DBG_IPMI | DBG_LVL1, "ipmi_process_app_req: IPMI_CMD_GET_WATCHDOG_TIMER\n" );

			gwd_resp->completion_code = CC_NORMAL;	
			gwd_resp->dont_log = wd_timer.dont_log;
			gwd_resp->dont_stop_timer = wd_timer.timer_running;
			gwd_resp->timer_use = wd_timer.timer_use;			
			gwd_resp->pre_timeout_intr = wd_timer.pre_timeout_intr;	
			gwd_resp->timeout_action = wd_timer.timeout_action;	
			gwd_resp->pre_timeout_interval = wd_timer.pre_timeout_interval;	
			gwd_resp->timer_use_exp_fl = wd_timer.timer_use_exp_fl;
			gwd_resp->init_countdown_lsb = wd_timer.init_countdown_lsb;
			gwd_resp->init_countdown_msb = wd_timer.init_countdown_msb;
			ticks = timer_get_expiration_time( &wd_timer );
			gwd_resp->present_countdown_lsb = ticks & 0xff;
			gwd_resp->present_countdown_msb = ( ticks >> 8 ) & 0xff;

			/* The initial countdown value and present countdown values 
			 * should match immediately after the countdown is initialized
			 * via a Set Watchdog Timer command and after a Reset Watchdog
			 * Timer has been executed.
			 * Note that internal delays in the BMC may require software to 
			 * delay up to 100ms before seeing the countdown value change 
			 * and be reflected in the Get Watchdog Timer command. */
			pkt->hdr.resp_data_len = sizeof( GET_WATCHDOG_TIMER_CMD_RESP ) - 1;
		    }
		    break;

		case IPMI_CMD_SEND_MESSAGE:
		    ipmi_send_message_cmd( pkt );
		    break;

		default:
		    {
			IPMI_CMD_RESP *generic_resp = ( IPMI_CMD_RESP *)(pkt->resp);

			dputstr( DBG_IPMI | DBG_LVL1, "ipmi_process_app_req: unknown command\n" );

			generic_resp->completion_code = CC_INVALID_CMD;
			pkt->hdr.resp_data_len = 0;
		    }
		    break;
	} /* switch( pkt->req->command ) */
	
	dputstr( DBG_IPMI | DBG_INOUT, "ipmi_process_app_req: egress\n" );
}

void
ipmi_wd_expired(uchar * arg) 
{
	dputstr( DBG_IPMI | DBG_INOUT, "ipmi_wd_expired: ingress\n" );

	/* is this the pre-timeout ? */
	if( wd_timer.pre_timeout_enabled ) {

		dputstr( DBG_IPMI | DBG_LVL1, "ipmi_wd_expired: pre-timeout\n" );

		wd_timer.pre_timeout_enabled = 0;

		switch( wd_timer.pre_timeout_intr ) {
			/* TODO: fill in as required */
			case WD_PRE_TIMEOUT_INTR_SMI:
			case WD_PRE_TIMEOUT_INTR_NMI:
			case WD_PRE_TIMEOUT_INTR_MSG:
			default:
				break;
		}
		
		if( wd_timer.pre_timeout_interval_ticks ) {
			/* add to callout queue for the remaining time */
			timer_add_callout_queue( (void *)&wd_timer,
				wd_timer.pre_timeout_interval_ticks,
				ipmi_wd_expired, 0 );
			return;
		} /* otherwise run the timeout actions below */
	}

	switch( wd_timer.timeout_action ) { 
		/* TODO: fill in as required, log if required */
		case WD_TIMEOUT_ACTION_NONE:
			break;
		case WD_TIMEOUT_ACTION_HARD_RESET:
			break;
		case WD_TIMEOUT_ACTION_POWER_DOWN:
			break;
		case WD_TIMEOUT_ACTION_POWER_CYCLE:
			break;
		default:
			break;
	}
}




/*======================================================================*/
/*======================================================================*/
/*			NETFN_NVSTORE
 *
 * Functionality present on any node that provides nonvolatile storage
 * and retrieval services
 */
/*======================================================================*/
/*======================================================================*/
void
ipmi_process_nvstore_req( IPMI_PKT *pkt )
{
	IPMI_CMD_RESP *resp = ( IPMI_CMD_RESP *)(pkt->resp);

	dputstr( DBG_IPMI | DBG_INOUT, "ipmi_process_nvstore_req: ingress\n" );

	switch( pkt->req->command ) {
		case IPMI_STO_CMD_GET_FRU_INVENTORY_AREA_INFO:
			ipmi_get_fru_inventory_area_info( pkt );
			break;
		case IPMI_STO_CMD_READ_FRU_DATA:
			ipmi_read_fru_data( pkt );
			break;
		case IPMI_STO_CMD_WRITE_FRU_DATA:
			ipmi_write_fru_data( pkt );
			break;
		case IPMI_STO_CMD_GET_SDR_REPOSITORY_INFO:
		case IPMI_STO_CMD_GET_SDR_REPOSITORY_ALLOCATION_INFO:
		case IPMI_STO_CMD_RESERVE_SDR_REPOSITORY:
		case IPMI_STO_CMD_GET_SDR:
		case IPMI_STO_CMD_ADD_SDR:
		case IPMI_STO_CMD_PARTIAL_ADD_SDR:
		case IPMI_STO_CMD_DELETE_SDR:
		case IPMI_STO_CMD_CLEAR_SDR_REPOSITORY:
		case IPMI_STO_CMD_GET_SDR_REPOSITORY_TIME:
		case IPMI_STO_CMD_SET_SDR_REPOSITORY_TIME:
		case IPMI_STO_CMD_ENTER_SDR_REPOSITORY_UPDATE_MODE:
		case IPMI_STO_CMD_EXIT_SDR_REPOSITORY_UPDATE_MODE:
		case IPMI_STO_CMD_RUN_INITIALIZATION_AGENT:
		case IPMI_STO_CMD_GET_SEL_INFO:
		case IPMI_STO_CMD_GET_SEL_ALLOCATION_INFO:
		case IPMI_STO_CMD_RESERVE_SEL:
		case IPMI_STO_CMD_GET_SEL_ENTRY:
		case IPMI_STO_CMD_ADD_SEL_ENTRY:
		case IPMI_STO_CMD_PARTIAL_ADD_SEL_ENTRY:
		case IPMI_STO_CMD_DELETE_SEL_ENTRY:
		case IPMI_STO_CMD_CLEAR_SEL:
		case IPMI_STO_CMD_GET_SEL_TIME:
		case IPMI_STO_CMD_SET_SEL_TIME:
		case IPMI_STO_CMD_GET_AUX_LOG_STATUS:
		case IPMI_STO_CMD_SET_AUX_LOG_STATUS:
			resp->completion_code = CC_INVALID_CMD;
			pkt->hdr.resp_data_len = 0;
			break;
		default:
			break;
	}
}

/*======================================================================*/
/*
 *    FRU Inventory Device Commands
 *
 *    Mandatory Commands
 *    	Get FRU Inventory Area Info
 *    	Read FRU Data
 *    	Write FRU Data
 *
 *    The FRU Inventory data contains information such as the serial number,
 *    part number, asset tag, and short descriptive string for the FRU. The
 *    contents of a FRU Inventory Record are specified in the Platform 
 *    Management FRU Information Storage Definition.
 *    	
 */
/*======================================================================*/

/* ipmi_get_fru_inventory_area_info()
 *
 * 	Given req->fru_dev_id, returns overall the size of the FRU Inventory
 * 	Area in this device, in bytes.
 *
 * 	Note that Reserved FRU Device ID 254 does not represent any Managed
 * 	FRU and is used only in “Get FRU Inventory Area Info,” “Read FRU Data”
 * 	and “Write FRU Data” commands directed to the Shelf Manager for
 * 	accessing logical Shelf FRU Information. See Section 3.6.4.4,  	
 * 	“Accessing Shelf FRU Information” of the PICMG® 3.0 Revision 2.0
 * 	AdvancedTCA® Base Specificationfor more details.
 * 	
 * 	If the device is on the i2c bus we grab a ws and configure it for an
 * 	i2c master read and put it on the work queue and return with completion
 * 	code of CC_DELAYED_COMPLETION.
 * 	
 * 	When the master read completes the completion function 
 * 	fru_inventory_area_info_callback() gets called which processes the 
 * 	data read and completes this transaction.
 *
 */
void
ipmi_get_fru_inventory_area_info( IPMI_PKT *pkt )
{
	GET_FRU_INVENTORY_AREA_INFO_CMD_REQ *req = ( GET_FRU_INVENTORY_AREA_INFO_CMD_REQ * )(pkt->req);
	GET_FRU_INVENTORY_AREA_CMD_RESP *resp = ( GET_FRU_INVENTORY_AREA_CMD_RESP * )(pkt->resp);
	int	i, found = 0;

	/* if the fru information is cached we already have this info */
	for( i = 0; i < FRU_INVENTORY_CACHE_ARRAY_SIZE; i++ ) {
		if( fru_inventory_cache[i].fru_dev_id == req->fru_dev_id ) {
			found = 1;
			break;
		}
	}

	if( found ) {
		resp->fru_inventory_area_size_lsb = fru_inventory_cache[i].fru_inventory_area_size & 0xff;
		resp->fru_inventory_area_size_msb = fru_inventory_cache[i].fru_inventory_area_size >> 8;
		resp->access_method = 0;	/* Device is accessed by bytes */
		resp->completion_code = CC_NORMAL;
		pkt->hdr.resp_data_len = 3;
	} else {
		resp->completion_code = CC_REQ_DATA_NOT_AVAIL;
		pkt->hdr.resp_data_len = 0;
	}
	
	return;	
}

/* ipmi_read_fru_data()
 * 
 * The command returns the specified data from the FRU Inventory Info area. 
 * This is effectively a ‘low level’ direct interface to a non-volatile storage
 * area. This means that the interface does not interpret or check any 
 * semantics or formatting for the data being accessed. The offset used in 
 * this command is a ‘logical’ offset that may or may not correspond to the 
 * physical address used in device that provides the non-volatile storage. 
 * For example, FRU information could be kept in FLASH at physical address 
 * 1234h, however offset 0000h would still be used with this command to access
 * the start of the FRU information. IPMI FRU device data (devices that are 
 * formatted per [FRU]) as well as processor and DIMM FRU data always starts 
 * from offset 0000h unless otherwise noted.
 * Note that while the offsets are 16-bit values, allowing FRU devices of up
 * to 64K words, the count to read, count returned, and count written fields 
 * are only 8-bits. This is in recognition of the limitations on the sizes of
 * messages. For example,IPMB messages are limited to 32-bytes total.
 *
 * If the fru data is not cached we have to read it from the device.
 * If the device is on the i2c bus we grab a ws and configure it for an
 * i2c master read and put it on the work queue and return with completion
 * code of CC_DELAYED_COMPLETION.
 * 	
 * When the master read completes the completion function 
 * fru_inventory_area_info_callback() gets called which processes the 
 * data read and completes this transaction.
 *
 */


void
ipmi_read_fru_data( IPMI_PKT *pkt )
{
	READ_FRU_DATA_CMD_REQ *req = ( READ_FRU_DATA_CMD_REQ * )(pkt->req);
	READ_FRU_DATA_CMD_RESP *resp = ( READ_FRU_DATA_CMD_RESP * )(pkt->resp);
	int	i, fru_inventory_offset, found = 0;
	IPMI_WS *ws;

	fru_inventory_offset = ( req->fru_inventory_offset_msb << 8 ) | 
		req->fru_inventory_offset_lsb;
	
	/* if the fru information is cached we already have this info */
	for( i = 0; i < FRU_INVENTORY_CACHE_ARRAY_SIZE; i++ ) {
		if( fru_inventory_cache[i].fru_dev_id == req->fru_dev_id ) {
			found = 1;
			break;
		}
	}
	
	if( found ) {
		if( fru_inventory_offset > fru_inventory_cache[i].fru_inventory_area_size ) {
			resp->completion_code = CC_RQST_DATA_LEN_INVALID;
			resp->count_returned = 0;			
			pkt->hdr.resp_data_len = 1;
			return;
		}
		
		if( ( fru_inventory_offset + req->count_to_read ) > 
				fru_inventory_cache[i].fru_inventory_area_size ) {
			resp->count_returned = fru_inventory_cache[i].fru_inventory_area_size - 
				fru_inventory_offset;
		} else {
			resp->count_returned = req->count_to_read;
		}
		
		/* we have a payload limit for IPMB */
		if( resp->count_returned > 20 ) /* TODO check size */
			resp->count_returned = 20;
		
		memcpy( &( resp->data ), fru_inventory_cache[i].fru_data + fru_inventory_offset,
			resp->count_returned );

		pkt->hdr.resp_data_len = resp->count_returned + 1;
		resp->completion_code = CC_NORMAL;
		return;
	}

	/* not in cache, we will have to read it from the device */
	if( !( ws = ws_alloc() ) ) {
		resp->completion_code = CC_OUT_OF_SPACE;
		pkt->hdr.resp_data_len = 0;
		return;
	}
	
	for( i = 0, found = 0; i < FRU_LOCATOR_TABLE_SIZE; i++ ) {
		if( fru_locator_table[i].fru_dev_id == req->fru_dev_id ) {
			found = 1;
			break;
		}
	}

	if( !found ) {
		resp->completion_code = CC_INVALID_DATA_IN_REQ;
		pkt->hdr.resp_data_len = 0;
		return;
	}
	
	resp->completion_code = CC_DELAYED_COMPLETION;

	ws->addr_out = fru_locator_table[i].i2c_address; /* protocol dependent */
	ws->outgoing_protocol = IPMI_CH_PROTOCOL_NONE;
       	ws->outgoing_medium = IPMI_CH_MEDIUM_IPMB;
	ws->bridged_ws = pkt->hdr.ws;
	ws->ipmi_completion_function = fru_read_complete;
	ws_set_state( ws, WS_ACTIVE_MASTER_READ );				
}

void
fru_read_complete( void *ws, int status )
{
	IPMI_WS *req_ws = ( (IPMI_WS *)( ws ) )->bridged_ws;
	IPMI_WS *fru_ws = ( (IPMI_WS *)( ws ) );
	IPMI_PKT *pkt = &req_ws->pkt;
	uchar requester_slave_addr;

	//TODO copy data to cache

	/* send back response */
	req_ws->outgoing_protocol = req_ws->incoming_protocol;
	req_ws->outgoing_medium = req_ws->incoming_medium;
	pkt->hdr.resp_data_len = fru_ws->len_in; // TODO Check size
	
	switch( req_ws->outgoing_protocol ) {
		case IPMI_CH_PROTOCOL_IPMB: {
			IPMI_IPMB_RESPONSE *ipmb_resp = ( IPMI_IPMB_RESPONSE * )&( req_ws->pkt_out );
			IPMI_IPMB_REQUEST *ipmb_req = ( IPMI_IPMB_REQUEST * )&( req_ws->pkt_in );
			
			memcpy(ipmb_resp->data, fru_ws->pkt_in, IPMB_RESP_MAX_DATA_LEN ); //TODO check size
//			ipmb_resp->requester_slave_addr = ipmb_req->requester_slave_addr;
			requester_slave_addr = ipmb_req->requester_slave_addr;
			ipmb_resp->netfn = ipmb_req->netfn + 1;
			ipmb_resp->requester_lun = ipmb_req->requester_lun;
			ipmb_resp->header_checksum = -( *( char * )ipmb_resp + requester_slave_addr );
			ipmb_resp->responder_slave_addr = module_get_i2c_address( I2C_ADDRESS_LOCAL );
			ipmb_resp->req_seq = ipmb_req->req_seq;
			ipmb_resp->responder_lun = ipmb_req->responder_lun;
			ipmb_resp->command = ipmb_req->command;
			ipmb_resp->completion_code = CC_NORMAL;
			/* The location of data_checksum field is bogus.
			 * It's used as a placeholder to indicate that a checksum follows the data field.
			 * The location of the data_checksum depends on the size of the data preceeding it.*/
			ipmb_resp->data_checksum = 
				ipmi_calculate_checksum( &ipmb_resp->responder_slave_addr, 
					pkt->hdr.resp_data_len + 4 ); 
			req_ws->len_out = sizeof(IPMI_IPMB_RESPONSE) 
				- IPMB_RESP_MAX_DATA_LEN  +  pkt->hdr.resp_data_len;
			/* Assign the checksum to it's proper location */
			*( (uchar *)ipmb_resp + req_ws->len_out) = ipmb_resp->data_checksum; 
			}			
			break;
		
		case IPMI_CH_PROTOCOL_TMODE: {		/* Terminal Mode */
			IPMI_TERMINAL_MODE_RESPONSE *tm_resp = ( IPMI_TERMINAL_MODE_RESPONSE * )&( req_ws->pkt_out );
			IPMI_TERMINAL_MODE_REQUEST *tm_req = ( IPMI_TERMINAL_MODE_REQUEST * )&( req_ws->pkt_in );
			memcpy(tm_resp->data, fru_ws->pkt_in, IPMB_RESP_MAX_DATA_LEN ); //TODO check size

			tm_resp->netfn = tm_req->netfn + 1;
			tm_resp->responder_lun = tm_req->responder_lun;
			tm_resp->req_seq = tm_req->req_seq;
			tm_resp->bridge = tm_req->bridge; /* TODO check */
			tm_resp->command = tm_req->command;
			req_ws->len_out = sizeof(IPMI_TERMINAL_MODE_RESPONSE)
				- TERM_MODE_RESP_MAX_DATA_LEN + pkt->hdr.resp_data_len;
			}
			break;
		
		case IPMI_CH_PROTOCOL_ICMB:		/* ICMB v1.0 */
		case IPMI_CH_PROTOCOL_SMB:		/* IPMI on SMSBus */
		case IPMI_CH_PROTOCOL_KCS:		/* KCS System Interface Format */
		case IPMI_CH_PROTOCOL_SMIC:		/* SMIC System Interface Format */
		case IPMI_CH_PROTOCOL_BT10:		/* BT System Interface Format, IPMI v1.0 */
		case IPMI_CH_PROTOCOL_BT15:		/* BT System Interface Format, IPMI v1.5 */
			/* Unsupported protocol */
			dputstr( DBG_IPMI | DBG_ERR, "ipmi_process_pkt: unsupported protocol\n" );
			break;
	}
	
	ws_free( fru_ws );
	ws_set_state( req_ws, WS_ACTIVE_MASTER_WRITE );	
}


void
ipmi_write_fru_data( IPMI_PKT *pkt )
{
	WRITE_FRU_DATA_CMD_REQ *req = ( WRITE_FRU_DATA_CMD_REQ * )(pkt->req);
	WRITE_FRU_DATA_CMD_RESP *resp = ( WRITE_FRU_DATA_CMD_RESP * )(pkt->resp);
	/* TODO */
}

void
init_fru_cache( void )
{
	/* init translation table from fru_dev_id to i2c bus address */
	 fru_locator_table[0].fru_dev_id = 0;  // fill in appropriately
	 fru_locator_table[0].i2c_address = 0;
}



/*======================================================================*/
/*
 *    SEL Device Commands
 *	For System Event Log Devices
 *	
 *    Mandatory Commands
 *    	Get SEL Info
 *    	Get SEL Entry
 *    	Add SEL Entry
 *    	Partial Add SEL Entry
 *    	Clear SEl
 *    	Get SEL Time
 *    	Set SEL Time
 */
/*======================================================================*/


/*======================================================================*/
/* BMC MESSAGE BRIDGING
 
Message Bridging Mechanism by Source and Destination
								BMC tracks
						Delivery	pending
Message Type and direction			Mechanism	responses
--------------------------------------------------------------------------
Request or Response from System Interface 	Send Message 	no
to any other channel

Request or Response to System Interface  	BMC LUN 10b 	no
from any other channel

Request from any channel except System  	Send Message 	yes
Interface to IPMB

Response from IPMB to any channel except 	BMC LUN 00b 	yes
System Interface
 
Request from any channel (except System 	Send Message 	yes
Interface) to PCI Management Bus

Request from PCI Management bus to any 		BMC LUN 00b 	yes
channel except System Interface

Request from Serial to LAN 			Send Message 	yes

Response LAN to Serial 				BMC LUN 00b 	yes

Request from LAN to Serial 			Send Message 	yes

Response from Serial to LAN 			BMC LUN 00b 	yes
 */
/*======================================================================*/


/*======================================================================*/
/* 
 * BMC Device and Messaging Commands
 */
/*======================================================================*/
/*
BMC Message Bridging
BMC Message Bridging provides a mechanism for routing IPMI Messages between different media. Bridging is
only specified for delivering messages between different channels; i.e. it is not specified for delivering messages
between two sessions on the same channel.
In IPMI 1.0, bridging was primarily specified just for providing access between SMS (System Interface) and the
IPMB. With IPMI 1.5, these mechanisms have been extended to support delivering IPMI messages between
active connections / sessions on any IPMI Messaging media connected to the BMC.
There are three mechanisms for bridging messages between different media connected to the BMC, depending on
what the target of the message is:
• BMC LUN 10b is used for delivering messages to the System Interface. The BMC automatically routes any
messages it receives via LUN 10b to the Receive Message Queue.
• Send Message command from System Interface is used for delivering messages to other channels, such as
the IPMB. The messages appear on the channel as if they’ve come from BMC LUN 10b. Thus, if the
message is a request message, the response will go to BMC LUN 10b and the BMC will automatically place
the response into the Receive Message Queue for retrieval. System software is responsible for matching the
response up with the original request, thus the ‘No Tracking’ setting in the Send Message command is used.
• Send Message command with response tracking. This format of Send Message command is used with
response tracking for bridging request messages to all other channels except when the System Interface is the
source or destination of the message.
*/ 
void
ipmi_send_message_cmd( IPMI_PKT *pkt )
{
	SEND_MESSAGE_CMD_REQ *req = ( SEND_MESSAGE_CMD_REQ * )(pkt->req);
	SEND_MESSAGE_CMD_RESP *resp = ( SEND_MESSAGE_CMD_RESP * )(pkt->resp);
	unsigned short destination_protocol, destination_medium;
	IPMI_WS *target_ws;
	uchar seq;
	
	dputstr( DBG_IPMI | DBG_INOUT, "ipmi_send_message_cmd: ingress\n" );

	if( !( target_ws = ws_alloc() ) ) {
		resp->completion_code = CC_OUT_OF_SPACE;
		pkt->hdr.resp_data_len = 0;
		return;
	}

	/* Two things to check: 1) the target should be a valid 
	 * channel 2) Bridging is only specified for delivering 
	 * messages between different channels */	
	if( ( req->channel_number != IPMI_CH_NUM_PRIMARY_IPMB )
	 || ( req->channel_number != IPMI_CH_NUM_CONSOLE )
	 || ( req->channel_number != IPMI_CH_NUM_SYS_INTERFACE )
	 || ( req->channel_number == ( ( IPMI_WS * )( pkt->hdr.ws ) )->incoming_channel ) ) {
		resp->completion_code = CC_DEST_UNAVAILABLE;
		pkt->hdr.resp_data_len = 0;
		return;
	}

	switch( req->tracking ) { /* TODO */
		case BRIDGE_NO_TRACKING:
		case BRIDGE_TRACK_REQ:
		case BRIDGE_SEND_RAW:
		default:
			break;
	}
	
	/*
        Each interface has a channel number that is used when 
        configuring the channel and for routing messages between
        channels. Only the channel number assignments for the 
        primary IPMB and the System Interface are fixed, the 
        assignment of other channel numbers can vary on a per-platform
        basis. Software uses a Get Channel Info command to determine
        what types of channels are available and what channel number
        assignments are used on a given platform.
	 */

	destination_protocol = channel_table[req->channel_number].protocol;
	destination_medium = channel_table[req->channel_number].medium;

	/* the medium determines the xport layer we will call ie. i2c/serial/LAN
	 * the protocol determines how we will package the payload to be
	 * transmitted */

	/* copy the request data payload to the outgoing packet */
	memcpy( target_ws->pkt_out, &( req->message_data ), sizeof( SEND_MESSAGE_CMD_REQ) );

	/* indicate which ws we should use to send back the response to the 
	 * bridged command */
	target_ws->bridged_ws = pkt->hdr.ws; /* used to find the send message ws */
	
	/* completion function is called when target_ws is successfully sent */
	target_ws->ipmi_completion_function = ipmi_send_message_cmd_complete;
	target_ws->outgoing_medium = destination_medium;

	/* swap in our seq number in so we can keep track */
	switch( destination_protocol ) {
		case IPMI_CH_PROTOCOL_IPMB:
			ipmi_get_next_seq( &seq );
			( ( IPMI_IPMB_REQUEST * )( target_ws->pkt_out ) )->req_seq = seq;
			target_ws->seq_out = seq;
			/* set completion code so that we immediately send 
			 * a response to the Send Message command */
			resp->completion_code = CC_NORMAL;
			pkt->hdr.resp_data_len = 0;
			break;
		case IPMI_CH_PROTOCOL_TMODE:
			resp->completion_code = CC_DEST_UNAVAILABLE;
			pkt->hdr.resp_data_len = 0;
			/* TODO: not supported at this time - fix. We need this to bridge 
			 * console & sys i/f
			if( tmode_bridging_ws ) {
				// someone is using the channel, can't process right now 
				resp->completion_code = CC_RQST_DATA_LEN_INVALID;
				resp->count_returned = 0;			
				pkt->hdr.resp_data_len = 1;
				return;
			}
			tmode_bridging_ws = pkt->hdr.ws;
			target_ws->seq_out = seq;
			*/
			return;
		default:
			resp->completion_code = CC_DEST_UNAVAILABLE;
			pkt->hdr.resp_data_len = 0;
			break;
	}

	dputstr( DBG_IPMI | DBG_LVL1, "ipmi_send_message_cmd: sending message\n" );

	/* dispatch the target_ws: change ws state, work list processing will do the rest */
	ws_set_state( target_ws, WS_ACTIVE_MASTER_WRITE );				
}

/*  This para from "- IPMI - IPMI v1.5 Addenda, Errata, and Clarifications
Intelligent Platform Management Interface Specification v1.5, revision 1.1
Addendum Document Revision 5 1/29/04". 

 When forwarding a response from IPMB to a different channel, only one header should be present in the response.
When a request message is bridged to another channel by encapsulating it in a Send Message command (from a source
channel other than the system interface), the BMC immediately returns a response to the Send Message command itself.
Meanwhile, the request is extracted from the Send Message command and forwarded to the specified target channel.
The Send Message command must be configured to direct the BMC to keep track of data in the request so when the
response comes back from the target device it can be forwarded by the BMC back to the channel that delivered the
original Send Message command to the BMC. When the response comes back from the target, the BMC uses the
tracking information to format the response for the given channel. To the party that initiated the Send Message
command, the response will appear as if the encapsulated request was directly executed by the BMC. I.e. it will look
like an asynchronously generated response message.
For example, suppose a Get Device ID command has been encapsulated in a Send Message command directed to the
IPMB from a LAN channel. The BMC will immediately send a response to the Send Message command back on LAN.
The BMC will extract the encapsulated Get Device ID message content and format it as a Get Device ID request for
IPMB. The target device on IPMB responds with a Get Device ID response message in IPMB format. The BMC takes
the tracking information that was stored when the Send Message command was issued, and uses it to create a Get
Device ID response in LAN format. The Responder’s address information in that response will be that of the BMC, not
of the device on IPMB that the request was targeted to.
 */
 

/* completion function is called when target_ws is successfully sent */
void
ipmi_send_message_cmd_complete( void *target_ws, int status )
{
	IPMI_WS *req_ws;
	uchar requester_slave_addr;

	dputstr( DBG_IPMI | DBG_INOUT, "ipmi_send_message_cmd_complete: ingress\n" );

	 /* Status options are :  XPORT_REQ_ERR, XPORT_REQ_NOERR */
	switch( status ) {
		case XPORT_REQ_ERR:
		default:
			dputstr( DBG_IPMI | DBG_ERR, "ipmi_send_message_cmd_complete: error when sending message\n" );
			req_ws = (( IPMI_WS *)( target_ws ))->bridged_ws ;
			ws_free( target_ws );

			if( !req_ws ) 
				break;
			
			/* if this is bridge req there is a pending response */ 
			/* send back response */
			req_ws->outgoing_protocol = req_ws->incoming_protocol;
			req_ws->outgoing_medium = req_ws->incoming_medium;
			switch( req_ws->outgoing_protocol ) {
				case IPMI_CH_PROTOCOL_IPMB: {
					IPMI_IPMB_RESPONSE *ipmb_resp = ( IPMI_IPMB_RESPONSE * )&( req_ws->pkt_out );
					IPMI_IPMB_REQUEST *ipmb_req = ( IPMI_IPMB_REQUEST * )&( req_ws->pkt_in );
					
//					ipmb_resp->requester_slave_addr = ipmb_req->requester_slave_addr;
					requester_slave_addr = ipmb_req->requester_slave_addr;
					ipmb_resp->netfn = ipmb_req->netfn + 1;
					ipmb_resp->requester_lun = ipmb_req->requester_lun;
					ipmb_resp->header_checksum = -( *( char * )ipmb_resp + requester_slave_addr );
					ipmb_resp->responder_slave_addr = module_get_i2c_address( I2C_ADDRESS_LOCAL );
					ipmb_resp->req_seq = ipmb_req->req_seq;
					ipmb_resp->responder_lun = ipmb_req->responder_lun;
					ipmb_resp->command = ipmb_req->command;
					ipmb_resp->completion_code = CC_DEST_UNAVAILABLE;
					/* The location of data_checksum field is bogus.
					 * It's used as a placeholder to indicate that a checksum follows the data field.
					 * The location of the data_checksum depends on the size of the data preceeding it.*/
					ipmb_resp->data_checksum = 
						ipmi_calculate_checksum( &ipmb_resp->responder_slave_addr, 4 ); 
					req_ws->len_out = sizeof(IPMI_IPMB_RESPONSE) - IPMB_RESP_MAX_DATA_LEN;
					/* Assign the checksum to it's proper location */
					*( (uchar *)ipmb_resp + req_ws->len_out) = ipmb_resp->data_checksum;
					}			
					break;
				
				case IPMI_CH_PROTOCOL_TMODE: {		/* Terminal Mode */
					IPMI_TERMINAL_MODE_RESPONSE *tm_resp = ( IPMI_TERMINAL_MODE_RESPONSE * )&( req_ws->pkt_out );
					IPMI_TERMINAL_MODE_REQUEST *tm_req = ( IPMI_TERMINAL_MODE_REQUEST * )&( req_ws->pkt_in );
					tm_resp->netfn = tm_req->netfn + 1;
					tm_resp->responder_lun = tm_req->responder_lun;
					tm_resp->req_seq = tm_req->req_seq;
					tm_resp->bridge = tm_req->bridge; /* TODO check */
					tm_resp->command = tm_req->command;
					req_ws->len_out = sizeof(IPMI_TERMINAL_MODE_RESPONSE)
						- TERM_MODE_RESP_MAX_DATA_LEN;
					}
					break;
				
				case IPMI_CH_PROTOCOL_ICMB:		/* ICMB v1.0 */
				case IPMI_CH_PROTOCOL_SMB:		/* IPMI on SMSBus */
				case IPMI_CH_PROTOCOL_KCS:		/* KCS System Interface Format */
				case IPMI_CH_PROTOCOL_SMIC:		/* SMIC System Interface Format */
				case IPMI_CH_PROTOCOL_BT10:		/* BT System Interface Format, IPMI v1.0 */
				case IPMI_CH_PROTOCOL_BT15:		/* BT System Interface Format, IPMI v1.5 */
					/* Unsupported protocol */
					dputstr( DBG_IPMI | DBG_ERR, "ipmi_send_message_cmd_complete: unsupported protocol\n" );
					break;
			}
			
			ws_set_state( req_ws, WS_ACTIVE_MASTER_WRITE );

			break;
		case XPORT_REQ_NOERR:
			dputstr( DBG_IPMI | DBG_LVL1, "ipmi_send_message_cmd_complete: req sent successfully\n" );
			break;
	}

}
