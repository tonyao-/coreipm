/*
-------------------------------------------------------------------------------
coreIPM/ipmi_test.c

Author: Gokhan Sozmen
-------------------------------------------------------------------------------
Copyright (C) 2007 Gokhan Sozmen
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
#include "ipmi.h"
#include "ws.h"
#include "strings.h"


int g_outgoing_protocol = IPMI_CH_PROTOCOL_TMODE;
int g_channel_number = 0;
int g_bridging_enabled = 0;
int g_responder_i2c_address = 20;
int g_outgoing_medium = IPMI_CH_MEDIUM_SERIAL;

/* Main menu options */
enum {
	KBD_APP_CMDS = 0,
	KBD_CHASSIS_CMDS,
	KBD_EVENT_CMDS,
	KBD_FIRMWARE_COMMANDS,
	KBD_NVSTORE_CMDS,
	KBD_MEDIA_SPECIFIC_CMDS,
	KBD_PICMG_CMDS,
	KBD_RUN_UNIX
};

#define KBD_SETTINGS	98
#define KBD_QUIT	99

char * main_str[] = {
	"Application commands",
	"Chassis commands",
	"Event commands",
	"Firmware commands",
	"NVSTORE commands",
	"Media specific commands",
	"PICMG commands",
	"Run UNIX command",
	"Quit"
};

enum {
	KBD_CHANNEL_NUMBER = 0,
	KBD_BRIDGE_COMMAND,
	KBD_RESPONDER_I2C_ADDR,
	KBD_OUTGOING_PROTOCOL
};

char * settings_str[] = {
	"Channel number",
	"Bridge command",
	"Responder i2c address",
	"Outgoing protocol"
};

/*------------------------------------------------------------------------------
 *              L O C A L   F U N C T I O N   P R O T O T Y P E S
 *----------------------------------------------------------------------------*/
void process_command_line( int argc, char **argv );
void main_menu( void );
void atca_cmd_get_picmg_properties( void );
void cmd_menu( STR_LST *ptr );
void kbd_app_cmds( void );
void kbd_chassis_cmds( void );
void kbd_event_cmds( void );
void kbd_firmware_commands( void );
void kbd_nvstore_cmds( void );
void kbd_media_specific_cmds( void );
void kbd_atca_cmds( void );
void fill_pkt_out( IPMI_WS *ws, IPMI_CMD_REQ *cmd_req, int data_len, int netfn );
void app_cmd_get_device_id( void );
void app_cmd_get_self_test_results( void );
void app_cmd_reset_watchdog_timer( void );
void kbd_settings( void );
void settings_menu( void );
void current_settings( void );
unsigned char calculate_checksum( char *ptr, int numchar );

/*------------------------------------------------------------------------------
 *              F U N C T I O N S
 *----------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------
	main()
		Main
	Preconditions:
	Postconditions:
 *----------------------------------------------------------------------------*/
int  main(
	int argc,
	char **argv )
/*----------------------------------------------------------------------------*/
{
	int user_input;
	char user_str[128];

	ws_init();

	printf("\nIPMI Exerciser -- %s\n", __DATE__); // say Hi

	process_command_line( argc, argv ); // process command line arguments
	// Get user keyboard input
	while( 1 )
	{
		main_menu();
		scanf( "%d", &user_input );
		fflush( stdin );

		switch ( user_input )
		{
			case KBD_APP_CMDS:
				kbd_app_cmds();
				break;
			case KBD_CHASSIS_CMDS:
				break;
			case KBD_EVENT_CMDS:
				break;
			case KBD_FIRMWARE_COMMANDS:
				break;
			case KBD_NVSTORE_CMDS:
				break;
			case KBD_MEDIA_SPECIFIC_CMDS:
				break;
			case KBD_PICMG_CMDS:
				kbd_atca_cmds();
				break;
			case KBD_RUN_UNIX:
				printf( "Enter command : " );
				scanf( "%s", user_str );
				//gets( user_str );
				fflush( stdin );
				if( system( user_str ) == -1 )
					printf( "\nError in command\n" );
				printf( "\n Press any key to continue\n" );
				getchar();
				fflush( stdin );
				break;
			case KBD_SETTINGS:
				kbd_settings();
				break;
			case KBD_QUIT:
				exit( EXIT_SUCCESS );
				break;
			default:
				printf( "Unknown option %d ignored.\n", user_input );
				break;
		} // end switch
		printf( "Press any key to continue\n" );
		getchar();
		fflush( stdin );

	}
}

