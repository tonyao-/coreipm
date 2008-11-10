/*
-------------------------------------------------------------------------------
coreIPM/mcmc.c

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
#include "amc.h"
#include "gpio.h"
#include "event.h"
#include "module.h"
#include "lpc21nn.h"
#include "mmcio.h"
#include "iopin.h"
#include "i2c.h"
#include "event.h"
#include "debug.h"
#include "arch.h" 
#include "ws.h"
#include "stdio.h"
#include "req.h"
#include "timer.h"

#ifndef uchar
#define uchar unsigned char
#endif

#define NUM_AMC_SLOTS	16

#define MAX_SDR_DATA	64
#define MAX_FRU_DATA	256
#define MAX_SDR_COUNT		8
#define MAX_SENSOR_COUNT	8

typedef struct _sdr_data {
	uchar record_id;
	uchar record_len;
	uchar sdr[MAX_SDR_DATA];
} SDR_DATA;

typedef struct _fru_data {
	uchar	fru_dev_id;
	unsigned int	fru_inventory_area_size;
	uchar	fru[MAX_FRU_DATA];
} FRU_DATA;

// per AMC data structure, gets filled during device discovery
typedef struct _amc_info {
	uchar 			state;
	GET_DEVICE_ID_CMD_RESP		device_id;
	GET_PICMG_PROPERTIES_CMD_RESP 	picmg_properties;
	GET_DEVICE_SDR_INFO_RESP 	sdr_info;
	uchar			sdr_to_process;
	SDR_DATA			sdr_data[MAX_SDR_COUNT];
	GET_FRU_INVENTORY_AREA_CMD_RESP	fru_info;
	FRU_CONTROL_CAPABILITIES_CMD_RESP	fru_capabilities;
	unsigned short			current_fru_offset;
	uchar			current_record_len;
	uchar			current_read_len;
	uchar			record_read;
	uchar			fru_read_state;
	FRU_DATA			fru_data;
	GET_CLOCK_STATE_CMD_RESP	clock_state;
	GET_AMC_PORT_STATE_CMD_RESP	port_state;
} AMC_INFO;

uchar mcmc_ipmbl_address;
AMC_INFO amc[NUM_AMC_SLOTS];

uchar discovery_state[NUM_AMC_SLOTS];
unsigned discovery_cmd_retry_timer_handle;


#define AMC_STATE_RESET		0
#define AMC_STATE_RUNNING	1

#define DEBUG_OUTGOING

void module_init2( void );
void cmd_complete( IPMI_WS *ws, int status );
void send_set_fru_led_state( uchar ipmi_ch, uchar dev_addr, uchar led_state, 
		void( *completion_function )( void *, int ) );
void send_get_device_id( uchar ipmi_ch, uchar dev_addr, 
		void( *completion_function )( void *, int ) );
void send_get_fru_inventory_area_info( uchar ipmi_ch, uchar dev_addr, 
		void( *completion_function )( void *, int ) );
void send_read_fru_data( uchar ipmi_ch, uchar dev_addr, unsigned short offset, 
		uchar count, void( *completion_function )( void *, int ) );
void send_fru_control( uchar ipmi_ch, uchar dev_addr, uchar action, 
		void( *completion_function )( void *, int ) );
void send_get_picmg_properties( uchar ipmi_ch, uchar dev_addr, 
		void( *completion_function )( void *, int ) );
void send_get_sensor_reading( uchar ipmi_ch, uchar dev_addr, uchar sensor_number, 
		void( *completion_function )( void *, int ) );
void send_get_device_sdr_info( uchar ipmi_ch, uchar dev_addr, uchar operation, 
		void( *completion_function )( void *, int ) );
void send_get_device_sdr( uchar ipmi_ch, uchar dev_addr, uchar rec_id, 
		void( *completion_function )( void *, int ) );
void send_reserve_device_sdr_repository( uchar ipmi_ch, uchar dev_addr, 
		void( *completion_function )( void *, int ) );
void send_get_fru_led_properties( uchar ipmi_ch, uchar dev_addr, 
		void( *completion_function )( void *, int ) );
void send_get_led_color_capabilities( uchar ipmi_ch, uchar dev_addr, uchar led_id, 
		void( *completion_function )( void *, int ) );
void send_get_fru_led_state( uchar ipmi_ch, uchar dev_addr, 
		void( *completion_function )( void *, int ) );
void send_get_device_locator_record_id( uchar ipmi_ch, uchar dev_addr, 
		void( *completion_function )( void *, int ) );
void send_get_fru_control_capabilities( uchar ipmi_ch, uchar dev_addr, 
		void( *completion_function )( void *, int ) );

void dump_outgoing( IPMI_WS *req_ws );
void mcmc_mmc_event( uchar dev_id, uchar event );
unsigned short get_next_fru_offset( uchar dev_id, uchar current_offset );
void dump_amc_info( uchar dev_id );
void dump_discovery_state( uchar dev_id );

uchar lookup_dev_id( uchar dev_addr );
uchar lookup_dev_addr( uchar dev_id );

void enable_payload( uchar dev_id );
void device_discovery( uchar dev_id );
void start_chassis_device_discovery( void );
void discovery_cmd_complete( IPMI_WS *ws, int status );
void discovery_cmd_retry( uchar *arg );


#define IPMBL_TABLE_SIZE	27
#define NUM_SLOTS		IPMBL_TABLE_SIZE

uchar IPMBL_TABLE[IPMBL_TABLE_SIZE] = { 
	0x70, 0x8A, 0x72, 0x8E, 0x92, 0x90, 0x74, 0x8C, 0x76, 0x98, 0x9C,
	0x9A, 0xA0, 0xA4, 0x88, 0x9E, 0x86, 0x84, 0x78, 0x94, 0x7A, 0x96,
	0x82, 0x80, 0x7C, 0x7E, 0xA2 };

typedef struct slot_info {
	unsigned long long pin;
	uchar amc_available;
} SLOT_INFO;

SLOT_INFO slot_info[NUM_SLOTS] = {
	{BP_0, 0}, {BP_1, 0}, {BP_2, 0}, {BP_3, 0}, {BP_4, 0}, {BP_5, 0},
       	{BP_6, 0}, {BP_7, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
		 {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, 
		 {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
		 {0, 0}, {0, 0}, {0, 0}
};

uchar g_dev_id = 0xff;	// dev id used for debugging & console commands

/* TODO
uTCA REQ 3.22 Each PM and CU EMMC shall use an IPMB-0 address based on its Geographic
Address inputs defined in Table 3-7.

REQ 3.71 As an exception to REQ 3.70, power negotiation and E-Keying shall be the
responsibility of Carrier Managers.

REQ 3.72 As an exception to REQ 3.70, FRU activation and deactivation shall be the
responsibility of each Carrier Manager, Shelf Manager or the System Manager,
depending on the Activation Control and Deactivation Control fields, respectively, of
the MicroTCA Carrier Activation and Power Management record.
REQ 3.73 As an exception to REQ 3.70, the hardware components providing the Carrier Manager
function should include a watchdog function.
REQ 3.74 Carrier Managers shall meet the Carrier Hot Swap requirements in AMC.0 Section
3.6.6 "Module Hot Swap sensor" except for power management which shall be
implemented as specified in Section 3.8.4, "Nor~nal power management".

REQ 3.75
REQ 3.76
REQ 3.77
REQ 3.78
REQ 3.79
REQ 3.80
REQ 3.81
REQ 3.82
REQ 3.83
REQ 3.84
REQ 3.85
REQ 3.86
REQ 3.87
REQ 3.88
REQ 3.89
REQ 3.90
REQ 3.91
REQ 3.92
Carrier Managers shall only enable IPMB-L isolators and pull-up power for an
AdvancedMC Slot when an AdvancedMC is present, and Management Power is
enabled and good.
If communication errors on IPMB-L are detected and isolation is required, Carrier
Managers shall isolate the errant Module Management Controller (MMC) and
transition the AdvanccdMC to communication lost state M7.
On re-enabling IPMB-L to an AdvancedMC, if communication is restored, Carrier
Managers shall transition the AdvancedMC back to a known state as communication is
regained.
Carrier Managers shall not allow the "Master Write-Read" IPMI command to be used
to access devices on either the IPMB-L or IPMB-0 interfaces.
In a modular MicroTCA Carrier, each MCMC shall implement an I*C interface to its
Carrier FRU Information device as specified in Section 3.2.4, "Carrier FRU
Information device I-equireinents".
Carrier Managers shall implement the Shelf-Carrier Manager Interface as described in
Section 3.4.3. "Shelf-Carrier Manager Interface''.
Carrier Managers shall manage power for the FRU(s) as per the requirements of
Section 3.8, "Power management".
Carrier Managers shall manage MicroTCA Carrier Backplane interconnects as
specified in Section 3.10, "Electronic Keying".
Carrier Managers shall use addressing as specified in Section 3.3.6, "Addressing".
Carrier Managers shall support all commands in I-able 336. "Command numbelassignments
and requireinet-tts" in the column titled Canier Mgr. Req and identified as
mandatory.
Carrier Managers shall return IPMI Completion Codes, as described in the
"Completion Codes" section of the IPMI specification, except as specified in this
specification.
Each Carrier Manager shall return an PMI Completion Code of "Parameter out of
range (C9h)" for any request where the FRU Device ID refers to a FRU that is not
implemented in its MicroTCA Carrier.
For all cornmands that take a FRU Device ID as request data, if that FRU is not present,
Carrier Managers shall return the "Destination unavailable (D3h)" Completion Code.
When Carrier Managers receives a "Node Busy (COh)" Completion Code, they should
retry the command that triggered that response.
Each Carrier Manager shall implement a Device SDR Repository, as specified in
Section 3.13.4. "Canier hlanagcr SDR requirements".
Carrier Managers shall comply with the IPM Controller requirements, as defined in
Section 3.6.2 "FRU Information access commands" of the AdvancedTCA specification,
except for FRU Device ID, which shall be assigned in accordance with Table 3-3,
Table 3-5, Table 3-7, and Section 3.2.3 in this specification.
Carrier Managers shall comply with the SEL requirements, as specified in Section 3.12.
"Systan Event Log".
Each Carrier Manager shall periodically verify that its MicroTCA Carrier FRUs are
still present and responding.

REQ 3.93 When communication is lost between the Carrier Manager and an MCMC or (E)MMC,
the Carrier Manager shall not disable Payload Power or Management Power to the
corresponding FRU, except as specified in Section 3.8. "Power rnanagcment" for PM
EMMCs.
REQ 3.94 If an MCMC or MMC enters the Co~nrnunication Lost state, M7, Carrier Managers
shall not change the associated E-Keying.
REQ 3.96 Carrier Managers shall send a FRU Hot Swap Sensor event message to the Shelf
Manager to alert it of communication loss with a FRU.
REQ 3.91; Carrier Managers shall disable the other ends of the Fabric Interface Ports connected to
any AdvancedMCs or MCMCs that are suddenly removed.

*/

/*==============================================================
 * MCMC ADDRESSING
 *==============================================================*/
uchar
module_get_i2c_address( int address_type )
{
	switch( address_type ) {
		case I2C_ADDRESS_LOCAL:
			return 0x20;
			break;
		case I2C_ADDRESS_REMOTE:	// used for debugging
			if( g_dev_id == 0xff ) {
				return 0x9c;
			} else {
				return ( lookup_dev_addr( g_dev_id ) );
			}
			break;
		default:
			return 0;
	}
}

void
poll_slots( void )
{
	int i;

	for( i = 0; i < NUM_SLOTS; i++ ) {
		if( iopin_get( slot_info[i].pin ) )
			slot_info[i].amc_available = 0;
		else
			slot_info[i].amc_available = 1;
			
	}
}

/* MMC power - up

3.6.1 Typical Module insertion

When the Module’s Management Power is enabled, the BLUE LED turns on as soon 
as possible.

When the Module Handle in the Module is closed, the MMC sends a Module Hot Swap
(Module Handle Closed) event message to the Carrier IPMC, as described in 
Table 3-8, “Module Hot Swap event message.”

The Carrier IPMC sends a “Set FRU LED State” command to the MMC with a request
to perform long blinks of the BLUE LED, indicating to the operator that the new
Module is waiting to be activated.

The Carrier IPMC reads the Module’s Module Current Requirements record and 
AdvancedMC Point-to-Point Connectivity record.

If the Module FRU Information is invalid or if the Carrier cannot provide the
necessary Payload Power then:

The Carrier IPMC sends a “Set FRU LED State” command to the MMC requesting the
“on” state for the BLUE LED. The FRU remains in M1.

On receipt of the “Set FRU Activation (Activate FRU)” command by the Carrier 
IPMC, designating a particular Module, the Carrier IPMC sends an M2 to M3 
transition event message to the Shelf Manager on behalf of the Module and sends
a “set FRU LED State” command to the MMC with a request to turn off the BLUE LED.

The Carrier IPMC enables Payload Power (PWR) for the Module.
*/

/*==============================================================
 * INITIALIZATION
 *==============================================================*/
/*
 * module_init()
 *
 * Gets called when we power up.
 */
void
module_init( void )
{
	module_init2();
}


void
module_init2( void )
{
	
	// get our IPMB-L address
	mcmc_ipmbl_address = module_get_i2c_address( I2C_ADDRESS_LOCAL );

	// module specific initialization for serial & i2c interfaces go in here

}


/*==============================================================
 * STATE CHANGE HANDLING
 *==============================================================*/
/*
REQ 3.55 When the Module Handle transitions to closed, the MMC shall 
send a Module Hot Swap (Module Handle Closed) event message as described
in Table 3-8, “Module Hot Swap event message.”

REQ 3.56 When the Module Handle state transitions to open, the MMC shall
send a Module Hot Swap (Module Handle Opened) event message as described
in Table 3-8, “Module Hot Swap event message.”

REQ 3.57MMCs shall periodically re-send Module Hot Swap event messages 
until either a Command Completed Normally (00h) Completion Code has been 
returned from the Carrier IPMC or the Module wants to send a new Module 
Hot Swap event message.
*/


/*
3.6.2 Typical Module extraction

The operator can initiate deactivation by pulling the Module Handle, which 
changes the state of the Module Handle to open. When the Module Handle in
the Module is opened, the MMC in the Module sends a Module Hot Swap (Handle 
Opened) event message to the Carrier IPMC (as described in Section 3-8, 
“Module Hot Swap event message.” )

The IPMC in the Carrier sends a “Set FRU LED State” command to the MMC 
in the Module with a request to perform short blinks of the BLUE LED. 
This indicates to the operator that the Module is waiting to be deactivated.

The Carrier IPMC now issues “Set AMC Port State (Disable)” command(s) 
for all Ports on the Module and for all Ports that connect to the Module. 
This will disable all Ports associated with the Module about to be removed.

When the Carrier IPMC has transitioned the Module to M6 state, the Carrier 
IPMC sends a “FRU Control (Quiesce)” command to the Module and awaits a 
Module Hot Swap (Quiesced) event message from the MMC.

Next, the Carrier IPMC disables the Module’s Payload Power.
the Carrier IPMC sends a “Set FRU LED State” command to the MMC with a 
request to turn on the BLUE LED. This indicates to the operator that 
the Module is ready to be safely extracted.

The operator removes the Module.
*/



/*==============================================================
 * MMC SPECIFIC COMMANDS
 *==============================================================*/
