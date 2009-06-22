/*
-------------------------------------------------------------------------------
coreIPM/ipmi_test.c

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
#include "ipmi.h"
#include "ws.h"
#include "strings.h"
#include "ipmi_pkt.h"
#include "rmcpd.h"

// AMC_INFO amc[NUM_AMC_SLOTS];


int g_outgoing_protocol = IPMI_CH_PROTOCOL_TMODE;
int g_channel_number = 0;
int g_bridging_enabled = 0;
int g_responder_i2c_address = 20;
int g_outgoing_medium = IPMI_CH_MEDIUM_SERIAL;

unsigned long lbolt = 0;

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
void app_cmd_get_device_id( void );
void app_cmd_get_self_test_results( void );
void app_cmd_reset_watchdog_timer( void );
void kbd_settings( void );
void settings_menu( void );
void current_settings( void );

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
	tty_init();
	// rmcpd_init_listener();

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
				tty_restore();
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
							printf( "Running loop test 6\n" );
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
			case ATCA_CMD_BUSED_RESOURCE_CONTROL:
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
	
	fill_pkt_out( ws, ( IPMI_CMD_REQ * )&cmd_req, 1, NETFN_PICMG_REQ );
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

void ipmi_process_pkt( IPMI_WS *ws )
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

// retrieve data from all AMC cards in the system and fill the AMC_INFO struct
void
get_amc_data( void )
{
	
}