/*------------------------------------------------------------------------------
	process_command_line()
		Process the command line arguments.
	Preconditions:
	Postconditions:
 *----------------------------------------------------------------------------*/
void process_command_line(
	int argc,
	char **argv )
/*----------------------------------------------------------------------------*/
{
	int i;
	char *opt;

	// get command line arguments
	for (i=1; i<argc; i++)
	{
		if ((*argv[i] != '/') && (*argv[i] != '-'))
			continue;
		else
		{
			opt = argv[i];         // Option scanning...
			switch ( opt[1] )
			{
				case 'L':	// loop test
				case 'l':
					switch (opt[2])
					{
						case '0':
							printf( "Running loop test 1\n" );
							break;

						case '1':
							printf( "Running loop test 1\n" );
							break;

						case '2':
							printf( "Running loop test 2\n" );
							break;

						case '3':
							printf( "Running loop test 3\n" );
							break;

						case '4':
							printf( "Running loop test 4\n" );
							break;

						case '5':
							printf( "Running loop test 5\n" );
							break;

						case '6':
							printf( "Running loop test 5\n" );
							break;

						default:
							printf( "Unknown loop test number\n" );
							break;
					}
					break;

				case 'T':	// canned test
				case 't':
					switch (opt[2])
					{
						case '1':
							printf( "Running Test 1\n" );
							break;

						case '2':
							printf( "Running Test 2\n" );
							break;

						case '3':
							printf( "Running Test 3\n" );
							break;

						case '4':
							printf( "Running Test 4\n" );
							break;

						default:
							printf( "Unknown test number\n" );
							break;
					}
					break;

				default:
					printf("Unknown option %s ignored.\n",argv[i]);
					break;
			}
		}
	}
}


/*------------------------------------------------------------------------------
	main_menu()

	Preconditions:
	Postconditions:
 *----------------------------------------------------------------------------*/
void main_menu( void )
/*----------------------------------------------------------------------------*/
{
	printf(
		"\n"
		" MAIN MENU        \n"
		" =====================================\n" );
	current_settings();
	printf(
		"%2d - %-34s"
		"%2d - %-34s\n"
		"%2d - %-34s"
		"%2d - %-34s\n"
		"%2d - %-34s"
		"%2d - %-34s\n"
		"%2d - %-34s"
		"%2d - %-34s\n"
		"%2d - Change Settings\n"
		"%2d - Quit Program\n"
		" Enter choice:",
		KBD_APP_CMDS, main_str[KBD_APP_CMDS],
		KBD_CHASSIS_CMDS, main_str[KBD_CHASSIS_CMDS],
		KBD_EVENT_CMDS, main_str[KBD_EVENT_CMDS],
		KBD_FIRMWARE_COMMANDS, main_str[KBD_FIRMWARE_COMMANDS],
		KBD_NVSTORE_CMDS, main_str[KBD_NVSTORE_CMDS],
		KBD_MEDIA_SPECIFIC_CMDS, main_str[KBD_MEDIA_SPECIFIC_CMDS],
		KBD_PICMG_CMDS, main_str[KBD_PICMG_CMDS],
		KBD_RUN_UNIX, main_str[KBD_RUN_UNIX],
		KBD_SETTINGS,
		KBD_QUIT );
}

/*------------------------------------------------------------------------------
	main_menu()

	Preconditions:
	Postconditions:
 *----------------------------------------------------------------------------*/