/*
Module MMC req. mandatory commands

IPMI commands
-------------
Get Device ID
Broadcast “Get Device ID”[1]
Set Event Receiver
Get Event Receiver
Platform Event (a.k.a. “Event Message”)
Get Device SDR Info
Get Device SDR
Reserve Device SDR Repository
Get Sensor Reading
Get FRU Inventory Area Info
Read FRU Data
Write FRU Data

PICMG related commands
----------------------
Get PICMG Properties
FRU Control
Get FRU LED Properties
Get LED Color Capabilities
Set FRU LED State
Get FRU LED State
Get Device Locator Record ID
FRU Control Capabilities
Set AMC Port State
Get AMC Port State
Set Clock State
Get Clock State
*/
/*
3.9.1.4 Set AMC Port State command
The “Set AMC Port State” command is used to enable or disable Ports associated with an
AdvancedMC Channel using Link Descriptor information from an AdvancedMC point-topoint
connectivity record. The AMC specification does not mandate that drivers for LVDS
Ports have the physical ability to be disabled or enabled. The “Set AMC Port State”
command contains Link information. The Link information could be used by Modules that
provide a configurable interface. The Module could configure the interface to match the type
used in the “Set AMC Port State” command allowing for the creation of Modules that can be
configured to support a variety of Fabric Interfaces. The MMC might receive this command
at any time as other Modules are inserted into or extracted from the Carrier.


3.9.1.5 Get AMC Port State command
The “Get AMC Port State” command provides a way to query the current Link, if any, on an
AdvancedMC Channel. In the request, the AdvancedMC Channel ID is provided to the
MMC. The MMC returns a response containing the state of that Link.

3.9.2 Clock E-Keying
By default, all AMC clocks and on-Carrier clock resources are in the 
disabled state when the AMC Module or Carrier is initially activated.

Requirements
REQ 3.182 Carriers and Modules shall support the “Set Clock State” command
defined in Table 3-44, “ Set Clock State command” for all implemented clocks.

REQ 3.186 Carriers and Modules shall support the “Get Clock State” command 
defined in Table 3-45, “ Get Clock State command.” If the designated clock 
resource has no clock enabled, Carriers and Modules shall indicate the clock 
as “Disabled” in the Clock State field.

3.10 Module Payload control
The “FRU Control Capabilities” Command provides a way to query which 
specific options an AdvancedMC supports in the “FRU Control” command. These
capabilities are expected to be static throughout the life of the AdvancedMC.
The “FRU Control (Cold Reset)” variant is mandatory, so the corresponding 
bit is marked reserved and the System Manager can assume that it is always 
supported.

REQ 3.53 The “Get Sensor Reading” command shall be implemented in the 
MMC to enable the Carrier IPMC to determine the current state of the 
Module. The Carrier IPMC can issue this command at any time to get 
the sensor status. This command can also be used by a Carrier when it
regains contact with a Module after a loss of contact (see Section 
3.6.8, “Communication lost.” )
*/

void
send_amc_set_port_state( uchar ipmi_ch, uchar dev_addr, void( *completion_function )( void *, int ) )
{
	IPMI_PKT *pkt;
	IPMI_WS *req_ws;	
	uchar seq;
	uchar responder_slave_addr;
	SET_AMC_PORT_STATE_CMD_REQ *req;
	IPMI_IPMB_REQUEST *ipmb_req;

	if( !( req_ws = ws_alloc() ) ) {
		return;
	}
	
	pkt = &( req_ws->pkt );
	ipmb_req = ( IPMI_IPMB_REQUEST * )&( req_ws->pkt_out );
	pkt->req = ( IPMI_CMD_REQ * )&( ipmb_req->command ) ;
	req = ( SET_AMC_PORT_STATE_CMD_REQ * )pkt->req;	
	ipmi_get_next_seq( &seq );

	pkt->hdr.ws = (char *)req_ws;
	pkt->hdr.req_data_len = sizeof( SET_AMC_PORT_STATE_CMD_REQ ) - 1;
	
	req->command = ATCA_CMD_SET_AMC_PORT_STATE;
	req->picmg_id = 0;
	req->link_grp_id = 0;		/* Link Grouping ID */
	req->link_type_ext = 0;		/* Link Type Extension */
	req->link_type = 0;		/* Link Type */
	req->lane_3_bit_flag = 0;	/* Lane 3 Bit Flag */
	req->lane_2_bit_flag = 0;	/* Lane 2 Bit Flag */
	req->lane_1_bit_flag = 0;	/* Lane 1 Bit Flag */
	req->lane_0_bit_flag = 0;	/* Lane 0 Bit Flag */
	req->amc_channel_id = 0;	/* AMC Channel ID */

	ipmb_req->requester_slave_addr = module_get_i2c_address( I2C_ADDRESS_LOCAL );
	ipmb_req->netfn = NETFN_GROUP_EXTENSION_REQ;
	ipmb_req->requester_lun = 0;
	responder_slave_addr = dev_addr;
	ipmb_req->header_checksum = -( *( char * )ipmb_req + responder_slave_addr );
	ipmb_req->req_seq = seq;
	ipmb_req->responder_lun = 0;
	ipmb_req->command = req->command;
	/* The location of data_checksum field is bogus.
	 * It's used as a placeholder to indicate that a checksum follows the data field.
	 * The location of the data_checksum depends on the size of the data preceeding it.*/
	ipmb_req->data_checksum = 
		ipmi_calculate_checksum( &ipmb_req->requester_slave_addr, 
			pkt->hdr.req_data_len + 3 ); 
	req_ws->len_out = sizeof( IPMI_IPMB_REQUEST ) 
				- IPMB_REQ_MAX_DATA_LEN  +  pkt->hdr.req_data_len;
	/* Assign the checksum to it's proper location */
	*( ( uchar * )ipmb_req + req_ws->len_out - 1 ) = ipmb_req->data_checksum; 

	req_ws->ipmi_completion_function = completion_function;
	req_ws->addr_out = dev_addr;
	req_ws->outgoing_protocol = IPMI_CH_PROTOCOL_IPMB;
	req_ws->outgoing_medium = IPMI_CH_MEDIUM_IPMB;
	req_ws->outgoing_channel = ipmi_ch;
	dump_outgoing( req_ws );

	/* dispatch the request */
	ws_set_state( req_ws, WS_ACTIVE_MASTER_WRITE );
}

/*
ATCA_CMD_SET_FRU_LED_STATE
ATCA_CMD_FRU_CONTROL
*/
#define LED_OFF			0
#define LED_ON			1
#define LED_LONG_BLINK		2
#define LED_SHORT_BLINK		3

void
send_set_fru_led_state( uchar ipmi_ch, uchar dev_addr, uchar led_state, void( *completion_function )( void *, int ) )
{
	IPMI_PKT *pkt;
	IPMI_WS *req_ws;	
	uchar seq;
	uchar responder_slave_addr;
	SET_FRU_LED_STATE_CMD_REQ *req;
	IPMI_IPMB_REQUEST *ipmb_req;

	if( !( req_ws = ws_alloc() ) ) {
		return;
	}
	
	pkt = &( req_ws->pkt );
	ipmb_req = ( IPMI_IPMB_REQUEST * )&( req_ws->pkt_out );
	pkt->req = ( IPMI_CMD_REQ * )&( ipmb_req->command ) ;
	req = ( SET_FRU_LED_STATE_CMD_REQ * )pkt->req;	
	ipmi_get_next_seq( &seq );

	pkt->hdr.ws = (char *)req_ws;
	pkt->hdr.req_data_len = sizeof( SET_FRU_LED_STATE_CMD_REQ ) - 1;
	
	req->command = ATCA_CMD_SET_FRU_LED_STATE;
	req->picmg_id = 0;
	req->fru_dev_id = 0;
	req->led_id = FRU_LED_BLUE;
	switch( led_state ) {
		case LED_OFF:
			req->led_function = 0;
			break;
		case LED_ON:
			req->led_function = 0xff;
			break;
		case LED_LONG_BLINK:
			req->led_function = 200;	// off duration in tens of ms
			req->on_duration = 200;		// tens of ms
			break;
		case LED_SHORT_BLINK:
			req->led_function = 50;		// off duration in tens of ms
			req->on_duration = 50;		// tens of ms
			break;
		default:
			break;
	}
	req->color = 0xff;		// use default color
	
	ipmb_req->requester_slave_addr = module_get_i2c_address( I2C_ADDRESS_LOCAL );
	ipmb_req->netfn = NETFN_GROUP_EXTENSION_REQ;
	ipmb_req->requester_lun = 0;
	responder_slave_addr = dev_addr;
	ipmb_req->header_checksum = -( *( char * )ipmb_req + responder_slave_addr );
	ipmb_req->req_seq = seq;
	ipmb_req->responder_lun = 0;
	ipmb_req->command = req->command;
	/* The location of data_checksum field is bogus.
	 * It's used as a placeholder to indicate that a checksum follows the data field.
	 * The location of the data_checksum depends on the size of the data preceeding it.*/
	ipmb_req->data_checksum = 
		ipmi_calculate_checksum( &( ipmb_req->requester_slave_addr ), 
			pkt->hdr.req_data_len + 3 ); 
	req_ws->len_out = sizeof( IPMI_IPMB_REQUEST ) 
				- IPMB_REQ_MAX_DATA_LEN  +  pkt->hdr.req_data_len;
	/* Assign the checksum to it's proper location */
	*( ( uchar * )ipmb_req + req_ws->len_out - 1 ) = ipmb_req->data_checksum; 

	req_ws->ipmi_completion_function = completion_function;
	req_ws->addr_out = dev_addr;
	req_ws->outgoing_protocol = IPMI_CH_PROTOCOL_IPMB;
	req_ws->outgoing_medium = IPMI_CH_MEDIUM_IPMB;
	req_ws->outgoing_channel = ipmi_ch;
	dump_outgoing( req_ws );

	/* dispatch the request */
	ws_set_state( req_ws, WS_ACTIVE_MASTER_WRITE );
}

// note this is module specific
#define FILL_PARAMS( params ) \
	params.completion_function = completion_function; \
	params.outgoing_protocol = IPMI_CH_PROTOCOL_IPMB; \
	params.outgoing_medium = IPMI_CH_MEDIUM_IPMB; \
	params.bridged = 0;

void
send_get_device_id_new( uchar ipmi_ch, uchar dev_addr, void( *completion_function )( void *, int ) )
{
	GENERIC_CMD_REQ req;
	REQ_PARAMS params;

	req.command = IPMI_CMD_GET_DEVICE_ID;
	FILL_PARAMS( params );
	params.addr_out[0] = dev_addr;
	params.req_data_len = sizeof( GENERIC_CMD_REQ ) - 1;
// TODO	params.netfn = NETFN_APP_REQ;

	req_send( &req, &params );	  // TODO wrong params
}


void
send_get_device_id( uchar ipmi_ch, uchar dev_addr, void( *completion_function )( void *, int ) )
{
	IPMI_PKT *pkt;
	IPMI_WS *req_ws;	
	uchar seq;
	uchar responder_slave_addr;
	GENERIC_CMD_REQ *req;
	IPMI_IPMB_REQUEST *ipmb_req;

	if( !( req_ws = ws_alloc() ) ) {
		return;
	}
	
	pkt = &( req_ws->pkt );
	ipmb_req = ( IPMI_IPMB_REQUEST * )&( req_ws->pkt_out );
	pkt->req = ( IPMI_CMD_REQ * )&( ipmb_req->command ) ;
	req = ( GENERIC_CMD_REQ * )pkt->req;	
	ipmi_get_next_seq( &seq );

	pkt->hdr.ws = (char *)req_ws;
	pkt->hdr.req_data_len = sizeof( GENERIC_CMD_REQ ) - 1;
	
	req->command = IPMI_CMD_GET_DEVICE_ID;

	ipmb_req->requester_slave_addr = module_get_i2c_address( I2C_ADDRESS_LOCAL );
	ipmb_req->netfn = NETFN_APP_REQ;
	ipmb_req->requester_lun = 0;
	responder_slave_addr = dev_addr;
	ipmb_req->header_checksum = -( *( char * )ipmb_req + responder_slave_addr );
	ipmb_req->req_seq = seq;
	ipmb_req->responder_lun = 0;
	ipmb_req->command = req->command;
	/* The location of data_checksum field is bogus.
	 * It's used as a placeholder to indicate that a checksum follows the data field.
	 * The location of the data_checksum depends on the size of the data preceeding it.*/
	ipmb_req->data_checksum = 
		ipmi_calculate_checksum( &ipmb_req->requester_slave_addr, 
			pkt->hdr.req_data_len + 3 ); 
	req_ws->len_out = sizeof( IPMI_IPMB_REQUEST ) 
				- IPMB_REQ_MAX_DATA_LEN  +  pkt->hdr.req_data_len;
	/* Assign the checksum to it's proper location */
	*( ( uchar * )ipmb_req + req_ws->len_out - 1 ) = ipmb_req->data_checksum; 

	req_ws->ipmi_completion_function = completion_function;
	req_ws->addr_out = dev_addr;
	req_ws->outgoing_protocol = IPMI_CH_PROTOCOL_IPMB;
	req_ws->outgoing_medium = IPMI_CH_MEDIUM_IPMB;
	req_ws->outgoing_channel = ipmi_ch;
	dump_outgoing( req_ws );

	/* dispatch the request */
	ws_set_state( req_ws, WS_ACTIVE_MASTER_WRITE );
}

void
send_get_fru_inventory_area_info( uchar ipmi_ch, uchar dev_addr, void( *completion_function )( void *, int ) )
{
	IPMI_PKT *pkt;
	IPMI_WS *req_ws;	
	uchar seq;
	uchar responder_slave_addr;
	GET_FRU_INVENTORY_AREA_INFO_CMD_REQ *req;
	IPMI_IPMB_REQUEST *ipmb_req;

	if( !( req_ws = ws_alloc() ) ) {
		return;
	}
	
	pkt = &( req_ws->pkt );
	ipmb_req = ( IPMI_IPMB_REQUEST * )&( req_ws->pkt_out );
	pkt->req = ( IPMI_CMD_REQ * )&( ipmb_req->command ) ;
	req = ( GET_FRU_INVENTORY_AREA_INFO_CMD_REQ * )pkt->req;	
	ipmi_get_next_seq( &seq );

	pkt->hdr.ws = (char *)req_ws;
	pkt->hdr.req_data_len = sizeof( GET_FRU_INVENTORY_AREA_INFO_CMD_REQ ) - 1;
	
	req->command = IPMI_STO_CMD_GET_FRU_INVENTORY_AREA_INFO;

	ipmb_req->requester_slave_addr = module_get_i2c_address( I2C_ADDRESS_LOCAL );
	ipmb_req->netfn = NETFN_NVSTORE_REQ;
	ipmb_req->requester_lun = 0;
	responder_slave_addr = dev_addr;
	ipmb_req->header_checksum = -( *( char * )ipmb_req + responder_slave_addr );
	ipmb_req->req_seq = seq;
	ipmb_req->responder_lun = 0;
	ipmb_req->command = req->command;
	/* The location of data_checksum field is bogus.
	 * It's used as a placeholder to indicate that a checksum follows the data field.
	 * The location of the data_checksum depends on the size of the data preceeding it.*/
	ipmb_req->data_checksum = 
		ipmi_calculate_checksum( &ipmb_req->requester_slave_addr, 
			pkt->hdr.req_data_len + 3 ); 
	req_ws->len_out = sizeof( IPMI_IPMB_REQUEST ) 
				- IPMB_REQ_MAX_DATA_LEN  +  pkt->hdr.req_data_len;
	/* Assign the checksum to it's proper location */
	*( ( uchar * )ipmb_req + req_ws->len_out - 1 ) = ipmb_req->data_checksum; 

	req_ws->ipmi_completion_function = completion_function;
	req_ws->addr_out = dev_addr;
	req_ws->outgoing_protocol = IPMI_CH_PROTOCOL_IPMB;
	req_ws->outgoing_medium = IPMI_CH_MEDIUM_IPMB;
	req_ws->outgoing_channel = ipmi_ch;
	dump_outgoing( req_ws );

	/* dispatch the request */
	ws_set_state( req_ws, WS_ACTIVE_MASTER_WRITE );
}


void
send_read_fru_data( uchar ipmi_ch, 
		uchar dev_addr, 
		unsigned short offset, 
		uchar count, 
		void( *completion_function )( void *, int ) )
{
	IPMI_PKT *pkt;
	IPMI_WS *req_ws;	
	uchar seq;
	uchar responder_slave_addr;
	READ_FRU_DATA_CMD_REQ *req;
	IPMI_IPMB_REQUEST *ipmb_req;

	if( !( req_ws = ws_alloc() ) ) {
		return;
	}
	
	pkt = &( req_ws->pkt );
	ipmb_req = ( IPMI_IPMB_REQUEST * )&( req_ws->pkt_out );
	pkt->req = ( IPMI_CMD_REQ * )&( ipmb_req->command ) ;
	req = ( READ_FRU_DATA_CMD_REQ * )pkt->req;	
	ipmi_get_next_seq( &seq );

	pkt->hdr.ws = (char *)req_ws;
	pkt->hdr.req_data_len = sizeof( READ_FRU_DATA_CMD_REQ ) - 1;
	
	req->command = IPMI_STO_CMD_READ_FRU_DATA;
	req->fru_dev_id = 0;
	req->fru_inventory_offset_lsb = ( uchar )offset;
	req->fru_inventory_offset_msb = ( uchar )( offset >> 8 );
	req->count_to_read = 20;

	ipmb_req->requester_slave_addr = module_get_i2c_address( I2C_ADDRESS_LOCAL );
	ipmb_req->netfn = NETFN_NVSTORE_REQ;
	ipmb_req->requester_lun = 0;
	responder_slave_addr = dev_addr;
	ipmb_req->header_checksum = -( *( char * )ipmb_req + responder_slave_addr );
	ipmb_req->req_seq = seq;
	ipmb_req->responder_lun = 0;
	ipmb_req->command = req->command;
	/* The location of data_checksum field is bogus.
	 * It's used as a placeholder to indicate that a checksum follows the data field.
	 * The location of the data_checksum depends on the size of the data preceeding it.*/
	ipmb_req->data_checksum = 
		ipmi_calculate_checksum( &ipmb_req->requester_slave_addr, 
			pkt->hdr.req_data_len + 3 ); 
	req_ws->len_out = sizeof( IPMI_IPMB_REQUEST ) 
				- IPMB_REQ_MAX_DATA_LEN  +  pkt->hdr.req_data_len;
	/* Assign the checksum to it's proper location */
	*( ( uchar * )ipmb_req + req_ws->len_out - 1 ) = ipmb_req->data_checksum; 

	req_ws->ipmi_completion_function = completion_function;
	req_ws->addr_out = dev_addr;
	req_ws->outgoing_protocol = IPMI_CH_PROTOCOL_IPMB;
	req_ws->outgoing_medium = IPMI_CH_MEDIUM_IPMB;
	req_ws->outgoing_channel = ipmi_ch;
	dump_outgoing( req_ws );

	/* dispatch the request */
	ws_set_state( req_ws, WS_ACTIVE_MASTER_WRITE );
}


void
send_fru_control( uchar ipmi_ch, 
		uchar dev_addr, 
		uchar action, 
		void( *completion_function )( void *, int ) )
{
	IPMI_PKT *pkt;
	IPMI_WS *req_ws;	
	uchar seq;
	uchar responder_slave_addr;
	FRU_CONTROL_CMD_REQ *req;
	IPMI_IPMB_REQUEST *ipmb_req;

	if( !( req_ws = ws_alloc() ) ) {
		return;
	}
	
	pkt = &( req_ws->pkt );
	ipmb_req = ( IPMI_IPMB_REQUEST * )&( req_ws->pkt_out );
	pkt->req = ( IPMI_CMD_REQ * )&( ipmb_req->command ) ;
	req = ( FRU_CONTROL_CMD_REQ * )pkt->req;	
	ipmi_get_next_seq( &seq );

	pkt->hdr.ws = (char *)req_ws;
	pkt->hdr.req_data_len = sizeof( FRU_CONTROL_CMD_REQ ) - 1;

	req->command = ATCA_CMD_FRU_CONTROL;
	req->picmg_id = 0;
	req->fru_dev_id = 0;
	req->fru_control_options = action;
	
	ipmb_req->requester_slave_addr = module_get_i2c_address( I2C_ADDRESS_LOCAL );
	ipmb_req->netfn = NETFN_PICMG_REQ;
	ipmb_req->requester_lun = 0;
	responder_slave_addr = dev_addr;
	ipmb_req->header_checksum = -( *( char * )ipmb_req + responder_slave_addr );
	ipmb_req->req_seq = seq;
	ipmb_req->responder_lun = 0;
	ipmb_req->command = req->command;
	/* The location of data_checksum field is bogus.
	 * It's used as a placeholder to indicate that a checksum follows the data field.
	 * The location of the data_checksum depends on the size of the data preceeding it.*/
	ipmb_req->data_checksum = 
		ipmi_calculate_checksum( &ipmb_req->requester_slave_addr, 
			pkt->hdr.req_data_len + 3 ); 
	req_ws->len_out = sizeof( IPMI_IPMB_REQUEST ) 
				- IPMB_REQ_MAX_DATA_LEN  +  pkt->hdr.req_data_len;
	/* Assign the checksum to it's proper location */
	*( ( uchar * )ipmb_req + req_ws->len_out - 1 ) = ipmb_req->data_checksum; 

	req_ws->ipmi_completion_function = completion_function;
	req_ws->addr_out = dev_addr;
	req_ws->outgoing_protocol = IPMI_CH_PROTOCOL_IPMB;
	req_ws->outgoing_medium = IPMI_CH_MEDIUM_IPMB;
	req_ws->outgoing_channel = ipmi_ch;
	dump_outgoing( req_ws );

	/* dispatch the request */
	ws_set_state( req_ws, WS_ACTIVE_MASTER_WRITE );
}


void
send_get_picmg_properties( uchar ipmi_ch, 
		uchar dev_addr, 
		void( *completion_function )( void *, int ) )
{
	IPMI_PKT *pkt;
	IPMI_WS *req_ws;	
	uchar seq;
	uchar responder_slave_addr;
	GET_PICMG_PROPERTIES_CMD_REQ *req;
	IPMI_IPMB_REQUEST *ipmb_req;

	if( !( req_ws = ws_alloc() ) ) {
		return;
	}
	
	pkt = &( req_ws->pkt );
	ipmb_req = ( IPMI_IPMB_REQUEST * )&( req_ws->pkt_out );
	pkt->req = ( IPMI_CMD_REQ * )&( ipmb_req->command ) ;
	req = ( GET_PICMG_PROPERTIES_CMD_REQ * )pkt->req;	
	ipmi_get_next_seq( &seq );

	pkt->hdr.ws = (char *)req_ws;
	pkt->hdr.req_data_len = sizeof( GET_PICMG_PROPERTIES_CMD_REQ ) - 1;
	
	req->command = ATCA_CMD_GET_PICMG_PROPERTIES;
	req->picmg_id = 0;
		
	ipmb_req->requester_slave_addr = module_get_i2c_address( I2C_ADDRESS_LOCAL );
	ipmb_req->netfn = NETFN_PICMG_REQ;
	ipmb_req->requester_lun = 0;
	responder_slave_addr = dev_addr;
	ipmb_req->header_checksum = -( *( char * )ipmb_req + responder_slave_addr );
	ipmb_req->req_seq = seq;
	ipmb_req->responder_lun = 0;
	ipmb_req->command = req->command;
	/* The location of data_checksum field is bogus.
	 * It's used as a placeholder to indicate that a checksum follows the data field.
	 * The location of the data_checksum depends on the size of the data preceeding it.*/
	ipmb_req->data_checksum = 
		ipmi_calculate_checksum( &ipmb_req->requester_slave_addr, 
			pkt->hdr.req_data_len + 3 ); 
	req_ws->len_out = sizeof( IPMI_IPMB_REQUEST ) 
				- IPMB_REQ_MAX_DATA_LEN  +  pkt->hdr.req_data_len;
	/* Assign the checksum to it's proper location */
	*( ( uchar * )ipmb_req + req_ws->len_out - 1 ) = ipmb_req->data_checksum; 

	req_ws->ipmi_completion_function = completion_function;
	req_ws->addr_out = dev_addr;
	req_ws->outgoing_protocol = IPMI_CH_PROTOCOL_IPMB;
	req_ws->outgoing_medium = IPMI_CH_MEDIUM_IPMB;
	req_ws->outgoing_channel = ipmi_ch;
	dump_outgoing( req_ws );

	/* dispatch the request */
	ws_set_state( req_ws, WS_ACTIVE_MASTER_WRITE );
}


void
send_get_sensor_reading( 
		uchar ipmi_ch, 
		uchar dev_addr, 
		uchar sensor_number, 
		void( *completion_function )( void *, int ) )
{
	IPMI_PKT *pkt;
	IPMI_WS *req_ws;	
	uchar seq;
	uchar responder_slave_addr;
	GET_SENSOR_READING_CMD_REQ *req;
	IPMI_IPMB_REQUEST *ipmb_req;

	if( !( req_ws = ws_alloc() ) ) {
		return;
	}
	
	pkt = &( req_ws->pkt );
	ipmb_req = ( IPMI_IPMB_REQUEST * )&( req_ws->pkt_out );
	pkt->req = ( IPMI_CMD_REQ * )&( ipmb_req->command ) ;
	req = ( GET_SENSOR_READING_CMD_REQ * )pkt->req;	
	ipmi_get_next_seq( &seq );

	pkt->hdr.ws = (char *)req_ws;
	pkt->hdr.req_data_len = sizeof( GET_SENSOR_READING_CMD_REQ ) - 1;
	
	req->command = IPMI_SE_CMD_GET_SENSOR_READING;
	req->sensor_number = sensor_number;
		
	ipmb_req->requester_slave_addr = module_get_i2c_address( I2C_ADDRESS_LOCAL );
	ipmb_req->netfn = NETFN_EVENT_REQ;
	ipmb_req->requester_lun = 0;
	responder_slave_addr = dev_addr;
	ipmb_req->header_checksum = -( *( char * )ipmb_req + responder_slave_addr );
	ipmb_req->req_seq = seq;
	ipmb_req->responder_lun = 0;
	ipmb_req->command = req->command;
	/* The location of data_checksum field is bogus.
	 * It's used as a placeholder to indicate that a checksum follows the data field.
	 * The location of the data_checksum depends on the size of the data preceeding it.*/
	ipmb_req->data_checksum = 
		ipmi_calculate_checksum( &ipmb_req->requester_slave_addr, 
			pkt->hdr.req_data_len + 3 ); 
	req_ws->len_out = sizeof( IPMI_IPMB_REQUEST ) 
				- IPMB_REQ_MAX_DATA_LEN  +  pkt->hdr.req_data_len;
	/* Assign the checksum to it's proper location */
	*( ( uchar * )ipmb_req + req_ws->len_out - 1 ) = ipmb_req->data_checksum; 

	req_ws->ipmi_completion_function = completion_function;
	req_ws->addr_out = dev_addr;
	req_ws->outgoing_protocol = IPMI_CH_PROTOCOL_IPMB;
	req_ws->outgoing_medium = IPMI_CH_MEDIUM_IPMB;
	req_ws->outgoing_channel = ipmi_ch;
	dump_outgoing( req_ws );

	/* dispatch the request */
	ws_set_state( req_ws, WS_ACTIVE_MASTER_WRITE );
}

void
send_get_device_sdr_info( 
		uchar ipmi_ch,
		uchar dev_addr,
		uchar operation, /* 1b = Get SDR count. This returns 
				   the total number of SDRs in the device.
				   0b = Get Sensor count. This returns the 
				   number of sensors implemented on LUN this
				   command was addressed to */
		void( *completion_function )( void *, int ) ) 
{
	IPMI_PKT *pkt;
	IPMI_WS *req_ws;	
	uchar seq;
	uchar responder_slave_addr;
	GET_DEVICE_SDR_INFO_CMD *req;
	IPMI_IPMB_REQUEST *ipmb_req;

	if( !( req_ws = ws_alloc() ) ) {
		return;
	}
	
	pkt = &( req_ws->pkt );
	ipmb_req = ( IPMI_IPMB_REQUEST * )&( req_ws->pkt_out );
	pkt->req = ( IPMI_CMD_REQ * )&( ipmb_req->command ) ;
	req = ( GET_DEVICE_SDR_INFO_CMD * )pkt->req;	
	ipmi_get_next_seq( &seq );

	pkt->hdr.ws = (char *)req_ws;
	pkt->hdr.req_data_len = sizeof( GET_DEVICE_SDR_INFO_CMD ) - 1;
	
	req->command = IPMI_SE_CMD_GET_DEVICE_SDR_INFO;
	req->operation = operation;
		
	ipmb_req->requester_slave_addr = module_get_i2c_address( I2C_ADDRESS_LOCAL );
	ipmb_req->netfn = NETFN_EVENT_REQ;
	ipmb_req->requester_lun = 0;
	responder_slave_addr = dev_addr;
	ipmb_req->header_checksum = -( *( char * )ipmb_req + responder_slave_addr );
	ipmb_req->req_seq = seq;
	ipmb_req->responder_lun = 0;
	ipmb_req->command = req->command;
	/* The location of data_checksum field is bogus.
	 * It's used as a placeholder to indicate that a checksum follows the data field.
	 * The location of the data_checksum depends on the size of the data preceeding it.*/
	ipmb_req->data_checksum = 
		ipmi_calculate_checksum( &ipmb_req->requester_slave_addr, 
			pkt->hdr.req_data_len + 3 ); 
	req_ws->len_out = sizeof( IPMI_IPMB_REQUEST ) 
				- IPMB_REQ_MAX_DATA_LEN  +  pkt->hdr.req_data_len;
	/* Assign the checksum to it's proper location */
	*( ( uchar * )ipmb_req + req_ws->len_out - 1 ) = ipmb_req->data_checksum; 

	req_ws->ipmi_completion_function = completion_function;
	req_ws->addr_out = dev_addr;
	req_ws->outgoing_protocol = IPMI_CH_PROTOCOL_IPMB;
	req_ws->outgoing_medium = IPMI_CH_MEDIUM_IPMB;
	req_ws->outgoing_channel = ipmi_ch;
	dump_outgoing( req_ws );

	/* dispatch the request */
	ws_set_state( req_ws, WS_ACTIVE_MASTER_WRITE );
}