void kbd_settings( void )
/*----------------------------------------------------------------------------*/
{
	int user_input;
	char user_str[128];

	// Get user keyboard input
	while( 1 )
	{
		settings_menu();
		scanf( "%d", &user_input );
		fflush( stdin );

		switch ( user_input )
		{
			case KBD_CHANNEL_NUMBER:
				printf( "Enter channel number: ");
				scanf( "%d", &user_input );
				fflush( stdin );
				if( user_input < 0 || user_input > 20 )
					printf( "Invalid channel number\n" );
				else
					g_channel_number = user_input;

				break;
			case KBD_BRIDGE_COMMAND:
				printf( "Enable bridging ? [y/n]: ");
				scanf( " %c", &user_input );
				fflush( stdin );
				if( user_input == 'Y' || user_input == 'y' )
					g_bridging_enabled = 1;
				else if( user_input == 'N' || user_input == 'n' )
					g_bridging_enabled = 0;
				else					
					printf( "Invalid selection %c\n", user_input );
				break;
			case KBD_RESPONDER_I2C_ADDR:
				printf( "Enter responder i2c address: ");
				scanf( "%d", &user_input );
				fflush( stdin );
				if( user_input < 0 || user_input > 20 )
					printf( "Invalid address\n" );
				else
					g_responder_i2c_address = user_input;

				break;
			case KBD_OUTGOING_PROTOCOL:
				break;
			case KBD_QUIT:
				return;
			default:
				printf( "Unknown option %d ignored.\n", user_input );
				break;
		} // end switch
		printf( "Press any key to continue\n" );
		getchar();
		fflush( stdin );

	}
}

void current_settings( void )
/*----------------------------------------------------------------------------*/
{
	printf(
		" +----------------SETTINGS------------------------------------------------------------+\n"
		" |  Bridging enabled      : %c              |  Channel number        : %1d               |\n"
		" |  Responder i2c address : %2d             |  Outgoing protocol     : %1d               |\n"		
		" +------------------------------------------------------------------------------------+\n", 
		g_bridging_enabled ? 'Y':'N', g_channel_number, 
		g_responder_i2c_address, g_outgoing_protocol ); 

}

/*------------------------------------------------------------------------------
	Preconditions:
	Postconditions:
 *----------------------------------------------------------------------------*/
void settings_menu( void )
/*----------------------------------------------------------------------------*/
{
	printf(
		"\n"
		" SETTINGS MENU        \n"
		" =====================================\n");
	current_settings();
	printf(	
		"%2d - %-34s"
		"%2d - %-34s\n"
		"%2d - %-34s"
		"%2d - %-34s\n"
		"%2d - Parent menu\n"
		" Enter choice:",
		KBD_CHANNEL_NUMBER, settings_str[KBD_CHANNEL_NUMBER],
		KBD_BRIDGE_COMMAND, settings_str[KBD_BRIDGE_COMMAND],
		KBD_RESPONDER_I2C_ADDR, settings_str[KBD_RESPONDER_I2C_ADDR],
		KBD_OUTGOING_PROTOCOL, settings_str[KBD_OUTGOING_PROTOCOL],
		KBD_QUIT );
}

/*------------------------------------------------------------------------------
	Preconditions:
	Postconditions:
 *----------------------------------------------------------------------------*/