void	
send_get_device_sdr( 
		uchar ipmi_ch, 
		uchar dev_addr, 
		uchar rec_id, 
		void( *completion_function )( void *, int ) )
{
	IPMI_PKT *pkt;
	IPMI_WS *req_ws;	
	uchar seq;
	uchar responder_slave_addr;
	GET_DEVICE_SDR_CMD *req;
	IPMI_IPMB_REQUEST *ipmb_req;

	if( !( req_ws = ws_alloc() ) ) {
		return;
	}
	
	pkt = &( req_ws->pkt );
	ipmb_req = ( IPMI_IPMB_REQUEST * )&( req_ws->pkt_out );
	pkt->req = ( IPMI_CMD_REQ * )&( ipmb_req->command ) ;
	req = ( GET_DEVICE_SDR_CMD * )pkt->req;	
	ipmi_get_next_seq( &seq );

	pkt->hdr.ws = (char *)req_ws;
	pkt->hdr.req_data_len = sizeof( GET_DEVICE_SDR_CMD ) - 1;
	
	req->command = IPMI_SE_CMD_GET_DEVICE_SDR;
	req->reservation_id_lsb = 0;    /* Reservation ID. LS Byte. Only required
				   	   for partial reads with a non-zero
				   	   ‘Offset into record’ field. Use 0000h
				   	   for reservation ID otherwise. */
	req->reservation_id_msb = 0;	/* Reservation ID. MS Byte. */
	req->record_id_lsb = 0;		/* Record ID of record to Get, LS Byte. 
					   0000h returns the first record. */
	req->record_id_msb = 0;		/* Record ID of record to Get, MS Byte */
	req->offset = 0;		/* Offset into record */
	req->bytes_to_read = 0xff;	/* Bytes to read. FFh means read entire record. */
		
	ipmb_req->requester_slave_addr = module_get_i2c_address( I2C_ADDRESS_LOCAL );
	ipmb_req->netfn = NETFN_EVENT_REQ;
	ipmb_req->requester_lun = 0;
	responder_slave_addr = dev_addr;
	ipmb_req->header_checksum = -( *( char * )ipmb_req + responder_slave_addr );
	ipmb_req->req_seq = seq;
	ipmb_req->responder_lun = 0;
	ipmb_req->command = req->command;
	/* The location of data_checksum field is bogus.
	 * It's used as a placeholder to indicate that a checksum follows the data field.
	 * The location of the data_checksum depends on the size of the data preceeding it.*/
	ipmb_req->data_checksum = 
		ipmi_calculate_checksum( &ipmb_req->requester_slave_addr, 
			pkt->hdr.req_data_len + 3 ); 
	req_ws->len_out = sizeof( IPMI_IPMB_REQUEST ) 
				- IPMB_REQ_MAX_DATA_LEN  +  pkt->hdr.req_data_len;
	/* Assign the checksum to it's proper location */
	*( ( uchar * )ipmb_req + req_ws->len_out - 1 ) = ipmb_req->data_checksum; 

	req_ws->ipmi_completion_function = completion_function;
	req_ws->addr_out = dev_addr;
	req_ws->outgoing_protocol = IPMI_CH_PROTOCOL_IPMB;
	req_ws->outgoing_medium = IPMI_CH_MEDIUM_IPMB;
	req_ws->outgoing_channel = ipmi_ch;
	dump_outgoing( req_ws );

	/* dispatch the request */
	ws_set_state( req_ws, WS_ACTIVE_MASTER_WRITE );
}

void
send_reserve_device_sdr_repository( 
		uchar ipmi_ch, 
		uchar dev_addr, 
		void( *completion_function )( void *, int ) )
{
	IPMI_PKT *pkt;
	IPMI_WS *req_ws;	
	uchar seq;
	uchar responder_slave_addr;
	GENERIC_CMD_REQ *req;
	IPMI_IPMB_REQUEST *ipmb_req;

	if( !( req_ws = ws_alloc() ) ) {
		return;
	}
	
	pkt = &( req_ws->pkt );
	ipmb_req = ( IPMI_IPMB_REQUEST * )&( req_ws->pkt_out );
	pkt->req = ( IPMI_CMD_REQ * )&( ipmb_req->command ) ;
	req = ( GENERIC_CMD_REQ * )pkt->req;	
	ipmi_get_next_seq( &seq );

	pkt->hdr.ws = (char *)req_ws;
	pkt->hdr.req_data_len = sizeof( GENERIC_CMD_REQ ) - 1;
	
	req->command = IPMI_SE_CMD_RSV_DEVICE_SDR_REPOSITORY;
		
	ipmb_req->requester_slave_addr = module_get_i2c_address( I2C_ADDRESS_LOCAL );
	ipmb_req->netfn = NETFN_EVENT_REQ;
	ipmb_req->requester_lun = 0;
	responder_slave_addr = dev_addr;
	ipmb_req->header_checksum = -( *( char * )ipmb_req + responder_slave_addr );
	ipmb_req->req_seq = seq;
	ipmb_req->responder_lun = 0;
	ipmb_req->command = req->command;
	/* The location of data_checksum field is bogus.
	 * It's used as a placeholder to indicate that a checksum follows the data field.
	 * The location of the data_checksum depends on the size of the data preceeding it.*/
	ipmb_req->data_checksum = 
		ipmi_calculate_checksum( &ipmb_req->requester_slave_addr, 
			pkt->hdr.req_data_len + 3 ); 
	req_ws->len_out = sizeof( IPMI_IPMB_REQUEST ) 
				- IPMB_REQ_MAX_DATA_LEN  +  pkt->hdr.req_data_len;
	/* Assign the checksum to it's proper location */
	*( ( uchar * )ipmb_req + req_ws->len_out - 1 ) = ipmb_req->data_checksum; 

	req_ws->ipmi_completion_function = completion_function;
	req_ws->addr_out = dev_addr;
	req_ws->outgoing_protocol = IPMI_CH_PROTOCOL_IPMB;
	req_ws->outgoing_medium = IPMI_CH_MEDIUM_IPMB;
	req_ws->outgoing_channel = ipmi_ch;
	dump_outgoing( req_ws );

	/* dispatch the request */
	ws_set_state( req_ws, WS_ACTIVE_MASTER_WRITE );
}

void
send_get_fru_led_properties( 
		uchar ipmi_ch, 
		uchar dev_addr, 
		void( *completion_function )( void *, int ) )
{
	IPMI_PKT *pkt;
	IPMI_WS *req_ws;	
	uchar seq;
	uchar responder_slave_addr;
	GET_LED_PROPERTIES_CMD_REQ *req;
	IPMI_IPMB_REQUEST *ipmb_req;

	if( !( req_ws = ws_alloc() ) ) {
		return;
	}
	
	pkt = &( req_ws->pkt );
	ipmb_req = ( IPMI_IPMB_REQUEST * )&( req_ws->pkt_out );
	pkt->req = ( IPMI_CMD_REQ * )&( ipmb_req->command ) ;
	req = ( GET_LED_PROPERTIES_CMD_REQ * )pkt->req;	
	ipmi_get_next_seq( &seq );

	pkt->hdr.ws = (char *)req_ws;
	pkt->hdr.req_data_len = sizeof( GET_LED_PROPERTIES_CMD_REQ ) - 1;
	
	req->command = ATCA_CMD_GET_FRU_LED_PROPERTIES;
	req->picmg_id = 0;
	req->fru_dev_id = 0; 
		
	ipmb_req->requester_slave_addr = module_get_i2c_address( I2C_ADDRESS_LOCAL );
	ipmb_req->netfn = NETFN_PICMG_REQ;
	ipmb_req->requester_lun = 0;
	responder_slave_addr = dev_addr;
	ipmb_req->header_checksum = -( *( char * )ipmb_req + responder_slave_addr );
	ipmb_req->req_seq = seq;
	ipmb_req->responder_lun = 0;
	ipmb_req->command = req->command;
	/* The location of data_checksum field is bogus.
	 * It's used as a placeholder to indicate that a checksum follows the data field.
	 * The location of the data_checksum depends on the size of the data preceeding it.*/
	ipmb_req->data_checksum = 
		ipmi_calculate_checksum( &ipmb_req->requester_slave_addr, 
			pkt->hdr.req_data_len + 3 ); 
	req_ws->len_out = sizeof( IPMI_IPMB_REQUEST ) 
				- IPMB_REQ_MAX_DATA_LEN  +  pkt->hdr.req_data_len;
	/* Assign the checksum to it's proper location */
	*( ( uchar * )ipmb_req + req_ws->len_out - 1 ) = ipmb_req->data_checksum; 

	req_ws->ipmi_completion_function = completion_function;
	req_ws->addr_out = dev_addr;
	req_ws->outgoing_protocol = IPMI_CH_PROTOCOL_IPMB;
	req_ws->outgoing_medium = IPMI_CH_MEDIUM_IPMB;
	req_ws->outgoing_channel = ipmi_ch;
	dump_outgoing( req_ws );

	/* dispatch the request */
	ws_set_state( req_ws, WS_ACTIVE_MASTER_WRITE );
}

void
send_get_led_color_capabilities( 
		uchar ipmi_ch, 
		uchar dev_addr, 
		uchar led_id, 
		void( *completion_function )( void *, int ) )
{
	IPMI_PKT *pkt;
	IPMI_WS *req_ws;	
	uchar seq;
	uchar responder_slave_addr;
	GET_LED_COLOR_CAPABILITIES_CMD_REQ *req;
	IPMI_IPMB_REQUEST *ipmb_req;

	if( !( req_ws = ws_alloc() ) ) {
		return;
	}
	
	pkt = &( req_ws->pkt );
	ipmb_req = ( IPMI_IPMB_REQUEST * )&( req_ws->pkt_out );
	pkt->req = ( IPMI_CMD_REQ * )&( ipmb_req->command ) ;
	req = ( GET_LED_COLOR_CAPABILITIES_CMD_REQ * )pkt->req;	
	ipmi_get_next_seq( &seq );

	pkt->hdr.ws = (char *)req_ws;
	pkt->hdr.req_data_len = sizeof( GET_LED_COLOR_CAPABILITIES_CMD_REQ ) - 1;
	
	req->command = ATCA_CMD_GET_LED_COLOR;
	req->picmg_id = 0;
	req->fru_dev_id = 0; 
	req->led_id = led_id;
		
	ipmb_req->requester_slave_addr = module_get_i2c_address( I2C_ADDRESS_LOCAL );
	ipmb_req->netfn = NETFN_PICMG_REQ;
	ipmb_req->requester_lun = 0;
	responder_slave_addr = dev_addr;
	ipmb_req->header_checksum = -( *( char * )ipmb_req + responder_slave_addr );
	ipmb_req->req_seq = seq;
	ipmb_req->responder_lun = 0;
	ipmb_req->command = req->command;
	/* The location of data_checksum field is bogus.
	 * It's used as a placeholder to indicate that a checksum follows the data field.
	 * The location of the data_checksum depends on the size of the data preceeding it.*/
	ipmb_req->data_checksum = 
		ipmi_calculate_checksum( &ipmb_req->requester_slave_addr, 
			pkt->hdr.req_data_len + 3 ); 
	req_ws->len_out = sizeof( IPMI_IPMB_REQUEST ) 
				- IPMB_REQ_MAX_DATA_LEN  +  pkt->hdr.req_data_len;
	/* Assign the checksum to it's proper location */
	*( ( uchar * )ipmb_req + req_ws->len_out - 1 ) = ipmb_req->data_checksum; 

	req_ws->ipmi_completion_function = completion_function;
	req_ws->addr_out = dev_addr;
	req_ws->outgoing_protocol = IPMI_CH_PROTOCOL_IPMB;
	req_ws->outgoing_medium = IPMI_CH_MEDIUM_IPMB;
	req_ws->outgoing_channel = ipmi_ch;
	dump_outgoing( req_ws );

	/* dispatch the request */
	ws_set_state( req_ws, WS_ACTIVE_MASTER_WRITE );
}


void
send_get_fru_led_state( 
		uchar ipmi_ch, 
		uchar dev_addr, 
		void( *completion_function )( void *, int ) )
{
	IPMI_PKT *pkt;
	IPMI_WS *req_ws;	
	uchar seq;
	uchar responder_slave_addr;
	GET_FRU_LED_STATE_CMD_REQ *req;
	IPMI_IPMB_REQUEST *ipmb_req;

	if( !( req_ws = ws_alloc() ) ) {
		return;
	}
	
	pkt = &( req_ws->pkt );
	ipmb_req = ( IPMI_IPMB_REQUEST * )&( req_ws->pkt_out );
	pkt->req = ( IPMI_CMD_REQ * )&( ipmb_req->command ) ;
	req = ( GET_FRU_LED_STATE_CMD_REQ * )pkt->req;	
	ipmi_get_next_seq( &seq );

	pkt->hdr.ws = (char *)req_ws;
	pkt->hdr.req_data_len = sizeof( GET_FRU_LED_STATE_CMD_REQ ) - 1;
	
	req->command = ATCA_CMD_GET_FRU_LED_STATE;
	req->picmg_id = 0;
	req->fru_dev_id = 0;		/* FRU Device ID. */
	req->led_id = 0;		/* LED ID - FFh Reserved */

		
	ipmb_req->requester_slave_addr = module_get_i2c_address( I2C_ADDRESS_LOCAL );
	ipmb_req->netfn = NETFN_PICMG_REQ;
	ipmb_req->requester_lun = 0;
	responder_slave_addr = dev_addr;
	ipmb_req->header_checksum = -( *( char * )ipmb_req + responder_slave_addr );
	ipmb_req->req_seq = seq;
	ipmb_req->responder_lun = 0;
	ipmb_req->command = req->command;
	/* The location of data_checksum field is bogus.
	 * It's used as a placeholder to indicate that a checksum follows the data field.
	 * The location of the data_checksum depends on the size of the data preceeding it.*/
	ipmb_req->data_checksum = 
		ipmi_calculate_checksum( &ipmb_req->requester_slave_addr, 
			pkt->hdr.req_data_len + 3 ); 
	req_ws->len_out = sizeof( IPMI_IPMB_REQUEST ) 
				- IPMB_REQ_MAX_DATA_LEN  +  pkt->hdr.req_data_len;
	/* Assign the checksum to it's proper location */
	*( ( uchar * )ipmb_req + req_ws->len_out - 1 ) = ipmb_req->data_checksum; 

	req_ws->ipmi_completion_function = completion_function;
	req_ws->addr_out = dev_addr;
	req_ws->outgoing_protocol = IPMI_CH_PROTOCOL_IPMB;
	req_ws->outgoing_medium = IPMI_CH_MEDIUM_IPMB;
	req_ws->outgoing_channel = ipmi_ch;
	dump_outgoing( req_ws );

	/* dispatch the request */
	ws_set_state( req_ws, WS_ACTIVE_MASTER_WRITE );
}


void
send_get_device_locator_record_id( 
		uchar ipmi_ch, 
		uchar dev_addr, 
		void( *completion_function )( void *, int ) )
{
	IPMI_PKT *pkt;
	IPMI_WS *req_ws;	
	uchar seq;
	uchar responder_slave_addr;
	GET_DEVICE_LOCATOR_RECORD_ID_CMD_REQ *req;
	IPMI_IPMB_REQUEST *ipmb_req;

	if( !( req_ws = ws_alloc() ) ) {
		return;
	}
	
	pkt = &( req_ws->pkt );
	ipmb_req = ( IPMI_IPMB_REQUEST * )&( req_ws->pkt_out );
	pkt->req = ( IPMI_CMD_REQ * )&( ipmb_req->command ) ;
	req = ( GET_DEVICE_LOCATOR_RECORD_ID_CMD_REQ * )pkt->req;	
	ipmi_get_next_seq( &seq );

	pkt->hdr.ws = (char *)req_ws;
	pkt->hdr.req_data_len = sizeof( GET_DEVICE_LOCATOR_RECORD_ID_CMD_REQ ) - 1;
	
	req->command = ATCA_CMD_GET_DEVICE_LOCATOR_REC_ID;
	req->picmg_id = 0;
	req->fru_dev_id = 0;		/* FRU Device ID. */

	ipmb_req->requester_slave_addr = module_get_i2c_address( I2C_ADDRESS_LOCAL );
	ipmb_req->netfn = NETFN_PICMG_REQ;
	ipmb_req->requester_lun = 0;
	responder_slave_addr = dev_addr;
	ipmb_req->header_checksum = -( *( char * )ipmb_req + responder_slave_addr );
	ipmb_req->req_seq = seq;
	ipmb_req->responder_lun = 0;
	ipmb_req->command = req->command;
	/* The location of data_checksum field is bogus.
	 * It's used as a placeholder to indicate that a checksum follows the data field.
	 * The location of the data_checksum depends on the size of the data preceeding it.*/
	ipmb_req->data_checksum = 
		ipmi_calculate_checksum( &ipmb_req->requester_slave_addr, 
			pkt->hdr.req_data_len + 3 ); 
	req_ws->len_out = sizeof( IPMI_IPMB_REQUEST ) 
				- IPMB_REQ_MAX_DATA_LEN  +  pkt->hdr.req_data_len;
	/* Assign the checksum to it's proper location */
	*( ( uchar * )ipmb_req + req_ws->len_out - 1 ) = ipmb_req->data_checksum; 

	req_ws->ipmi_completion_function = completion_function;
	req_ws->addr_out = dev_addr;
	req_ws->outgoing_protocol = IPMI_CH_PROTOCOL_IPMB;
	req_ws->outgoing_medium = IPMI_CH_MEDIUM_IPMB;
	req_ws->outgoing_channel = ipmi_ch;
	dump_outgoing( req_ws );

	/* dispatch the request */
	ws_set_state( req_ws, WS_ACTIVE_MASTER_WRITE );
}


void
send_get_amc_port_state( 
		uchar ipmi_ch, 
		uchar dev_addr, 
		void( *completion_function )( void *, int ) )
{
	IPMI_PKT *pkt;
	IPMI_WS *req_ws;	
	uchar seq;
	uchar responder_slave_addr;
	GET_AMC_PORT_STATE_CMD_REQ *req; 
	IPMI_IPMB_REQUEST *ipmb_req;
	
	if( !( req_ws = ws_alloc() ) ) {
		return;
	}
	
	pkt = &( req_ws->pkt );
	ipmb_req = ( IPMI_IPMB_REQUEST * )&( req_ws->pkt_out );
	pkt->req = ( IPMI_CMD_REQ * )&( ipmb_req->command ) ;
	req = ( GET_AMC_PORT_STATE_CMD_REQ * )pkt->req;	
	ipmi_get_next_seq( &seq );

	pkt->hdr.ws = (char *)req_ws;
	pkt->hdr.req_data_len = sizeof( GET_AMC_PORT_STATE_CMD_REQ ) - 1;
	
	req->command = ATCA_CMD_GET_AMC_PORT_STATE;
	req->picmg_id = 0;
	req->amc_channel_id = 0;
	req->on_carrier_device_id = 0; 	/* Identifies the on-Carrier device to which the
					   described AMC Channel is connected.
					  TODO what do we set this to ? */

	ipmb_req->requester_slave_addr = module_get_i2c_address( I2C_ADDRESS_LOCAL );
	ipmb_req->netfn = NETFN_PICMG_REQ;
	ipmb_req->requester_lun = 0;
	responder_slave_addr = dev_addr;
	ipmb_req->header_checksum = -( *( char * )ipmb_req + responder_slave_addr );
	ipmb_req->req_seq = seq;
	ipmb_req->responder_lun = 0;
	ipmb_req->command = req->command;
	/* The location of data_checksum field is bogus.
	 * It's used as a placeholder to indicate that a checksum follows the data field.
	 * The location of the data_checksum depends on the size of the data preceeding it.*/
	ipmb_req->data_checksum = 
		ipmi_calculate_checksum( &ipmb_req->requester_slave_addr, 
			pkt->hdr.req_data_len + 3 ); 
	req_ws->len_out = sizeof( IPMI_IPMB_REQUEST ) 
				- IPMB_REQ_MAX_DATA_LEN  +  pkt->hdr.req_data_len;
	/* Assign the checksum to it's proper location */
	*( ( uchar * )ipmb_req + req_ws->len_out - 1 ) = ipmb_req->data_checksum; 

	req_ws->ipmi_completion_function = completion_function;
	req_ws->addr_out = dev_addr;
	req_ws->outgoing_protocol = IPMI_CH_PROTOCOL_IPMB;
	req_ws->outgoing_medium = IPMI_CH_MEDIUM_IPMB;
	req_ws->outgoing_channel = ipmi_ch;
	dump_outgoing( req_ws );

	/* dispatch the request */
	ws_set_state( req_ws, WS_ACTIVE_MASTER_WRITE );
}


void
send_set_amc_port_state( 
		uchar ipmi_ch, 
		uchar dev_addr, 
		void( *completion_function )( void *, int ) )
{
	IPMI_PKT *pkt;
	IPMI_WS *req_ws;	
	uchar seq;
	uchar responder_slave_addr;
	SET_AMC_PORT_STATE_CMD_REQ *req; 
	IPMI_IPMB_REQUEST *ipmb_req;

	if( !( req_ws = ws_alloc() ) ) {
		return;
	}

	pkt = &( req_ws->pkt );
	ipmb_req = ( IPMI_IPMB_REQUEST * )&( req_ws->pkt_out );
	pkt->req = ( IPMI_CMD_REQ * )&( ipmb_req->command ) ;
	req = ( SET_AMC_PORT_STATE_CMD_REQ * )pkt->req;	
	ipmi_get_next_seq( &seq );

	pkt->hdr.ws = (char *)req_ws;
	pkt->hdr.req_data_len = sizeof( SET_AMC_PORT_STATE_CMD_REQ ) - 1;
	
	req->command = ATCA_CMD_SET_AMC_PORT_STATE;
	req->picmg_id = 0;
	req->link_grp_id = 0;		/* Link Grouping ID */
	req->link_type_ext = 0;		/* Link Type Extension */
	req->link_type = 0;		/* Link Type */
	req->lane_3_bit_flag = 0;	/* Lane 3 Bit Flag */
	req->lane_2_bit_flag = 0;	/* Lane 2 Bit Flag */
	req->lane_1_bit_flag = 0;	/* Lane 1 Bit Flag */
	req->lane_0_bit_flag = 0;	/* Lane 0 Bit Flag */
	req->amc_channel_id = 0;	/* AMC Channel ID */
	req->state = 0;			/* 00h = Disable, 01h = Enable */
	req->on_carrier_dev_id = 0;	/* On-Carrier device ID. 
					   Identifies the on-Carrier device
					   to which the described AMC Channel
					   is connected. */
	
	ipmb_req->requester_slave_addr = module_get_i2c_address( I2C_ADDRESS_LOCAL );
	ipmb_req->netfn = NETFN_PICMG_REQ;
	ipmb_req->requester_lun = 0;
	responder_slave_addr = dev_addr;
	ipmb_req->header_checksum = -( *( char * )ipmb_req + responder_slave_addr );
	ipmb_req->req_seq = seq;
	ipmb_req->responder_lun = 0;
	ipmb_req->command = req->command;
	/* The location of data_checksum field is bogus.
	 * It's used as a placeholder to indicate that a checksum follows the data field.
	 * The location of the data_checksum depends on the size of the data preceeding it.*/
	ipmb_req->data_checksum = 
		ipmi_calculate_checksum( &ipmb_req->requester_slave_addr, 
			pkt->hdr.req_data_len + 3 ); 
	req_ws->len_out = sizeof( IPMI_IPMB_REQUEST ) 
				- IPMB_REQ_MAX_DATA_LEN  +  pkt->hdr.req_data_len;
	/* Assign the checksum to it's proper location */
	*( ( uchar * )ipmb_req + req_ws->len_out - 1 ) = ipmb_req->data_checksum; 

	req_ws->ipmi_completion_function = completion_function;
	req_ws->addr_out = dev_addr;
	req_ws->outgoing_protocol = IPMI_CH_PROTOCOL_IPMB;
	req_ws->outgoing_medium = IPMI_CH_MEDIUM_IPMB;
	req_ws->outgoing_channel = ipmi_ch;
	dump_outgoing( req_ws );

	/* dispatch the request */
	ws_set_state( req_ws, WS_ACTIVE_MASTER_WRITE );
}


void
send_set_amc_clock_state( 
		uchar ipmi_ch, 
		uchar dev_addr, 
		void( *completion_function )( void *, int ) )
{
	IPMI_PKT *pkt;
	IPMI_WS *req_ws;	
	uchar seq;
	uchar responder_slave_addr;
	SET_CLOCK_STATE_CMD_REQ	*req;
	IPMI_IPMB_REQUEST *ipmb_req;
	
	if( !( req_ws = ws_alloc() ) ) {
		return;
	}
	
	pkt = &( req_ws->pkt );
	ipmb_req = ( IPMI_IPMB_REQUEST * )&( req_ws->pkt_out );
	pkt->req = ( IPMI_CMD_REQ * )&( ipmb_req->command ) ;
	req = ( SET_CLOCK_STATE_CMD_REQ * )pkt->req;	
	ipmi_get_next_seq( &seq );

	pkt->hdr.ws = (char *)req_ws;
	pkt->hdr.req_data_len = sizeof( SET_CLOCK_STATE_CMD_REQ ) - 1;
	
	req->command = ATCA_CMD_SET_CLOCK_STATE;
	req->picmg_id = 0;
	req->clock_id = 0;
	req->clock_config_index = 0;
	req->clock_state = 0;
	req->clock_direction = 0;
	req->pll_control = 0;
	req->clock_family = 0;
	req->clock_accuracy_level = 0;
	req->clock_freq = 0;
	req->clock_resource_id = 0;

	ipmb_req->requester_slave_addr = module_get_i2c_address( I2C_ADDRESS_LOCAL );
	ipmb_req->netfn = NETFN_PICMG_REQ;
	ipmb_req->requester_lun = 0;
	responder_slave_addr = dev_addr;
	ipmb_req->header_checksum = -( *( char * )ipmb_req + responder_slave_addr );
	ipmb_req->req_seq = seq;
	ipmb_req->responder_lun = 0;
	ipmb_req->command = req->command;
	/* The location of data_checksum field is bogus.
	 * It's used as a placeholder to indicate that a checksum follows the data field.
	 * The location of the data_checksum depends on the size of the data preceeding it.*/
	ipmb_req->data_checksum = 
		ipmi_calculate_checksum( &ipmb_req->requester_slave_addr, 
			pkt->hdr.req_data_len + 3 ); 
	req_ws->len_out = sizeof( IPMI_IPMB_REQUEST ) 
				- IPMB_REQ_MAX_DATA_LEN  +  pkt->hdr.req_data_len;
	/* Assign the checksum to it's proper location */
	*( ( uchar * )ipmb_req + req_ws->len_out - 1 ) = ipmb_req->data_checksum; 

	req_ws->ipmi_completion_function = completion_function;
	req_ws->addr_out = dev_addr;
	req_ws->outgoing_protocol = IPMI_CH_PROTOCOL_IPMB;
	req_ws->outgoing_medium = IPMI_CH_MEDIUM_IPMB;
	req_ws->outgoing_channel = ipmi_ch;
	dump_outgoing( req_ws );

	/* dispatch the request */
	ws_set_state( req_ws, WS_ACTIVE_MASTER_WRITE );
}

void
send_get_amc_clock_state( 
		uchar ipmi_ch, 
		uchar dev_addr, 
		void( *completion_function )( void *, int ) )
{
	IPMI_PKT *pkt;
	IPMI_WS *req_ws;	
	uchar seq;
	uchar responder_slave_addr;
	GET_CLOCK_STATE_CMD_REQ	*req;
	IPMI_IPMB_REQUEST *ipmb_req;

	if( !( req_ws = ws_alloc() ) ) {
		return;
	}
	
	pkt = &( req_ws->pkt );
	ipmb_req = ( IPMI_IPMB_REQUEST * )&( req_ws->pkt_out );
	pkt->req = ( IPMI_CMD_REQ * )&( ipmb_req->command ) ;
	req = ( GET_CLOCK_STATE_CMD_REQ * )pkt->req;	
	ipmi_get_next_seq( &seq );

	pkt->hdr.ws = (char *)req_ws;
	pkt->hdr.req_data_len = sizeof( GET_CLOCK_STATE_CMD_REQ ) - 1;
	
	req->command = ATCA_CMD_GET_CLOCK_STATE;
	req->picmg_id = 0;
	req->clock_id = 0;
	req->clock_resource_id = 0;

	ipmb_req->requester_slave_addr = module_get_i2c_address( I2C_ADDRESS_LOCAL );
	ipmb_req->netfn = NETFN_PICMG_REQ;
	ipmb_req->requester_lun = 0;
	responder_slave_addr = dev_addr;
	ipmb_req->header_checksum = -( *( char * )ipmb_req + responder_slave_addr );
	ipmb_req->req_seq = seq;
	ipmb_req->responder_lun = 0;
	ipmb_req->command = req->command;
	/* The location of data_checksum field is bogus.
	 * It's used as a placeholder to indicate that a checksum follows the data field.
	 * The location of the data_checksum depends on the size of the data preceeding it.*/
	ipmb_req->data_checksum = 
		ipmi_calculate_checksum( &ipmb_req->requester_slave_addr, 
			pkt->hdr.req_data_len + 3 ); 
	req_ws->len_out = sizeof( IPMI_IPMB_REQUEST ) 
				- IPMB_REQ_MAX_DATA_LEN  +  pkt->hdr.req_data_len;
	/* Assign the checksum to it's proper location */
	*( ( uchar * )ipmb_req + req_ws->len_out - 1 ) = ipmb_req->data_checksum; 

	req_ws->ipmi_completion_function = completion_function;
	req_ws->addr_out = dev_addr;
	req_ws->outgoing_protocol = IPMI_CH_PROTOCOL_IPMB;
	req_ws->outgoing_medium = IPMI_CH_MEDIUM_IPMB;
	req_ws->outgoing_channel = ipmi_ch;
	dump_outgoing( req_ws );

	/* dispatch the request */
	ws_set_state( req_ws, WS_ACTIVE_MASTER_WRITE );
}

void
send_get_fru_control_capabilities( 
		uchar ipmi_ch, 
		uchar dev_addr, 
		void( *completion_function )( void *, int ) ) 
{
	IPMI_PKT *pkt;
	IPMI_WS *req_ws;	
	uchar seq;
	uchar responder_slave_addr;
	FRU_CONTROL_CAPABILITIES_CMD_REQ *req;
	IPMI_IPMB_REQUEST *ipmb_req;

	if( !( req_ws = ws_alloc() ) ) {
		return;
	}
	
	pkt = &( req_ws->pkt );
	ipmb_req = ( IPMI_IPMB_REQUEST * )&( req_ws->pkt_out );
	pkt->req = ( IPMI_CMD_REQ * )&( ipmb_req->command ) ;
	req = ( FRU_CONTROL_CAPABILITIES_CMD_REQ * )pkt->req;	
	ipmi_get_next_seq( &seq );

	pkt->hdr.ws = (char *)req_ws;
	pkt->hdr.req_data_len = sizeof( FRU_CONTROL_CAPABILITIES_CMD_REQ ) - 1;
	
	req->command = ATCA_CMD_FRU_CONTROL_CAPABILITIES;
	req->picmg_id = 0;
	req->fru_dev_id = 0;		/* FRU Device ID. */

	ipmb_req->requester_slave_addr = module_get_i2c_address( I2C_ADDRESS_LOCAL );
	ipmb_req->netfn = NETFN_PICMG_REQ;
	ipmb_req->requester_lun = 0;
	responder_slave_addr = dev_addr;
	ipmb_req->header_checksum = -( *( char * )ipmb_req + responder_slave_addr );
	ipmb_req->req_seq = seq;
	ipmb_req->responder_lun = 0;
	ipmb_req->command = req->command;
	/* The location of data_checksum field is bogus.
	 * It's used as a placeholder to indicate that a checksum follows the data field.
	 * The location of the data_checksum depends on the size of the data preceeding it.*/
	ipmb_req->data_checksum = 
		ipmi_calculate_checksum( &ipmb_req->requester_slave_addr, 
			pkt->hdr.req_data_len + 3 ); 
	req_ws->len_out = sizeof( IPMI_IPMB_REQUEST ) 
				- IPMB_REQ_MAX_DATA_LEN  +  pkt->hdr.req_data_len;
	/* Assign the checksum to it's proper location */
	*( ( uchar * )ipmb_req + req_ws->len_out - 1 ) = ipmb_req->data_checksum; 

	req_ws->ipmi_completion_function = completion_function;
	req_ws->addr_out = dev_addr;
	req_ws->outgoing_protocol = IPMI_CH_PROTOCOL_IPMB;
	req_ws->outgoing_medium = IPMI_CH_MEDIUM_IPMB;
	req_ws->outgoing_channel = ipmi_ch;
	dump_outgoing( req_ws );

	/* dispatch the request */
	ws_set_state( req_ws, WS_ACTIVE_MASTER_WRITE );
}