void cmd_menu( STR_LST *ptr )
/*----------------------------------------------------------------------------*/
{
	int i = 0;

	while( 1 ) {
		if( ptr[i].id == -1 )
			break;

		printf( "%2d - %-34s", ptr[i].id, ptr[i].str );

		if( !( i%2 ) )
			printf( "\n" );

		i++;
	}
	printf( "%2d - Change settings\n", KBD_SETTINGS );
	printf(	"%2d - Main menu\n"
		" Enter choice:", KBD_QUIT );
}
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

		size = fill_pkt( cmd_req, data_len + 1, &send_message_req.data, netfn, IPMI_CH_PROTOCOL_IPMB );
		size = fill_pkt( &send_message_req, size, ws->pkt_out, NETFN_APP_REQ, g_outgoing_protocol );

		ws->pkt.hdr.netfn = netfn;
		ws->outgoing_protocol = g_outgoing_protocol;
		ws->outgoing_medium = g_outgoing_medium;
		ws->len_out = size;

	} else {
		size = fill_pkt( cmd_req, data_len + 1, ws->pkt_out, netfn, g_outgoing_protocol );
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
	
			req->responder_slave_addr = g_responder_i2c_address;
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
	
			req->responder_slave_addr = g_responder_i2c_address;
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

			req->header_checksum = calculate_checksum( req, 2 );
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
/*==============================================================================
 * 			A P P   C O M M A N D S
 *============================================================================*/

/*------------------------------------------------------------------------------
	Preconditions:
	Postconditions:
 *----------------------------------------------------------------------------*/
void kbd_app_cmds( void )
/*----------------------------------------------------------------------------*/
{
	int user_input;

	while( 1 )
	{
		printf(
			"\n"
			" APP COMMANDS MENU        \n"
			" =====================================\n" );
		current_settings();
		cmd_menu( app_str );
		scanf( "%d", &user_input );
		fflush( stdin );
		switch( user_input )
		{
			case IPMI_CMD_GET_DEVICE_ID:
				app_cmd_get_device_id();
				break;
			case IPMI_CMD_GET_SELF_TEST_RESULTS:
				app_cmd_get_self_test_results();
				break;
			case IPMI_CMD_RESET_WATCHDOG_TIMER:
				app_cmd_reset_watchdog_timer();
				break;
			case IPMI_CMD_SET_WATCHDOG_TIMER:
			case IPMI_CMD_GET_WATCHDOG_TIMER:
			case IPMI_CMD_SEND_MESSAGE:
				break;
			case KBD_SETTINGS:
				kbd_settings();
				break;
			case KBD_QUIT:
				return;
			default:
				printf( "Invalid Entry\n" );
		}
		ws_process_work_list();
	}
}

/*------------------------------------------------------------------------------

	Fill in the ws structure and set state.
	It will get picked up and forwarded to the 
	proper interface
	
	Preconditions:
	Postconditions:
 *----------------------------------------------------------------------------*/
void app_cmd_get_device_id( void )
/*----------------------------------------------------------------------------*/
{
	IPMI_WS		*ws;
	IPMI_CMD_REQ	cmd_req;
	
	ws = ws_alloc();
	
	if( !ws ) {
		printf( "app_cmd_get_device_id: ws allocation failed\n" );
		return;
	}

	cmd_req.command = IPMI_CMD_GET_DEVICE_ID;

	fill_pkt_out( ws, &cmd_req, 0, NETFN_APP_REQ );
	/* the completion function will be called by the transport layer
	 * completion routine after the xfer has completed. It is up to the 
	 * test framework to keep track of request/response pairs using the 
	 * sequence numbers */
	ws->ipmi_completion_function = 0; // none at this moment 

	/* change ws state, work list processing will do the rest */
	ws_set_state( ws, WS_ACTIVE_MASTER_WRITE );

}

void app_cmd_get_self_test_results( void )
/*----------------------------------------------------------------------------*/
{
	IPMI_WS		*ws;
	IPMI_CMD_REQ	cmd_req;
	
	ws = ws_alloc();
	
	if( !ws ) {
		printf( "app_cmd_get_device_id: ws allocation failed\n" );
		return;
	}

	cmd_req.command = IPMI_CMD_GET_SELF_TEST_RESULTS;

	fill_pkt_out( ws, &cmd_req, 0, NETFN_APP_REQ );
	ws->ipmi_completion_function = 0; // none at this moment 

	/* change ws state, work list processing will do the rest */
	ws_set_state( ws, WS_ACTIVE_MASTER_WRITE );

}

void app_cmd_reset_watchdog_timer( void )
/*----------------------------------------------------------------------------*/
{
	IPMI_WS		*ws;
	IPMI_CMD_REQ	cmd_req;
	
	ws = ws_alloc();
	
	if( !ws ) {
		printf( "app_cmd_get_device_id: ws allocation failed\n" );
		return;
	}

	cmd_req.command = IPMI_CMD_RESET_WATCHDOG_TIMER;

	fill_pkt_out( ws, &cmd_req, 0, NETFN_APP_REQ );
	ws->ipmi_completion_function = 0; // none at this moment 

	/* change ws state, work list processing will do the rest */
	ws_set_state( ws, WS_ACTIVE_MASTER_WRITE );
}


/*==============================================================================
 * 			C H A S S I S   C O M M A N D S
 *============================================================================*/

/*------------------------------------------------------------------------------
	Preconditions:
	Postconditions:
 *----------------------------------------------------------------------------*/
void kbd_chassis_cmds( void )
/*----------------------------------------------------------------------------*/
{
	int user_input;

	printf(
		"\n"
		" CHASSIS COMMANDS MENU        \n"
		" =====================================\n" );
	while( 1 )
	{
		current_settings();
		cmd_menu( chassis_str );
		scanf( "%d", &user_input );
		fflush( stdin );
		switch( user_input )
		{
			case KBD_QUIT:
				return;
			default:
				printf( "Invalid Entry\n" );
		}
	}
}

/*==============================================================================
 * 			E V E N T   C O M M A N D S
 *============================================================================*/

/*------------------------------------------------------------------------------
	Preconditions:
	Postconditions:
 *----------------------------------------------------------------------------*/
void kbd_event_cmds( void )
/*----------------------------------------------------------------------------*/
{
	int user_input;

	printf(
		"\n"
		" EVENT COMMANDS MENU        \n"
		" =====================================\n" );
	while( 1 )
	{
		current_settings();
		cmd_menu( event_str );
		scanf( "%d", &user_input );
		fflush( stdin );
		switch( user_input )
		{
			case KBD_QUIT:
				return;
			default:
				printf( "Invalid Entry\n" );
		}
	}
}

/*==============================================================================
 * 			F I R M W A R E   C O M M A N D S
 *============================================================================*/

/*------------------------------------------------------------------------------
	Preconditions:
	Postconditions:
 *----------------------------------------------------------------------------*/
void kbd_firmware_commands( void )
/*----------------------------------------------------------------------------*/
{
	int user_input;

	printf(
		"\n"
		" FIRMWARE COMMANDS MENU        \n"
		" =====================================\n" );
	while( 1 )
	{
		current_settings();
		cmd_menu( firmware_str );
		scanf( "%d", &user_input );
		fflush( stdin );
		switch( user_input )
		{
			case KBD_QUIT:
				return;
			default:
				printf( "Invalid Entry\n" );
		}
	}
}

/*==============================================================================
 * 			N V S T O R E   C O M M A N D S
 *============================================================================*/

/*------------------------------------------------------------------------------
	Preconditions:
	Postconditions:
 *----------------------------------------------------------------------------*/
void kbd_nvstore_cmds( void )
/*----------------------------------------------------------------------------*/
{
	int user_input;

	printf(
		"\n"
		" NVSTORE COMMANDS MENU        \n"
		" =====================================\n" );
	while( 1 )
	{
		current_settings();
		cmd_menu( nvstore_str );
		scanf( "%d", &user_input );
		fflush( stdin );
		switch( user_input )
		{
			case KBD_QUIT:
				return;
			default:
				printf( "Invalid Entry\n" );
		}
	}
}

/*==============================================================================
 * 			M E D I A   S P E C I F I C   C O M M A N D S
 *============================================================================*/

/*------------------------------------------------------------------------------
	Preconditions:
	Postconditions:
 *----------------------------------------------------------------------------*/
void kbd_media_specific_cmds( void )
/*----------------------------------------------------------------------------*/
{
	int user_input;

	printf(
		"\n"
		" MEDIA SPECIFIC COMMANDS MENU        \n"
		" =====================================\n" );
	while( 1 )
	{
		current_settings();
		cmd_menu( media_specific_str );
		scanf( "%d", &user_input );
		fflush( stdin );
		switch( user_input )
		{
			case KBD_QUIT:
				return;
			default:
				printf( "Invalid Entry\n" );
		}
	}
}

/*==============================================================================
 * 			A T C A   C O M M A N D S
 *============================================================================*/

/*------------------------------------------------------------------------------
	Preconditions:
	Postconditions:
 *----------------------------------------------------------------------------*/
void kbd_atca_cmds( void )
/*----------------------------------------------------------------------------*/
{
	int user_input;

	while( 1 )
	{
	printf(
		"\n"
		" ATCA COMMANDS MENU        \n"
		" =====================================\n" );
		current_settings();
		cmd_menu( atca_str );
		scanf( "%d", &user_input );
		fflush( stdin );
		switch( user_input )
		{
			case ATCA_CMD_GET_PICMG_PROPERTIES:
				atca_cmd_get_picmg_properties();
				break;
			case ATCA_CMD_GET_ADDRESS_INFO:
			case ATCA_CMD_GET_SHELF_ADDRESS_INFO:
			case ATCA_CMD_SET_SHELF_ADDRESS_INFO:
			case ATCA_CMD_FRU_CONTROL:
			case ATCA_CMD_GET_FRU_LED_PROPERTIES:
			case ATCA_CMD_GET_LED_COLOR:
			case ATCA_CMD_SET_FRU_LED_STATE:
			case ATCA_CMD_GET_FRU_LED_STATE:
			case ATCA_CMD_SET_IPMB_STATE:
			case ATCA_CMD_SET_FRU_ACTIVATION_POLICY:
			case ATCA_CMD_GET_FRU_ACTIVATION_POLICY:
			case ATCA_CMD_SET_FRU_ACTIVATION:
			case ATCA_CMD_GET_DEVICE_LOCATOR_REC_ID:
			case ATCA_CMD_SET_PORT_STATE:
			case ATCA_CMD_GET_PORT_STATE:
			case ATCA_CMD_COMPUTE_POWER_PROPERTIES:
			case ATCA_CMD_SET_POWER_LEVEL:
			case ATCA_CMD_GET_POWER_LEVEL:
			case ATCA_CMD_RENEGOTIATE_POWER:
			case ATCA_CMD_GET_FAN_SPEED_PROPERTIES:
			case ATCA_CMD_SET_FAN_LEVEL:
			case ATCA_CMD_GET_FAN_LEVEL:
			case ATCA_CMD_BUSED_RESOURCE:
			case ATCA_CMD_GET_IPMB_LINK_INFO:
				break;
			case KBD_QUIT:
				return;
			default:
				printf( "Invalid Entry\n" );
		}
		ws_process_work_list();
	}
}


/*------------------------------------------------------------------------------
	Preconditions:
	Postconditions:
 *----------------------------------------------------------------------------*/
void atca_cmd_get_picmg_properties( void )
/*----------------------------------------------------------------------------*/
{
	IPMI_WS		*ws;
	GET_PICMG_PROPERTIES_CMD_REQ	cmd_req;
	
	printf( "app_cmd_get_device_id:\n" );

	ws = ws_alloc();
	
	if( !ws ) {
		printf( "app_cmd_get_device_id: ws allocation failed\n" );
		return;
	}

	cmd_req.command = ATCA_CMD_GET_PICMG_PROPERTIES;
	cmd_req.picmg_id = 0;
	
	fill_pkt_out( ws, &cmd_req, 1, NETFN_PICMG_REQ );
	/* the completion function will be called by the transport layer
	 * completion routine after the xfer has completed. It is up to the 
	 * test framework to keep track of request/response pairs using the 
	 * sequence numbers */
	ws->ipmi_completion_function = 0; // none at this moment 

	/* change ws state, work list processing will do the rest */
	ws_set_state( ws, WS_ACTIVE_MASTER_WRITE );
}

/*==============================================================================
 * 			P R O T O C O L   H A N D L E R S
 *============================================================================*/

int ipmi_process_pkt( IPMI_WS *ws )
/*----------------------------------------------------------------------------*/
{
	printf( "i2c_process_pkt\n" );
}

void i2c_master_read( IPMI_WS *ws )
/*----------------------------------------------------------------------------*/
{
	printf( "i2c_master_read\n" );
	
}

void i2c_master_write( IPMI_WS *ws )
/*----------------------------------------------------------------------------*/
{
	printf( "i2c_master_write\n" );
	
}

void serial_tm_send( IPMI_WS *ws )
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

	