/*
 * cmd_complete()
 * 
 * Completion function 
 */
void
cmd_complete( IPMI_WS *ws, int status )
{
	switch( status ) {
		case XPORT_REQ_NOERR:
		default:
			ws_free( ws );
			break;
	}
}

/*==============================================================
 * FRU CONTROL
 *==============================================================*/
/*
The “FRU Control” command provides base level control over the Modules 
to the Carrier IPMC. Through this command, the Modules can be reset, 
rebooted, instructed to quiesce, or have its diagnostics initiated. The
implementation of these commands will vary, and allcommand variants with 
the exception of the “FRU Control (Cold Reset)” and “FRU Control
(Quiesce)” are optional. The “FRU Control” command does not directly change the
operational state of the Module as represented by the Carrier IPMC (which is typically M4 or
FRU Active).

Table 3-46 provides specifics for the FRU Control command.

Requirements
REQ 3.194 An MMC shall respond to the “FRU Control Capabilities” command 
defined in Table 3-24 of the PICMG3.0 specification by identifying the 
optional capabilities of the “FRU Control” command that the Module supports.

REQ 3.100 The “FRU Control” command should not directly change Modules’ 
FRU states.

REQ 3.101 Receipt of a “FRU Control (Cold Reset)” command shall
cause a hardware reset to its Payload, similar to a power on reset.

REQ 3.102 Receipt of a “FRU Control (Warm Reset)” command on a Module 
which supports this command shall cause the Module’s Payload to be reset
to a stable condition, attempting to preserve its operational state. If
this command variant is unsupported, the MMC shall return the “Invalid 
data field in Request (CCh)” Completion Code.

REQ 3.103 Receipt of a “FRU Control (Graceful Reboot)” command on a Module 
which supports this command shall initiate a graceful shutdown and reboot 
of its Payload operating system. If this command variant is unsupported, 
the MMC shall return the “Invalid data field in Request (CCh)” Completion Code.

REQ 3.104 Receipt of a “FRU Control (Issue Diagnostic Interrupt)” command
on a Module which supports this command shall trigger a diagnostic interrupt
to the Module’s Payload. If this command variant is unsupported, the MMC 
shall return the “Invalid data field in Request (CCh)” Completion Code.

REQ 3.105b On receipt of the “FRU Control (Quiesce)” command, the MMC shall
take appropriate action (implementation specific) to bring the Payload to
a quiesced state and shall send a Module Hot Swap (Quiesced) event messag
e to the Carrier IPMC.
*/
/* 
 * The following are called by picmg_fru_control()
 */

void
send_amc_cold_reset( uchar dev_id )
{
}

void
send_amc_warm_reset( uchar dev_id )
{
}

void
send_amc_graceful_reboot( uchar dev_id )
{
}

void
send_amc_issue_diag_int( uchar dev_id )
{
}

/* 
 * module_quiesce()
 * 
 * called by picmg_fru_control()
 * When the Carrier IPMC has transitioned the Module to M6 state, the Carrier 
 * IPMC sends a “FRU Control (Quiesce)” command to the Module and awaits a 
 * Module Hot Swap (Quiesced) event message from the MMC.
 */

void
send_amc_quiesce( uchar dev_id )
{
	
}

void
enable_payload( uchar dev_id )
{

}




/*==============================================================
 * MODULE SENSORS
 *==============================================================*/

/*
== HOT SWAP SENSOR ==
3.6.6 Module Hot Swap sensor
 46 Each MMC contains one Module Hot Swap sensor. This sensor proactively generate events
(Module Handle Closed, Module Handle Opened, Quiesced, Backend Power Shut Down,
and Backend Power Failure) to enable the Carrier IPMC to perform Hot Swap management
for the Modules it represents.

A Module’s Backend Power includes all the power supplies on a Module derived from
Payload Power. Module Backend Power could be disabled by a Module as an
implementation defined option. How a Module Payload communicates with the MMC to
indicate it has Quiesced, has requested that its Module Backend Power be shut down, or to
indicate a Module Backend Power Failure is implementation defined.

REQ 3.52 A Module Hot Swap sensor shall be implemented in the MMC.

REQ 3.158 MMCs on Modules implementing local on/ off switching control of 
power derived from Payload Power shall set the Backend Power Shut Down bit
in the Module Hot Swap sensor Current State Mask field to 1b when the MMC
has turned off its Module Backend Power. MMCs on Modules implementing local
on/ off switching control of power derived from Payload Power shall clear 
the Backend Power Shut Down bit to 0b when Payload Power from the Carrier 
transitions from disabled to enabled.

REQ 3.159 MMCs on Modules implementing local on/ off switching control 
of power derived from Payload Power shall set the Backend Power Failure
bit in the Module Hot Swap sensor Current State Mask field to 1b when
the Module Backend Power fails. MMCs on Modules implementing local on/ 
off switching control of power derived from Payload Power shall clear 
the Backend Power Failure bit to 0b when Payload Power from the Carrier
transitions from disabled to enabled.

REQ 3.160 MMCs shall set the Quiesced bit in the Module Hot Swap sensor 
Current State Mask field to 1b after the Module Payload has quiesced. 
The MMC shall clear the Quiesced bit to 0b upon reception of the “FRU
Control (Quiesce)” request just prior to  the Module Payload being quiesced. 
MMCs with Payload Power monitoring capability should clear the Quiesced 
bit to 0b when Payload Power from the Carrier transitions from disabled
to enabled.

== TEMPERATURE SENSORS ==
3.8 Cooling management
To support higher level management in appropriately managing the cooling resources, the
Module must provide reports of abnormal temperature in its environment. Every Module
must have temperature sensors; when an MMC detects that a monitored temperature sensor
crosses one or more thresholds in either direction, the MMC sends an IPMI temperature
event message to the Carrier IPMC. The Carrier IPMC, or higher level management, uses
this information to manage the cooling.

Every Carrier and every Module must contain at least two temperature sensors and
appropriate Sensor Data Records (SDRs) to describe the sensors. See Section 5.3,
“Temperature” for functional requirements regarding these sensors.

Requirements
REQ 3.164 The Module and the Carrier shall implement the SDRs for the 
temperature sensors required by Section 5, “Thermal.” 

REQ 3.80 The Module shall generate temperature sensor events in accordance 
with Section 3.9.3.2 “Abnormal Event Message” of the PICMG 3.0 specification.

REQ 3.165For each on-Module temperature sensor, Modules shall provide the 
warning operating temperature (upper non critical threshold) and the maximum
operating temperature (Upper critical threshold) in the SDR information.



3.11.1 Module SDR requirements

REQ 3.106 The MMC shall support IPMI commands “Get Device SDR Info”, “Get Device SDR” 
and “Reserve Device SDR Repository”. (See Chapter 29 of the IPMI specification.)

REQ 3.107 The MMC Device SDR shall have a static sensor population. (See 
Table 29-2 of the IPMI specification.) The number of MMC SDRs is fixed. 
All MMC sensors shall be assigned to LUN 0.

REQ 3.108 The MMC shall use AdvancedMC Module entity ID C1h for all device SDRs.

REQ 3.109 The MMC shall use its own Site Number + 60h as a device-relative
entity instance number. 

REQ 3.110Each MMC shall have one management controller device locator (SDR type 12h).

REQ 3.111 The MMC shall not have FRU locator records (SDR type 11h) as only
one FRU may be represented by one MMC.

REQ 3.112 The MMC shall not expect any Init Agent action from the Carrier IPMC.

REQ 3.113 The MMC shall initialize its sensors, set the event receiver address 
to 20h and event receiver LUN to 0 on reset.

3.12 FRU Information

All Carriers and Modules must contain a FRU Information storage device 
(for instance, an SEEPROM) that contains information about capabilities 
(e.g. E-Keying) and inventory data. The format of the FRU Information follows
the requirements set forth in Section 3.6.3, IPM Controller FRU Information,
in the PICMG 3.0 Specification. In addition to this basic information, 
additional records are required to support functions unique to the Modules.

3.12.1.2 MMC requirements

REQ 3.138 The MMC shall support the “FRU Inventory Device” commands specified
in Chapter 28 of the IPMI specification.

REQ 3.139Module FRU Information shall be available when MP is available.

REQ 3.140The entity updating the Module FRU Information shall be responsible 
for updating all appropriate checksums as well.

REQ 3.141 The MMC may write protect data in the Module storage area and 
return a Completion Code of “write protected offset (80h)” for a command
that attempts to change such data.
	
REQ 3.142If an MMC implements write protected data, it shall do it by areas
(Board Info, Product Info, etc.) with the exception of the multi-record area
where it shall implement write protection at the record level.

REQ 3.143 If an MMC implements write protected multi-record data, it shall
allow moving a record to a new offset (without any change to the data). That
is, the size or number of records prior to a particular record may change 
and it may be necessary for system management software to move a record 
without changing its contents. The MMC shall allow this, but may confirm
that the data has not been changed.

REQ 3.144 MMC FRU Information shall meet the requirements in Chapter 3.6.3
of the IPMI specification.REQ 3.145The MMC shall implement the relevant 
records defined in Section 3.7, “Power management” and Section 3.9, “E-Keying.” 

REQ 3.146 All Modules that are sold as independent products shall fill
in all predefined fields in Product Info Area record, as this is the 
only location where product version number is defined.

REQ 3.147Every MMC shall place the MultiRecords defined by this specification
in its MultiRecord Info Area.

REQ 3.148Any MMC may place private data in the Internal Use Area and/or MultiRecord Info Area.
*/

/*==============================================================
 * MODULE RECORDS 
 *==============================================================*/
/*
3.7.1 Module Current Requirements record
Each Module defines its maximum current requirement even if that 
value is required for only a transitional amount of time (for all components
on the Module). The Module FRU Information structure described below informs
the Carrier of these requirements. Table 3-10 Module Current Requirements record

The capabilities of an AdvancedMC Module to communicate over point-to-point
connections are described in the Module’s FRU Information.

*/

/* given the i2c addr lookup the slot number */
uchar
lookup_dev_id( uchar dev_addr )
{
	int i;

	for( i = 0; i < IPMBL_TABLE_SIZE ; i++ ) {
		if( IPMBL_TABLE[i] == dev_addr )
			return i;
	}
	return 0xff;
}

/* given the slot number, lookup the i2c addr */
uchar 
lookup_dev_addr( uchar dev_id )
{
	if( dev_id < IPMBL_TABLE_SIZE )	// assuming slot enumeration starts from 0, TODO check
		return( IPMBL_TABLE[dev_id] );
	else
		return 0xff;
}

// handle events from the AMC modules
/* 
 read the Module’s Module Current Requirements record and 
AdvancedMC Point-to-Point Connectivity record.

If the Module FRU Information is invalid or if the Carrier cannot provide the
necessary Payload Power then:

The Carrier IPMC sends a “Set FRU LED State” command to the MMC requesting the
“on” state for the BLUE LED. The FRU remains in M1.

On receipt of the “Set FRU Activation (Activate FRU)” command by the Carrier 
IPMC, designating a particular Module, the Carrier IPMC sends an M2 to M3 
transition event message to the Shelf Manager on behalf of the Module and sends
a “set FRU LED State” command to the MMC with a request to turn off the BLUE LED.

The Carrier IPMC enables Payload Power (PWR) for the Module.
*/

#define AMC_EVT_HANDLE_CLOSED_MSG_RCVD		0
#define AMC_EVT_SET_LED_STATE_CMD_OK		1
#define AMC_EVT_READ_CURRENT_REQ_CMD_OK		2
#define AMC_EVT_READ_P2P_RECORD_CMD_OK		3
#define AMC_EVT_ACTIVATION_REQ_MSG_OK		4
#define AMC_EVT_ACTIVATE_FRU_MSG_RCVD		5
#define AMC_EVT_PAYLOAD_ENABLED			6
#define AMC_EVT_HANDLE_OPENED_MSG_RCVD		7
#define AMC_EVT_SET_PORT_STATE_DISABLE_OK	8
#define AMC_EVT_FRU_QUIESCE_CMD_OK		9
#define AMC_EVT_DEVICE_DISCOVERY_OK		10

#define AMC_STATE_M1				0
#define AMC_STATE_M2_LED_LONG_BLINK_SENT	1
#define AMC_STATE_M2_DEVICE_DISCOVERY_STARTED	2
#define AMC_STATE_M2_READ_P2P_RECORD_REQ_SENT	3
#define AMC_STATE_M2_SHM_ACT_REQ_SENT		4
#define AMC_STATE_M2_SHM_ACT_MSG_WAIT		5
#define AMC_STATE_M2_LED_OFF_SENT		6
#define AMC_STATE_M3				7
#define AMC_STATE_M3_PAYLOAD_ENABLE_SENT	8
#define AMC_STATE_M4				9
#define AMC_STATE_M4_LED_BLINK_SENT		10
#define AMC_STATE_M4_PORT_DISABLE_SENT		11
#define AMC_STATE_M4_FRU_QUIESCE_SENT		12

void
module_event_handler( IPMI_PKT *pkt )
{
	PLATFORM_EVENT_MESSAGE_CMD_REQ	*req = ( PLATFORM_EVENT_MESSAGE_CMD_REQ * )pkt->req;
	GENERIC_EVENT_MSG *evt_msg = ( GENERIC_EVENT_MSG * )&( req->EvMRev );

	uchar dev_id, dev_addr = (( IPMI_WS * )(pkt->hdr.ws))->incoming_channel;
	
	dev_id = lookup_dev_id( dev_addr );
	
	if( evt_msg->sensor_type == IPMI_SENSOR_HOT_SWAP ) {
		switch( evt_msg->evt_data1 ) {
			case MODULE_HANDLE_CLOSED:
				mcmc_mmc_event( dev_id, AMC_EVT_HANDLE_CLOSED_MSG_RCVD );
				break;

			case MODULE_HANDLE_OPENED:
				mcmc_mmc_event( dev_id, AMC_EVT_HANDLE_OPENED_MSG_RCVD );
				break;

			case MODULE_QUIESCED:
				mcmc_mmc_event( dev_id, AMC_EVT_FRU_QUIESCE_CMD_OK );
				break;

			case MODULE_BACKEND_POWER_FAILURE:
			case MODULE_BACKEND_POWER_SHUTDOWN:
			default:
				break;
		}
	} else if ( evt_msg->sensor_type == ST_HOT_SWAP ) {
		FRU_TEMPERATURE_EVENT_MSG_REQ *temp_req = ( FRU_TEMPERATURE_EVENT_MSG_REQ * )req;
		switch( temp_req->evt_reason ) {
			case UPPER_NON_CRITICAL_GOING_HIGH:
				break;
			case UPPER_CRITICAL_GOING_HIGH:
				break;
			case UPPER_NON_RECOVERABLE_GOING_HIGH:
				/* Assume system hardware in jeopardy or damaged */
				/* Turn off all power to AMC cards */
				break;
		}
	}
}


/* mcmc state machine */
void
mcmc_mmc_event( uchar dev_id, uchar event )
{
	uchar dev_addr;

	dev_addr = lookup_dev_addr( dev_id );
	
	switch( event ) {
		case	AMC_EVT_HANDLE_CLOSED_MSG_RCVD:
			/* Send set LED state long blink cmd */
			/* send a “Set FRU LED State” command to the MMC
			 * with a request to perform long blinks of the
			 * BLUE LED, indicating to the operator that the
			 * new Module is waiting to be activated. */
			send_set_fru_led_state( IPMI_CH_NUM_IPMBL, dev_addr, LED_LONG_BLINK, cmd_complete );
			amc[dev_id].state = AMC_STATE_M2_LED_LONG_BLINK_SENT;
			break;
		case	AMC_EVT_SET_LED_STATE_CMD_OK:
			if( amc[dev_id].state == AMC_STATE_M2_LED_LONG_BLINK_SENT ) {
				/* Start device discovery */
				amc[dev_id].state = AMC_STATE_M2_DEVICE_DISCOVERY_STARTED;
				device_discovery( dev_id );
			} else if ( amc[dev_id].state == AMC_STATE_M2_LED_OFF_SENT ) {
				amc[dev_id].state = AMC_STATE_M3;
				/* check payload requirements */
				/* enable payload */
				enable_payload( dev_id );
				amc[dev_id].state = AMC_STATE_M3_PAYLOAD_ENABLE_SENT;
			} else if ( amc[dev_id].state == AMC_STATE_M4_LED_BLINK_SENT ) {
				/* send port state disable cmd */
				amc[dev_id].state = AMC_STATE_M4_PORT_DISABLE_SENT;
			}
			break;
		case	AMC_EVT_DEVICE_DISCOVERY_OK:
			// TODO check the fru power record, enable power if within limits
			if( amc[dev_id].state == AMC_STATE_M2_DEVICE_DISCOVERY_STARTED ) {
				send_set_fru_led_state( IPMI_CH_NUM_IPMBL, dev_addr, LED_OFF, cmd_complete );
				amc[dev_id].state = AMC_STATE_M2_LED_OFF_SENT;
			}
			break;
		case	AMC_EVT_READ_CURRENT_REQ_CMD_OK:
			/* Send read p2p connectivity record cmd */
			amc[dev_id].state = AMC_STATE_M2_READ_P2P_RECORD_REQ_SENT;
			break;			
		case	AMC_EVT_READ_P2P_RECORD_CMD_OK:
			/* Send FRU activation req msg to SHM */
			amc[dev_id].state = AMC_STATE_M2_SHM_ACT_REQ_SENT;
			break;
		case	AMC_EVT_ACTIVATION_REQ_MSG_OK:
			amc[dev_id].state = AMC_STATE_M2_SHM_ACT_MSG_WAIT;
			break;
		case	AMC_EVT_ACTIVATE_FRU_MSG_RCVD:
			/* Send set LED state off cmd */
			amc[dev_id].state = AMC_STATE_M2_LED_OFF_SENT;
			break;
		case	AMC_EVT_PAYLOAD_ENABLED:
			amc[dev_id].state = AMC_STATE_M4;
			break;
		case	AMC_EVT_HANDLE_OPENED_MSG_RCVD:
			// Send quiesce req
			send_fru_control( IPMI_CH_NUM_IPMBL, dev_addr, FRU_CONTROL_QUIESCE, cmd_complete );
			/* Send set LED state short blink cmd */
			send_set_fru_led_state( IPMI_CH_NUM_IPMBL, dev_addr, LED_SHORT_BLINK, cmd_complete );
			amc[dev_id].state = AMC_STATE_M4_LED_BLINK_SENT;
			break;
		case	AMC_EVT_SET_PORT_STATE_DISABLE_OK:
			/* Send FRU Quiesce cmd */
			amc[dev_id].state = AMC_STATE_M4_FRU_QUIESCE_SENT;
			break;
		case	AMC_EVT_FRU_QUIESCE_CMD_OK:
			amc[dev_id].state = AMC_STATE_M1;
			break;
		default:
			break;
	}
}

/*
 * AMC device discovery:
 *
 * For each device, send "get device id", keep returned values in local table.
 *
 * For each available device with SDR (indicated in the "get device id" response)
 * send a "get device sdr info" command.
 *
 * Send "get device sdr" using the "Total Number of SDRs in the device" value
 * returned from "get device sdr info" command.
 *
 * Send "get fru inventory area info" command.
 *
 * Send "read fru data" commands
*/

/* start_chassis_device_discovery()
 *
 * Initiate the discovery process for all devices currently plugged to the backplane
 */
void
start_chassis_device_discovery( void )
{
	uchar dev_id;

	poll_slots();

	for( dev_id = 0; dev_id < NUM_SLOTS; dev_id++ ) {
		if( slot_info[dev_id].amc_available ) {
			device_discovery( dev_id );
		}
	}
}

#define DISC_ST_GET_DEV_ID_SENT				0
#define DISC_ST_GET_DEV_ID_OK				1
#define DISC_ST_GET_DEV_SDR_INFO_SENT			2
#define DISC_ST_GET_DEV_SDR_INFO_OK			3
#define DISC_ST_GET_DEV_SDR_SENT			4
#define DISC_ST_GET_DEV_SDR_OK				5
#define DISC_ST_GET_DEV_SDR_COMPLETE			6
#define DISC_ST_GET_FRU_INVENTORY_AREA_INFO_SENT	7
#define DISC_ST_GET_FRU_INVENTORY_AREA_INFO_OK		8
#define DISC_ST_READ_FRU_DATA_SENT			9
#define DISC_ST_READ_FRU_DATA_OK			10
#define DISC_ST_READ_FRU_DATA_COMPLETE			11

#define FRU_READ_ST_READING_FRU_HDR			0
#define FRU_READ_ST_READING_INTERNAL_USE_AREA		1
#define FRU_READ_ST_READING_CHASSIS_INFO		2
#define FRU_READ_ST_READING_BOARD_AREA			3
#define FRU_READ_ST_READING_PRODUCT_INFO		4
#define FRU_READ_ST_READING_MULTIRECORD_AREA		5

void
device_discovery( uchar dev_id )
{
	uchar dev_addr;
	uchar ipmi_ch = IPMI_CH_NUM_IPMBL;

	dev_addr = lookup_dev_addr( dev_id );

	if( dev_addr != 0xff ) {
		discovery_state[dev_id] = DISC_ST_GET_DEV_ID_SENT;
		send_get_device_id( ipmi_ch, dev_addr, discovery_cmd_complete );
	}
}


void
discovery_cmd_retry( uchar *arg )
{
	IPMI_WS *ws = ( IPMI_WS * )arg;

	ws_set_state( ws, WS_ACTIVE_MASTER_WRITE );
}


#define FRU_READ_LEN	16
/*
 * discovery_cmd_complete()
 * 
 * Completion function for discovery commands 
 */
void
discovery_cmd_complete( IPMI_WS *ws, int status )
{
	IPMI_PKT *pkt = &ws->pkt;
	IPMI_IPMB_REQUEST *ipmb_req = ( IPMI_IPMB_REQUEST * )&( ws->pkt_in );
	uchar dev_id, rec_id = 0, dev_addr;
	uchar ipmi_ch = IPMI_CH_NUM_IPMBL;

	dev_addr = ipmb_req->requester_slave_addr;
	dev_id = lookup_dev_id( dev_addr );

	if( status != XPORT_REQ_NOERR ) {
		// add to timeout queue to retry in a few secs
		timer_add_callout_queue( (void *)&discovery_cmd_retry_timer_handle,
	       		10*HZ, discovery_cmd_retry,( uchar * )ws ); /* 10 sec timeout */
		
		// TODO we should prevent other commands being issued to this device
		// or cancel the discovery process at some point if we want to do something
		// else with the device.

		return;
	}

	switch( ( pkt->hdr.netfn << 8 ) | pkt->req->command ) {
		case APP_CMD_GET_DEVICE_ID:
			// copy data to amc.device_id
			memcpy( &amc[dev_id].device_id, pkt->resp, sizeof( GET_DEVICE_ID_CMD_RESP ) );
			discovery_state[dev_id] = DISC_ST_GET_DEV_SDR_INFO_SENT;
			send_get_device_sdr( dev_addr, rec_id, 0, discovery_cmd_complete );
			break;
		case EVENT_CMD_GET_DEVICE_SDR_INFO:
			memcpy( &amc[dev_id].sdr_info, pkt->resp, sizeof( GET_DEVICE_SDR_INFO_RESP ) );
			amc[dev_id].sdr_to_process = amc[dev_id].sdr_info.num;
			if( amc[dev_id].sdr_to_process > 0 ) {
				discovery_state[dev_id] = DISC_ST_GET_DEV_SDR_SENT;
				amc[dev_id].sdr_to_process--;
				rec_id = amc[dev_id].sdr_info.num - amc[dev_id].sdr_to_process;
				send_get_device_sdr( dev_addr, rec_id, 0, discovery_cmd_complete );
			}
			break;
		case EVENT_CMD_GET_DEVICE_SDR:
			// copy data
			rec_id = amc[dev_id].sdr_info.num - amc[dev_id].sdr_to_process;
			memcpy( &amc[dev_id].sdr_data[rec_id], pkt->resp, sizeof( GET_DEVICE_SDR_RESP ) );
			if( amc[dev_id].sdr_to_process > 0 ) {
				discovery_state[dev_id] = DISC_ST_GET_DEV_SDR_SENT;
				amc[dev_id].sdr_to_process--;
				rec_id = amc[dev_id].sdr_info.num - amc[dev_id].sdr_to_process;
				send_get_device_sdr( dev_addr, rec_id, 0, discovery_cmd_complete );

			} else {
				discovery_state[dev_id] = DISC_ST_GET_FRU_INVENTORY_AREA_INFO_SENT;
				send_get_fru_inventory_area_info( ipmi_ch, dev_addr, discovery_cmd_complete );
			}
			break;
		case NVSTORE_CMD_GET_FRU_INVENTORY_AREA_INFO: {
			GET_FRU_INVENTORY_AREA_CMD_RESP *resp = ( GET_FRU_INVENTORY_AREA_CMD_RESP * )(pkt->resp);
			amc[dev_id].fru_data.fru_inventory_area_size = 
				( resp->fru_inventory_area_size_msb << 8 ) | resp->fru_inventory_area_size_lsb;
			// read header first
			amc[dev_id].current_fru_offset = 0;
			amc[dev_id].fru_read_state = FRU_READ_ST_READING_FRU_HDR;
			discovery_state[dev_id] = DISC_ST_READ_FRU_DATA_SENT;
			send_read_fru_data( ipmi_ch, dev_addr, 0, sizeof( FRU_COMMON_HEADER ), discovery_cmd_complete );
			break;
			}
		case NVSTORE_CMD_IPMI_STO_CMD_READ_FRU_DATA: {
			READ_FRU_DATA_CMD_RESP *resp = ( READ_FRU_DATA_CMD_RESP * )(pkt->resp);
			FRU_COMMON_HEADER *fru_hdr = ( FRU_COMMON_HEADER * )&amc[dev_id].fru_data.fru[0];
			MULTIRECORD_AREA_HEADER *mr_hdr;
			
			unsigned short offset;
			
			// Calculate the offset value we used for requesting the data
			offset = ( ( READ_FRU_DATA_CMD_REQ * )( pkt->req ) )->fru_inventory_offset_lsb;
			offset |= ( ( ( READ_FRU_DATA_CMD_REQ * )( pkt->req ) )->fru_inventory_offset_msb ) << 8;
			
			// TODO offset sanity checking
			// Copy the retrieved data
			memcpy( &amc[dev_id].fru_data + offset, &(resp->data), resp->count_returned );

			switch( amc[dev_id].fru_read_state ) {

				case FRU_READ_ST_READING_FRU_HDR:
					// we read the FRU HDR now read the first record hdr
					amc[dev_id].fru_read_state = FRU_READ_ST_READING_MULTIRECORD_AREA;
					offset = fru_hdr->multirecord_offset;
					amc[dev_id].current_fru_offset = offset;
					amc[dev_id].current_read_len = sizeof( MULTIRECORD_AREA_HEADER );
					if( offset )
						send_read_fru_data( ipmi_ch, dev_addr, 
								offset, 
								sizeof( MULTIRECORD_AREA_HEADER ), 
								discovery_cmd_complete );
					break;

				case FRU_READ_ST_READING_INTERNAL_USE_AREA:
				case FRU_READ_ST_READING_CHASSIS_INFO:
				case FRU_READ_ST_READING_BOARD_AREA:
				case FRU_READ_ST_READING_PRODUCT_INFO:
					break;
					
				case FRU_READ_ST_READING_MULTIRECORD_AREA:
					if( fru_hdr->multirecord_offset == amc[dev_id].current_fru_offset ) {
						// We've read the header of the first record, get the record len
						mr_hdr = ( MULTIRECORD_AREA_HEADER * )
							( &amc[dev_id].fru_data + offset );
						amc[dev_id].current_record_len = mr_hdr->record_len;
					} 
					amc[dev_id].record_read += amc[dev_id].current_read_len;
					
					if( amc[dev_id].record_read >= amc[dev_id].current_record_len ) { 
						discovery_state[dev_id] = DISC_ST_READ_FRU_DATA_COMPLETE;
						mcmc_mmc_event( dev_id, AMC_EVT_DEVICE_DISCOVERY_OK );
						break;
					}
					if( ( amc[dev_id].current_read_len = 
					    amc[dev_id].current_record_len - amc[dev_id].record_read ) > 16 ) {
						amc[dev_id].current_read_len = 16;
					}
					send_read_fru_data( ipmi_ch, dev_addr, 
						amc[dev_id].current_fru_offset, 
						amc[dev_id].current_read_len, discovery_cmd_complete );
					break;
			}
			break;
		}
		break;
	}
	ws_free( ws );			
}

#define NUM_FRU_ENTRIES 5 // int_use_offset, chassis_info_offset, board_offset, product_info_offset, multirecord_offset
/*
The offset values in the FRU header is in multiples of 8 bytes so any value
we get has to be multiplied by 8 (<<3)
*/
unsigned short
get_next_fru_offset( uchar dev_id, uchar current_offset )
{
	int i;

	FRU_COMMON_HEADER *fru_hdr = ( FRU_COMMON_HEADER * )&amc[dev_id].fru_data.fru[0];
	uchar *offset_array = &fru_hdr->int_use_offset;
	uchar offset = current_offset;

	offset = current_offset;
	if( offset == 0 ) {
		offset = fru_hdr->int_use_offset << 3;
		if( offset )
			return ( offset << 3 );
	}

	for( i = 0; i < NUM_FRU_ENTRIES - 1; i++ ) {
		if( offset == offset_array[i] ) {
			offset = offset_array[i + 1];
			if( offset )
				return ( offset << 3 );
		}
	}

	return 0xffff;
}

uchar
get_fru_index( uchar dev_id, uchar offset )
{
	FRU_COMMON_HEADER *fru_hdr = ( FRU_COMMON_HEADER * )&amc[dev_id].fru_data.fru[0];
	uchar *offset_array = &fru_hdr->int_use_offset;
	unsigned i;

	for( i = 0; i < NUM_FRU_ENTRIES; i++ ) {
		if( offset_array[i] == offset )
			return i;
	}
	return 0xff;
}
		
	

void
module_cold_reset( uchar dev_id )
{
}

void
module_warm_reset( uchar dev_id )
{
}

void
module_graceful_reboot( uchar dev_id )
{
}

void
module_issue_diag_int( uchar dev_id )
{
}

void
module_quiesce( uchar dev_id )
{
}


void
module_term_process( uchar * ptr )
{
	int i, buf_len, val;
	uchar dev_addr, dev_id, rec_id = 0, digit_count, digit[2];
	uchar *cptr;
	uchar ipmi_ch = IPMI_CH_NUM_IPMBL;
	dev_addr = module_get_i2c_address( I2C_ADDRESS_REMOTE );

	dev_id = lookup_dev_id( dev_addr );

	// Set dev id to use
	if( ( strncmp( ( const char * )ptr, "DEVID", 5 ) == 0 ) 
			|| ( strncmp( ptr, "devid", 5 ) == 0 ) ) {

		cptr = ptr + 5;
		digit_count = 0;
		buf_len = strlen( ( const char * )cptr );
		
		while( cptr < ptr + buf_len ) {
			if( ( ( *cptr >= '0' ) && ( *cptr <= '9' ) ) &&
			    ( digit_count < 2 ) ) {
				digit[digit_count] = *cptr;
				digit_count++;
				cptr++;
				if( digit_count == 2 ) {
					val = ( digit[0] - 48 ) << 4;
					val += ( digit[1] - 48 );
				} 
			} else if ( *cptr == ' ' ) {
				digit_count = 0;
				putchar( *cptr++ );
			} else if ( *cptr == ']' ) {
				putstr( "\n" );
				g_dev_id = val;
				putstr( "dev id set to " );
				putchar( digit[0] );
				putchar( digit[1] );
				putstr( "\n" );
				putstr( "i2c addr set to 0x" );
				puthex( lookup_dev_addr( g_dev_id ) );
				putstr( "\n" );
				return;
			} else {
				/* invalid character */
				putstr( "[ERR]\n" );
				return;
			}
		}
		putstr( "[ERR]\n" );
		return;
	}
	
	// get port state
	if( ( strncmp( ( const char * )ptr, "GPS]", 4 ) == 0 ) 
			|| ( strncmp( ptr, "gps]", 4 ) == 0 ) ) {
		putstr( "sending get port state cmd\n" );
		send_get_amc_port_state( ipmi_ch, dev_addr, cmd_complete );
		return;
	}
	// set port state
	if( ( strncmp( ( const char * )ptr, "SPS]", 4 ) == 0 ) 
			|| ( strncmp( ptr, "sps]", 4 ) == 0 ) ) {
		putstr( "sending set port state cmd\n" );
		send_set_amc_port_state( ipmi_ch, dev_addr, cmd_complete );
		return;
	}
	// set led on
	if( ( strncmp( ( const char * )ptr, "LEDON]", 6 ) == 0 ) 
			|| ( strncmp( ptr, "ledon]", 6 ) == 0 ) ) {
		putstr( "sending set fru LED state cmd\n" );
		send_set_fru_led_state( ipmi_ch, dev_addr, LED_ON, cmd_complete );
		return;
	}

	// set led off
	if( ( strncmp( ( const char * )ptr, "LEDOFF]", 7 ) == 0 ) 
			|| ( strncmp( ptr, "ledoff]", 7 ) == 0 ) ) {
		putstr( "sending set fru LED state cmd\n" );
		send_set_fru_led_state( ipmi_ch, dev_addr, LED_OFF, cmd_complete );
		return;
	}

	// led long blink
	if( ( strncmp( ( const char * )ptr, "LEDLONG]", 6 ) == 0 ) 
			|| ( strncmp( ptr, "ledlong]", 6 ) == 0 ) ) {
		putstr( "sending set fru LED state cmd\n" );
		send_set_fru_led_state( ipmi_ch, dev_addr, LED_LONG_BLINK, cmd_complete );
		return;
	}

	// led short blink
	if( ( strncmp( ( const char * )ptr, "LEDSHORT]", 6 ) == 0 ) 
			|| ( strncmp( ptr, "ledshort]", 6 ) == 0 ) ) {
		putstr( "sending set fru LED state cmd\n" );
		send_set_fru_led_state( ipmi_ch, dev_addr, LED_SHORT_BLINK, cmd_complete );
		return;
	}

	// get device ID
	if( ( strncmp( ( const char * )ptr, "GDI]", 4 ) == 0 ) 
			|| ( strncmp( ptr, "gdi]", 4 ) == 0 ) ) {
		putstr( "sending get device id cmd\n" );
		for( i = 0; i < 6; i++)
			send_get_device_id( ipmi_ch, dev_addr, cmd_complete );
		return;
	}

	// get fru inventory area info
	if( ( strncmp( ( const char * )ptr, "FRUINV]", 7 ) == 0 ) 
			|| ( strncmp( ptr, "fruinv]", 7 ) == 0 ) ) {
		putstr( "sending get fru inventory area info cmd\n" );
		send_get_fru_inventory_area_info( ipmi_ch, dev_addr, cmd_complete );
		return;
	}

	// read FRU data
	if( ( strncmp( ( const char * )ptr, "FRUDATA]", 8 ) == 0 ) 
			|| ( strncmp( ptr, "frudata]", 8 ) == 0 ) ) {
		putstr( "sending read fru data cmd\n" );
		send_read_fru_data( ipmi_ch, dev_addr, 0, sizeof( FRU_COMMON_HEADER ), cmd_complete );
		return;
	}
	
	// read FRU Power data
	if( ( strncmp( ( const char * )ptr, "PWRDATA]", 8 ) == 0 ) 
			|| ( strncmp( ptr, "pwrdata]", 8 ) == 0 ) ) {
		putstr( "sending read fru power data cmd\n" );
		send_read_fru_data( ipmi_ch, dev_addr, 0x16, 0x10, cmd_complete );
		return;
	}

	// FRU cold reset
	if( ( strncmp( ( const char * )ptr, "FRUCRST]", 8 ) == 0 ) 
			|| ( strncmp( ptr, "frucrst]", 8 ) == 0 ) ) {
		putstr( "sending fru control (cold reset) cmd\n" );
		send_fru_control( ipmi_ch, dev_addr, FRU_CONTROL_COLD_RESET, cmd_complete );
		return;
	}

	// FRU warm reset
	if( ( strncmp( ( const char * )ptr, "FRUWRST]", 8 ) == 0 ) 
			|| ( strncmp( ptr, "fruwrst]", 8 ) == 0 ) ) {
		putstr( "sending fru control (warm reset) cmd\n" );
		send_fru_control( ipmi_ch, dev_addr, FRU_CONTROL_WARM_RESET, cmd_complete );
		return;
	}

	// Get PICMG Properties
	if( ( strncmp( ( const char * )ptr, "PICPROP]", 8 ) == 0 ) 
			|| ( strncmp( ptr, "picprop]", 8 ) == 0 ) ) {
		putstr( "sending get picmg properties cmd\n" );
		send_get_picmg_properties( ipmi_ch, dev_addr, cmd_complete );
		return;
	}

	// Get sensor reading
	if( ( strncmp( ( const char * )ptr, "SENSOR]", 7 ) == 0 ) 
			|| ( strncmp( ptr, "sensor]", 7 ) == 0 ) ) {
		putstr( "sending get sensor reading cmd\n" );
		send_get_sensor_reading( ipmi_ch, dev_addr, 0, cmd_complete );
		return;
	}

	// Get Device SDR Info
	if( ( strncmp( ( const char * )ptr, "SDRINFO]", 8 ) == 0 ) 
			|| ( strncmp( ptr, "sdrinfo]", 8 ) == 0 ) ) {
		putstr( "sending get device sdr info cmd\n" );
		send_get_device_sdr_info( ipmi_ch, dev_addr, 0, cmd_complete );
		return;
	}
	
	// Get Device SDR
	if( ( strncmp( ( const char * )ptr, "SDR]", 4 ) == 0 ) 
			|| ( strncmp( ptr, "sdr]", 4 ) == 0 ) ) {
		putstr( "sending get device sdr cmd\n" );
		send_get_device_sdr( ipmi_ch, dev_addr, rec_id, cmd_complete );
		return;
	}
	
	// Reserve Device SDR Repository
	if( ( strncmp( ( const char * )ptr, "RSDR]", 5 ) == 0 ) 
			|| ( strncmp( ptr, "rsdr]", 5 ) == 0 ) ) {
		putstr( "sending reserve device sdr repository cmd\n" );
		send_reserve_device_sdr_repository( ipmi_ch, dev_addr, cmd_complete );
		return;
	}

	// Get FRU LED Properties
	if( ( strncmp( ( const char * )ptr, "LEDPROP]", 8 ) == 0 ) 
			|| ( strncmp( ptr, "ledprop]", 8 ) == 0 ) ) {
		putstr( "sending get FRU LED properties cmd\n" );
		send_get_fru_led_properties( ipmi_ch, dev_addr, cmd_complete );
		return;
	}

	// Get LED Color Capabilities
	if( ( strncmp( ( const char * )ptr, "LEDCOLOR]", 9 ) == 0 ) 
			|| ( strncmp( ptr, "ledcolor]", 9 ) == 0 ) ) {
		putstr( "sending get LED color capabilities cmd\n" );
		send_get_led_color_capabilities( ipmi_ch, dev_addr, 0, cmd_complete );
		return;
	}

	// Get FRU LED State
	if( ( strncmp( ( const char * )ptr, "LEDSTATE]", 9 ) == 0 ) 
			|| ( strncmp( ptr, "ledstate]", 9 ) == 0 ) ) {
		putstr( "sending get FRU LED state cmd\n" );
		send_get_fru_led_state( ipmi_ch, dev_addr, cmd_complete );
		return;
	}

	// Get Device Locator Record ID
	if( ( strncmp( ( const char * )ptr, "DEVLOC]", 7 ) == 0 ) 
			|| ( strncmp( ptr, "devloc]", 7 ) == 0 ) ) {
		putstr( "sending get device locator record id cmd\n" );
		send_get_device_locator_record_id( ipmi_ch, dev_addr, cmd_complete );
		return;
	}

	// FRU Control Capabilities
	if( ( strncmp( ( const char * )ptr, "FRUCAP]", 7 ) == 0 ) 
			|| ( strncmp( ptr, "frucap]", 7 ) == 0 ) ) {
		putstr( "sending get FRU control capabilities cmd\n" );
		send_get_fru_control_capabilities( ipmi_ch, dev_addr, cmd_complete );
		return;
	}

	// Set AMC Port State
	if( ( strncmp( ( const char * )ptr, "SETPORT]", 8 ) == 0 ) 
			|| ( strncmp( ptr, "setport]", 8 ) == 0 ) ) {
		putstr( "sending set AMC port state cmd\n" );
		send_set_amc_port_state( ipmi_ch, dev_addr, cmd_complete );
		return;
	}

	// Get AMC Port State
	if( ( strncmp( ( const char * )ptr, "GETPORT]", 8 ) == 0 ) 
			|| ( strncmp( ptr, "getport]", 8 ) == 0 ) ) {
		putstr( "sending get AMC port state cmd\n" );
		send_get_amc_port_state( ipmi_ch, dev_addr, cmd_complete );
		return;
	}

	// Set Clock State
	if( ( strncmp( ( const char * )ptr, "SETCLOCK]", 9 ) == 0 ) 
			|| ( strncmp( ptr, "setclock]", 9 ) == 0 ) ) {
		putstr( "sending set clock state cmd\n" );
		send_set_amc_clock_state( ipmi_ch, dev_addr, cmd_complete );
		return;
	}

	// Get Clock State
	if( ( strncmp( ( const char * )ptr, "GETCLOCK]", 9 ) == 0 ) 
			|| ( strncmp( ptr, "getclock]", 9 ) == 0 ) ) {
		putstr( "sending get clock state cmd\n" );
		send_get_amc_clock_state( ipmi_ch, dev_addr, cmd_complete );
		return;
	}
	
	// Start device discovery
	if( ( strncmp( ( const char * )ptr, "DISC]", 5 ) == 0 ) 
			|| ( strncmp( ptr, "disc]", 5 ) == 0 ) ) {
		putstr( "starting device discovery\n" );
		device_discovery( dev_id );
		return;
	}
	
	// Get device state
	if( ( strncmp( ( const char * )ptr, "DSTATE]", 7 ) == 0 ) 
			|| ( strncmp( ptr, "dstate]", 7 ) == 0 ) ) {
		putstr( "retrieving discovery state\n" );
		dump_discovery_state( dev_id );
		return;
	}
	
	// Get AMC info
	if( ( strncmp( ( const char * )ptr, "AMCINFO]", 8 ) == 0 ) 
			|| ( strncmp( ptr, "amcinfo]", 8 ) == 0 ) ) {
		putstr( "dumping AMC INFO struct\n" );
		dump_amc_info( dev_id );
		return;
	}
}


void 
module_process_response( IPMI_WS *req_ws, uchar seq, uchar completion_code )
{

}

void
module_rearm_events( void )
{

}

void
dump_outgoing( IPMI_WS *req_ws )
{
	int i;
#ifdef DEBUG_OUTGOING
		putstr( "\n[" );
		for( i = 0; i < req_ws->len_out; i++ ) {
			puthex( req_ws->pkt_out[i] );
			putchar( ' ' );
		}
		putstr( "]\n" );
#endif
}

void
dump_amc_info( uchar dev_id )
{
	int i, len = sizeof( AMC_INFO );
	uchar *ptr = ( uchar * )&amc[dev_id];
	
	putstr( "\n[" );
	for( i = 0; i < len; i++ ) {
		puthex( ptr[i] );
		putchar( ' ' );
	}
	putstr( "]\n" );

}

void
dump_discovery_state( uchar dev_id )
{
	putstr( "\n[" );
	puthex( discovery_state[dev_id] );
	putstr( "]\n" );
}

/*
 * module_if_lookup()
 *
 * Given an "ipmi channel number" i.e IPMI_CH_NUM_PRIMARY_IPMB
 * return the interface number i.e. i2c-0 to use.
 *
 * Arguments: 
 * 	IPMI_CH_NUM_PRIMARY_IPMB
 * 	IPMI_CH_NUM_CONSOLE
 * 	IPMI_CH_NUM_IPMBL
 * 	IPMI_CH_NUM_LOCAL
 * 	IPMI_CH_NUM_PRESENT_INTERFACE
 * 	IPMI_CH_NUM_SYS_INTERFACE
 */
void
module_if_lookup( uchar ipmi_channel, uchar *if1, uchar *if2  )
{
	switch( ipmi_channel ) {
		case IPMI_CH_NUM_PRIMARY_IPMB:
			*if1 = 2;
			*if2 = 3;
			break;

		case IPMI_CH_NUM_IPMBL:
			*if1 = 0;
			*if2 = 0xff;
			break;

		case IPMI_CH_NUM_LOCAL:
			*if1 = 1;
			*if2 = 0xff;
			break;
	}
}
