/*
-------------------------------------------------------------------------------
coreIPM/picmg.c

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

#include "ipmi.h"
#include "ws.h"
#include "i2c.h"
#include "timer.h"
#include "debug.h"
#include "wd.h"
#include "stdio.h"
#include "stdarg.h"
#include "debug.h"
#include "picmg.h"
#include "gpio.h"
#include "serial.h"
#include "fan.h"
#include "module.h"
#include "event.h"
#ifdef MMC
#include "mmc.h"
#endif

#define	FRU_DEV_ID	1
#define SITE_ID		1
#define PICMG_ID	0
#define NUM_PICMG_ADDRESS_INFO_TABLE_ENTRIES	16

#define LONG_BLINK_ON	9 /* in 100 ms */
#define LONG_BLINK_OFF	1 /* in 100 ms */
#define SHORT_BLINK_ON	1 /* in 100 ms */
#define SHORT_BLINK_OFF	9 /* in 100 ms */

FRU_INFO fru[MAX_FRU_DEV_ID + 1];
FRU_FAN_INFO fru_fan[MAX_FRU_DEV_ID + 1];
PICMG_ADDRESS_INFO picmg_address_info_table[NUM_PICMG_ADDRESS_INFO_TABLE_ENTRIES] = { { 0, 0, 0, 0, 0 } };
uchar controller_fru_dev_id = 0; // fru dev id for the BMC

#define NUM_IPMB_SENSORS			2
struct {
	uchar	link_number;
	uchar	sensor_number;
} ipmb_sensor[NUM_IPMB_SENSORS] = { { 0, 0 }, { 1, 1 } };

#define NUM_LINK_INFO_ENTRIES	8
LINK_INFO_ENTRY link_info_table[NUM_LINK_INFO_ENTRIES];

void picmg_m1_state( unsigned fru_id );
void picmg_m2_state( unsigned fru_id );
void picmg_m3_state( unsigned fru_id );
void picmg_m4_state( unsigned fru_id );
void picmg_m5_state( unsigned fru_id );
void picmg_m6_state( unsigned fru_id );

void picmg_get_picmg_properties( IPMI_PKT * );
void picmg_get_address_info( IPMI_PKT * );
void picmg_get_shelf_address_info( IPMI_PKT * );
void picmg_set_shelf_address_info( IPMI_PKT * );
void picmg_fru_control( IPMI_PKT * );
void picmg_get_fru_led_properties( IPMI_PKT * );
void picmg_get_led_color_capabilities( IPMI_PKT * );
void picmg_set_fru_led_state( IPMI_PKT * );
void picmg_get_fru_led_state( IPMI_PKT * );
void picmg_set_ipmb_state( IPMI_PKT * );
void picmg_set_fru_activation_policy( IPMI_PKT * );
void picmg_get_fru_activation_policy( IPMI_PKT *kt );
void picmg_set_fru_activation( IPMI_PKT * );
void picmg_get_device_locator_rec_id( IPMI_PKT * );
void picmg_set_port_state( IPMI_PKT * );
void picmg_get_port_state( IPMI_PKT * );
void picmg_compute_power_properties( IPMI_PKT * );
void picmg_set_power_level( IPMI_PKT * );
void picmg_get_power_level( IPMI_PKT * );
void picmg_renegotiate_power( IPMI_PKT * );
void picmg_get_fan_speed_properties( IPMI_PKT * );
void picmg_set_fan_level( IPMI_PKT * );
void picmg_get_fan_level( IPMI_PKT * );
void picmg_bused_resource_control( IPMI_PKT * );
void picmg_get_ipmb_link_info( IPMI_PKT * );


/*======================================================================*/
/*    AdvancedTCA® and PICMG® specific request commands
 *
 *    Mandatory Commands
 * 	Get PICMG Properties
 * 	Get Address Info
 * 	FRU Control
 * 	Get FRU LED Properties
 * 	Get LED Color Capabilities
 * 	Set FRU LED State
 * 	Get FRU LED State
 * 	Set IPMB State
 * 	Set FRU Activation Policy
 * 	Get FRU Activation Policy
 * 	Set FRU Activation
 * 	Get Device Locator Record ID
 * 	Compute Power Properties
 * 	Set Power Level
 * 	Get Power Level
 * 	Get Fan Speed Properties
 */
void
picmg_process_command( IPMI_PKT *pkt )
{
#ifdef PICMG
	PICMG_CMD_RESP	*resp = ( PICMG_CMD_RESP * )pkt->resp;
	/* Ignore checking PICMG Identifier for the time being. 
	 * Indicates that this is a PICMG®-defined group
	 * extension command. A value of 00h shall be used.
	 */
	dprintf( DBG_IPMI | DBG_INOUT, "picmg_process_command: ingress\n" );

	switch( pkt->req->command ) {
		case ATCA_CMD_GET_PICMG_PROPERTIES:	/* Get PICMG Properties */
			picmg_get_picmg_properties( pkt );
			break;
		case ATCA_CMD_GET_ADDRESS_INFO:		/* Get Address Info */
			picmg_get_address_info( pkt );
			break;
		case ATCA_CMD_GET_SHELF_ADDRESS_INFO:	/* Get Shelf Address Info */
			picmg_get_shelf_address_info( pkt );
			break;
		case ATCA_CMD_SET_SHELF_ADDRESS_INFO:	/* Set Shelf Address Info */
			picmg_set_shelf_address_info( pkt );
			break;
		case ATCA_CMD_FRU_CONTROL:		/* FRU Control */
			picmg_fru_control( pkt );
			break;
		case ATCA_CMD_GET_FRU_LED_PROPERTIES:	/* Get FRU LED Properties */
			picmg_get_fru_led_properties( pkt );
			break;
		case ATCA_CMD_GET_LED_COLOR:		/* Get LED Color Capabilities */
			picmg_get_led_color_capabilities( pkt );
			break;
		case ATCA_CMD_SET_FRU_LED_STATE:	/* Set FRU LED State */
			picmg_set_fru_led_state( pkt );
			break;
		case ATCA_CMD_GET_FRU_LED_STATE:	/* Get FRU LED State */
			picmg_get_fru_led_state( pkt );
			break;
		case ATCA_CMD_SET_IPMB_STATE:		/* Set IPMB State */
			picmg_set_ipmb_state( pkt );
			break;
		case ATCA_CMD_SET_FRU_ACTIVATION_POLICY:	/* Set FRU Activation Policy */
			picmg_set_fru_activation_policy( pkt );
			break;
		case ATCA_CMD_GET_FRU_ACTIVATION_POLICY:	/* Get FRU Activation Policy */
			picmg_get_fru_activation_policy( pkt );
			break;
		case ATCA_CMD_SET_FRU_ACTIVATION:	/* Set FRU Activation */
			picmg_set_fru_activation( pkt );
			break;
		case ATCA_CMD_GET_DEVICE_LOCATOR_REC_ID:	/* Get Device Locator Record ID */
			picmg_get_device_locator_rec_id( pkt );
			break;
		case ATCA_CMD_SET_PORT_STATE:		/* Set Port State */
			picmg_set_port_state( pkt );
			break;
		case ATCA_CMD_GET_PORT_STATE:		/* Get Port State */
			picmg_get_port_state( pkt );
			break;
		case ATCA_CMD_COMPUTE_POWER_PROPERTIES:	/* Compute Power Properties */
			picmg_compute_power_properties( pkt );
			break;
		case ATCA_CMD_SET_POWER_LEVEL:		/* Set Power Level */
			picmg_set_power_level( pkt );
			break;
		case ATCA_CMD_GET_POWER_LEVEL:		/* Get Power Level */
			picmg_get_power_level( pkt );
			break;
		case ATCA_CMD_RENEGOTIATE_POWER:	/* Renegotiate Power */
			picmg_renegotiate_power( pkt );
			break;
		case ATCA_CMD_GET_FAN_SPEED_PROPERTIES:	/* Get Fan Speed Properties */
			picmg_get_fan_speed_properties( pkt );
			break;
		case ATCA_CMD_SET_FAN_LEVEL:		/* Set Fan Level */
			picmg_set_fan_level( pkt );
			break;
		case ATCA_CMD_GET_FAN_LEVEL:		/* Get Fan Level */
			picmg_get_fan_level( pkt );
			break;
		case ATCA_CMD_BUSED_RESOURCE_CONTROL:		/* Bused Resource */
			picmg_bused_resource_control( pkt );
			break;
		case ATCA_CMD_GET_IPMB_LINK_INFO:	/* Get IPMB Link Info */
			picmg_get_ipmb_link_info( pkt );
			break;
#ifdef MMC
		case ATCA_CMD_FRU_CONTROL_CAPABILITIES:	/* FRU control capabilities */
			mmc_get_fru_control_capabilities( pkt );
			break;
		case ATCA_CMD_SET_AMC_PORT_STATE:
			mmc_set_port_state( pkt );
			break;
		case ATCA_CMD_GET_AMC_PORT_STATE:
			mmc_get_port_state( pkt );
			break;
		case ATCA_CMD_SET_CLOCK_STATE:
			mmc_set_clock_state( pkt );
			break;
		case ATCA_CMD_GET_CLOCK_STATE:
			mmc_get_clock_state( pkt );
			break;
#endif
		default:
			resp->completion_code = CC_INVALID_CMD;
			resp->picmg_id = PICMG_ID;
			pkt->hdr.resp_data_len = 0;
    			break;	
	} /* end switch */
#endif
}

#ifdef PICMG
void
picmg_get_picmg_properties( IPMI_PKT *pkt )
{
	GET_PICMG_PROPERTIES_CMD_RESP	*resp = (GET_PICMG_PROPERTIES_CMD_RESP *)(pkt->resp);

	dprintf( DBG_IPMI | DBG_INOUT, "picmg_get_picmg_properties: ingress\n" );

	resp->completion_code = CC_NORMAL;
	resp->picmg_id = PICMG_ID;
	resp->picmg_extension_ver_minor = 1;
	resp->picmg_extension_ver_major = 2;
	/* PICMG Extension Version. Indicates the version of PICMG® extensions 
	   implemented by the IPM Controller.
	   [7:4] = BCD encoded minor version
	   [3:0] = BCD encoded major version
	   This implementation is compliant with version 2.1 of the PICMG® 
	   extensions. IPM Controllers implementing the extensions as 
	   defined by this specification shall report a value of 12h.The 
	   value 00h is reserved. */
	resp->max_fru_dev_id = MAX_FRU_DEV_ID;
	/* Max FRU Device ID. The numerically largest FRU Device ID for the
	   Managed FRUs implemented by this IPM Controller, excluding FRU
	   Device IDs reserved at the top of the range for special purposes,
	   as detailed in Table 3-10, “Reserved FRU Device IDs.” */
	resp->ipmc_fru_dev_id = 0;
	/* FRU Device ID for IPM Controller. Indicates a FRU Device ID for the
	   FRU containing the IPM Controller. IPM Controllers implementing the
	   extensions defined by the IPMI specification shall report 0. */
	pkt->hdr.resp_data_len = sizeof( GET_PICMG_PROPERTIES_CMD_RESP ) - 1;

}

/*
 * picmg_get_address_info()
 * 
 Each IPM Controller that supports an alternative interface besides IPMB also
 implements two variations of the command. Variation one is that support for 
 Request data bytes 2 - 5 are not required. The second variation is that 
 Request bytes 3 - 5 are not required. When the IPM Controller receives the 
 message without byte 2 and beyond, it defaults FRU Device ID to the FRU number 
 that holds the IPM Controller (always 0 under this specification). When bytes 
 3 - 5 are missing, the IPM Controller returns address information about the 
 local IPM Controller and the FRU located at the FRU Device ID. This provides 
 a method for the host interface to retrieve information such as the device’s 
 IPMB address. This data is needed for the host interface to do a “Send Message”
 or “Get Message”.
 */
void
picmg_get_address_info( IPMI_PKT *pkt )
{
	GET_ADDRESS_INFO_CMD_REQ	*req = ( GET_ADDRESS_INFO_CMD_REQ * )pkt->req;
	GET_ADDRESS_INFO_CMD_RESP	*resp = ( GET_ADDRESS_INFO_CMD_RESP * )pkt->resp;
	unsigned		cmd_len = pkt->hdr.req_data_len + 1; 
	unsigned char		cc = CC_NORMAL, fru, i, found = 0;

	dprintf( DBG_IPMI | DBG_INOUT, "picmg_get_address_info: ingress\n" );

	if( cmd_len < 3 ) {
		/* variation 1: the command shall return addressing information
		 * for the FRU containing the IPM Controller that implements
		 * the command.
		 */
		for( i = 0; i < NUM_PICMG_ADDRESS_INFO_TABLE_ENTRIES; i++ ) {
			if( picmg_address_info_table[i].fru_dev_id == controller_fru_dev_id ) {
				found = 1;
				break;
			}
		}
	} else if ( cmd_len < 4 ) {
		/*  variation 2: the IPM Controller returns address information 
		 *  about the local IPM Controller and the FRU located at the 
		 *  FRU Device ID.
		 */
		for( i = 0; i < NUM_PICMG_ADDRESS_INFO_TABLE_ENTRIES; i++ ) {
			if( picmg_address_info_table[i].fru_dev_id == req->fru_dev_id ) {
				found = 1;
				break;
			}
		}
	} else {
		/* full command - implemented only for shelf controllers */

		/* req->addr_key is is the address to look-up in the table. This field is 
		   required if Address Key Type is present. This holds a Hardware Address, 
		   IPMB address, or Site Number depending on what is in the Address Key Type. */

		switch( req->addr_key_type ) {	
			/* Address Key Type. This defines the type of address
			   that is being provided in the Address Key field. */
			
			case AKT_HW_ADDR:
				/* Hardware Address. This is an address
				   assigned with hardware signals from
				   the Shelf to a Front Board. */
				for( i = 0; i < NUM_PICMG_ADDRESS_INFO_TABLE_ENTRIES; i++ ) {
					if( picmg_address_info_table[i].hw_addr == req->addr_key ) {
						found = 1;
						break;
					}
				}
				break;
				
			case AKT_IPMB0_ADDR:	/* IPMB-0 Address */
				for( i = 0; i < NUM_PICMG_ADDRESS_INFO_TABLE_ENTRIES; i++ ) {
					if( picmg_address_info_table[i].ipmb0_addr == req->addr_key ) {
						found = 1;
						break;
					}
				}
				break;
				
			case AKT_PHYS_ADDR:	
				/* Physical Address. This is an address that 
				   defines the physical location of a FRU; consists
				   of Site Type and Site Number. */
				/* req->site_type is required is also required 
				   to further qualify the match if Address Key
				   Type is a Physical Address. */
				for( i = 0; i < NUM_PICMG_ADDRESS_INFO_TABLE_ENTRIES; i++ ) {
					if( picmg_address_info_table[i].phys_addr == req->addr_key ) {
						found = 1;
						break;
					}
				}
				break;
			case AKT_RESERVED:	/* Reserved for PICMG® 2.9. */
			default:
				break;
		}
	}

	/* fill in the response */
	if( found ) {
		resp->completion_code = CC_NORMAL;	/* Completion Code. */
		resp->picmg_id = PICMG_ID;	/* PICMG Identifier. Indicates that this 
					   is a PICMG®-defined group extension
					   command. A value of 00h shall be used. */
		resp->hw_addr = picmg_address_info_table[i].hw_addr;		/* Hardware Address. */
		resp->ipmb0_addr = picmg_address_info_table[i].ipmb0_addr;
		/* IPMB-0 Address. Indicates the IPMB address for IPMB-0, if implemented. For
		   PICMG 2.9. a value of FFh indicates that IPMB-0 is not implemented. */
		resp->reserved = 0xFF;		/* Reserved. Shall have a value of FFh. 
					   Other values reserved in PICMG® 2.9. */
		resp->fru_dev_id =  picmg_address_info_table[i].fru_dev_id;
		resp->site_id = picmg_address_info_table[i].site_id;
		resp->site_type = SITE_TYPE_ATCA;			/* Site Type. */
		pkt->hdr.resp_data_len = sizeof( GET_ADDRESS_INFO_CMD_RESP ) - 1;
	} else {
		resp->completion_code = CC_PARAM_OUT_OF_RANGE;
		pkt->hdr.resp_data_len = 0;
	}
}

/*
   Called at power-up
   - validate the Hardware Address
   - turn on BLUE LED
   - set state to M1 
   - register function to detect Handle Switch state changes
*/
void
picmg_init( void )
{
	int i;

	dprintf( DBG_IPMI | DBG_INOUT, "picmg_init: ingress\n" );

	/* reset fru states, M1 except for uninstalled mezzanine FRUs which are M0 */
	for( i = 0; i < MAX_FRU_DEV_ID; i++ )
		fru[i].state = FRU_STATE_M1_INACTIVE;

	/* turn on the BLUE LED */
	gpio_led_on( GPIO_FRU_LED_BLUE );

	/* if the Insertion Criteria Met condition exists then we can go to M2 state */
	if( gpio_get_handle_switch_state() == HANDLE_SWITCH_CLOSED ) {
		picmg_m2_state( 0 );
	}
}


void
picmg_m1_state( unsigned fru_id )
{
	FRU_HOT_SWAP_EVENT_MSG_REQ msg;

	dprintf( DBG_IPMI | DBG_INOUT, "picmg_m1_state: ingress\n" );

	/* set current state */
	fru[fru_id].state = FRU_STATE_M1_INACTIVE;

	/* turn on the BLUE LED */
	gpio_led_on( GPIO_FRU_LED_BLUE );
	
	/* announce presence in the shelf by sending a hot swap event msg to shelf controler */
	msg.command = IPMI_SE_PLATFORM_EVENT;
	msg.evt_msg_rev = IPMI_EVENT_MESSAGE_REVISION;
	msg.sensor_type = IPMI_SENSOR_HOT_SWAP;
	msg.sensor_number = 0;
	msg.evt_direction = IPMI_EVENT_TYPE_GENERIC_AVAILABILITY;
	msg.evt_data1 = 0xa << 4 | FRU_STATE_M2_ACTIVATION_REQUEST;
	msg.evt_data2 = STATE_CH_NORMAL << 4 | FRU_STATE_M1_INACTIVE;
	msg.evt_data3 = controller_fru_dev_id;

	/* dispatch message */
	ipmi_send_event_req( ( unsigned char * )&msg, sizeof( FRU_HOT_SWAP_EVENT_MSG_REQ ), 0 );

	/* if the Insertion Criteria Met condition exists then we can go to M2 state */
	if( ( gpio_get_handle_switch_state() == HANDLE_SWITCH_CLOSED ) && ( !fru[fru_id].locked ) ) {
		picmg_m2_state( 0 );
	}
}

/* 
   Once the Insertion Criteria Met condition exists, the FRU transitions to the
   M2 state if the "locked" bit is not set.
   - announce presence in the Shelf
   - blink BLUE LED at Long Blink rate to indicate to the operator that the 
     new FRU has contacted the Shelf Manager and is waiting to be activated.
     
   While in M2, the FRU awaits permission from the Shelf Manager to transition
   to M3 (Activation In Progress).

   Note: this function may be called in interrupt context.
  
*/
void
picmg_m2_state( unsigned fru_id )
{
	FRU_HOT_SWAP_EVENT_MSG_REQ msg;

	dprintf( DBG_IPMI | DBG_INOUT, "picmg_m2_state: ingress\n" );

	fru[fru_id].state = FRU_STATE_M2_ACTIVATION_REQUEST;

	/* blink blue LED at long blink rate */
	gpio_led_blink( GPIO_FRU_LED_BLUE, LONG_BLINK_ON, LONG_BLINK_OFF, 0 );	
	
	/* announce presence in the shelf by sending a hot swap event msg to shelf controler */
	msg.command = IPMI_SE_PLATFORM_EVENT;
	msg.evt_msg_rev = IPMI_EVENT_MESSAGE_REVISION;
	msg.sensor_type = IPMI_SENSOR_HOT_SWAP;
	msg.sensor_number = 0;
	msg.evt_direction = IPMI_EVENT_TYPE_GENERIC_AVAILABILITY;
	msg.evt_data1 = 0xa << 4 | FRU_STATE_M2_ACTIVATION_REQUEST;
	msg.evt_data2 = STATE_CH_NORMAL << 4 | FRU_STATE_M1_INACTIVE;
	msg.evt_data3 = controller_fru_dev_id;
	
	/* dispatch message */
	ipmi_send_event_req( ( unsigned char * )&msg, sizeof( FRU_HOT_SWAP_EVENT_MSG_REQ ), 0 );
}



/*
   The FRU leaves state M4 when Extraction Criteria Met occurs. Extraction Criteria Met occurs
   when the Deactivation-Locked bit is cleared. For FRUs with a Handle Switch, the Deactivation-
   Locked bit is cleared by the act of opening the Handle. Software (FRU level, Shelf Manager, or
   System Manager software) may also initiate a deactivation by clearing the Deactivation-Locked bit
   explicitly. The FRU transitions to M5 and then sends an event to the Shelf Manager that the FRU
   wishes to deactivate. The BLUE LED begins to blink at the Short Blink rate, indicating to the
   operator that deactivation has been requested. Since the FRU might be carrying a high availability
   function that cannot be shut down, the Shelf/System Manager determines if it is valid for the FRU
   to deactivate. Normally, the request is granted and the Shelf Manager sends a “Set FRU Activation
   (Deactivate FRU)” command to allow the FRU to begin deactivation. During the time the FRU is
   in the M5 state, the Payload functionality is not impacted. That is, from the Payload’s perspective,
   M4 and M5 are equivalent.
   When the FRU receives the deactivation command, it moves on to the M6 state, and does whatever
   steps are necessary to shut down the FRU’s Payload gracefully. The BLUE LED continues to blink
   at the Short Blink rate. Prior to transitioning to the M1 state, the FRU must power down all Payload
   functions and stop using at least its E-Keying-governed interfaces. It is the responsibility of the
   IPM Controller representing the FRU to do these tasks. The Shelf Manager does not send “Set
   Power Level” or “Set Port State” commands to the IPM Controller since the Shelf Manager does
   not know when the IPM Controller no longer needs the resources.
   Once the FRU’s Payload is powered down, it transitions to the M1 state and sends the event to the
   Shelf Manager. The BLUE LED is turned on. When the Shelf Manager receives the M6 to M1
   transition event, it reclaims the FRU’s power budget, removes the FRU from the SDR Repository,
   and if the FRU is a Front Board, it disables all Ports on other Front Boards that share an interface
   with this Front Board.
   At some point after the FRU enters M1, it most likely is extracted from the Shelf. Most of the time,
   the Shelf Manager does not get notification of the FRU transitioning to M0 since the FRU is no
   longer present.
   */


void
picmg_handle_switch_state_change( uchar state, uchar fru_id )
{
	dprintf( DBG_IPMI | DBG_INOUT, "picmg_handle_switch_state_change: ingress\n" );
	
	/* if the Insertion Criteria Met condition exists then we can go to M2 state */
	if( gpio_get_handle_switch_state() == HANDLE_SWITCH_CLOSED ) {
		picmg_m2_state( fru_id );
	} 

	/* clear locked bit */
	fru[fru_id].locked = 0;
	
	if( state == HANDLE_SWITCH_CLOSED ) {	// Handle closed
		switch( fru[fru_id].state ) {
			case FRU_STATE_M0_NOT_INSTALLED:
				/* how did we get here ? */
				break;
			case FRU_STATE_M1_INACTIVE:
				picmg_m2_state( fru_id );
				break;
			case FRU_STATE_M2_ACTIVATION_REQUEST:
				break;
			case FRU_STATE_M3_ACTIVATION_IN_PROGRESS:
				break;
			case FRU_STATE_M4_ACTIVE:
				break;
			case FRU_STATE_M5_DEACTIVATION_REQUEST:
				break;
			case FRU_STATE_M6_DEACTIVATION_IN_PROGRESS:
				break;
			case FRU_STATE_M7_COMMUNICATION_LOST:
				break;
			default:
				break;
		}
	} else {	// Handle open
		switch( fru[fru_id].state ) {
			case FRU_STATE_M0_NOT_INSTALLED:
				/* how did we get here ? */
				break;
			case FRU_STATE_M1_INACTIVE:
				break;
			case FRU_STATE_M2_ACTIVATION_REQUEST:
				picmg_m1_state( fru_id );
				break;
			case FRU_STATE_M3_ACTIVATION_IN_PROGRESS:
				break;
			case FRU_STATE_M4_ACTIVE:
				picmg_m5_state( fru_id );
				break;
			case FRU_STATE_M5_DEACTIVATION_REQUEST:
				break;
			case FRU_STATE_M6_DEACTIVATION_IN_PROGRESS:
				picmg_m6_state( fru_id );
				break;
			case FRU_STATE_M7_COMMUNICATION_LOST:
				break;
			default:
				break;
		}
	}
}

/*
   When the FRU receives the 
   “Set FRU Activation (Activate FRU)” command, it sets the Deactivation-Locked
   bit of the Activation Policy to 1b (true). This is done to enable the approach 
   to Extraction Criteria Met that is discussed in Section 3.2.4.1.2, “Typical 
   FRU extraction”.
   Once the FRU reaches the M3 state, it sends the M2 to M3 event to the Shelf
   Manager and waits for the Shelf Manager to begin power and/or cooling 
   negotiation (see Section 3.9, “Shelf power and cooling”). At this point, the
   BLUE LED is turned off. The Shelf Manager determines the proper power allocation
   and sends a “Set Power Level” command to inform the FRU of the power budget it
   has been allocated. The M3 state is also the place where the Shelf Manager 
   computes the E-Keying requirement; however, the FRU can transition to M4 prior
   to having received its EKeying. Though E-Keys are computed in M3, the E-keys
   may be read earlier in the FRU’s life cycle (i.e., in either M2 or M3). When 
   the Shelf Manager figures out which E-Keying to enable for the FRU, it sends 
   “Set Port State” command(s) to inform the FRU of the enabled and disabled Ports
   (see Section 3.7, “Electronic Keying”).
   The FRU determines when it begins using the power budget, enabling the Port 
   interfaces, and when it transitions to the M4 state. It might transition to M4 
   as soon as power is applied or it might wait until it has received all of its 
   Port enables or disables. When the FRU’s activation is complete, it sends an 
   M3 to M4 event to inform the Shelf Manager that it is now active.
   Once a FRU has reached the M4 state, the Shelf Manager’s job becomes monitoring
   the FRU for health related events and, for each Front Board, managing changes 
   to the E-Keying based on insertion or extraction of other Front Boards that 
   share an interface with that Front Board.
   */
void
picmg_set_fru_activation( IPMI_PKT *pkt )
{
	SET_FRU_ACTIVATION_CMD_REQ	*req = ( SET_FRU_ACTIVATION_CMD_REQ * )pkt->req;
	SET_FRU_ACTIVATION_CMD_RESP	*resp = ( SET_FRU_ACTIVATION_CMD_RESP * )(pkt->resp);
	unsigned char		cc = CC_NORMAL;

	dprintf( DBG_IPMI | DBG_INOUT, "picmg_set_fru_activation: ingress\n" );

	pkt->hdr.resp_data_len = sizeof( SET_FRU_ACTIVATION_CMD_RESP ) - 1;

	if ( req->fru_dev_id < MAX_FRU_DEV_ID + 1 ) {
		switch( req->fru_activation ) {
			case FRU_CONTROL_DEACTIVATE_FRU:
				if( fru[req->fru_dev_id].state == FRU_STATE_M5_DEACTIVATION_REQUEST ) {
					picmg_m6_state( req->fru_dev_id );
				}
				break;
			case FRU_CONTROL_ACTIVATE_FRU:
				if( fru[req->fru_dev_id].state == FRU_STATE_M2_ACTIVATION_REQUEST ) {
					picmg_m3_state( req->fru_dev_id );
				} else if ( fru[req->fru_dev_id].state == FRU_STATE_M5_DEACTIVATION_REQUEST ) {
					picmg_m4_state( req->fru_dev_id );
				} else {
					cc = CC_CMD_ILLEGAL;
					pkt->hdr.resp_data_len = 0;
				}
				break;
			default:
				cc = CC_PARAM_OUT_OF_RANGE;
				pkt->hdr.resp_data_len = 0;
				break;
		}
	} 
	else {
		cc = CC_PARAM_OUT_OF_RANGE;
		pkt->hdr.resp_data_len = 0;
	}

	resp->picmg_id = PICMG_ID;
	resp->completion_code = cc;
}

/*=============================================================================
 * picmg_m3_state()
 * 	Send M2 to M3 event to the Shelf Manager
 */
void
picmg_m3_state( unsigned fru_id )
{
	FRU_HOT_SWAP_EVENT_MSG_REQ msg;

	dprintf( DBG_IPMI | DBG_INOUT, "picmg_m3_state: ingress\n" );

	fru[fru_id].state = FRU_STATE_M3_ACTIVATION_IN_PROGRESS;

	/* blink blue LED at long blink rate */
	gpio_led_off( GPIO_FRU_LED_BLUE );

	fru[fru_id].deactivation_locked = 1;
		
	/* send transition msg to shelf controler */
	msg.command = IPMI_SE_PLATFORM_EVENT;
	msg.evt_msg_rev = IPMI_EVENT_MESSAGE_REVISION;
	msg.sensor_type = IPMI_SENSOR_HOT_SWAP;
	msg.sensor_number = 0;
	msg.evt_direction = IPMI_EVENT_TYPE_GENERIC_AVAILABILITY;
	msg.evt_data1 = 0xa << 4 | FRU_STATE_M3_ACTIVATION_IN_PROGRESS;
	msg.evt_data2 = STATE_CH_NORMAL << 4 | FRU_STATE_M2_ACTIVATION_REQUEST;
	msg.evt_data3 = controller_fru_dev_id;

	/* dispatch message */
	ipmi_send_event_req( ( unsigned char * )&msg, sizeof( FRU_HOT_SWAP_EVENT_MSG_REQ ), 0 );

}

void
picmg_m4_state( unsigned fru_id )
{
	FRU_HOT_SWAP_EVENT_MSG_REQ msg;

	dprintf( DBG_IPMI | DBG_INOUT, "picmg_m4_state: ingress\n" );
	
	fru[fru_id].state = FRU_STATE_M4_ACTIVE;

	/* architecture dependent - adjust power level */
	// SET_POWER(fru[req->fru_dev_id].power_level_steady_state)

	/* send transition msg to shelf controler */
	msg.command = IPMI_SE_PLATFORM_EVENT;
	msg.evt_msg_rev = IPMI_EVENT_MESSAGE_REVISION;
	msg.sensor_type = IPMI_SENSOR_HOT_SWAP;
	msg.sensor_number = 0;
	msg.evt_direction = IPMI_EVENT_TYPE_GENERIC_AVAILABILITY;
	msg.evt_data1 = 0;		/* Event Data 1
					   [7:4] = Ah (OEM code in Event Data 2,
					   OEM code in Event Data 3)
					   [3:0] = Current State  */
	msg.evt_data2 = 0;		/* Event Data 2
					   [7:4] = Cause of state change. 
					   See Table 3-20, “Cause of State Change values,” 
					   for values.
					   [3:0] = Previous State */
	msg.evt_data3 = 0;		/* Event Data 3
					   [7:0] = FRU Device ID */
	msg.evt_data1 = 0xa << 4 | FRU_STATE_M4_ACTIVE;
	msg.evt_data2 = STATE_CH_NORMAL << 4 | FRU_STATE_M3_ACTIVATION_IN_PROGRESS;
	msg.evt_data3 = controller_fru_dev_id;

	/* dispatch message */
	ipmi_send_event_req( ( unsigned char * )&msg, sizeof( FRU_HOT_SWAP_EVENT_MSG_REQ ), 0 );

}

void
picmg_m5_state( unsigned fru_id )
{
	FRU_HOT_SWAP_EVENT_MSG_REQ msg;

	dprintf( DBG_IPMI | DBG_INOUT, "picmg_m5_state: ingress\n" );
	
	fru[fru_id].state = FRU_STATE_M5_DEACTIVATION_REQUEST;

	/* blink blue LED at short blink rate */
	gpio_led_blink( GPIO_FRU_LED_BLUE, SHORT_BLINK_ON, SHORT_BLINK_OFF, 0 );	

	/* send transition msg to shelf controler */
	msg.command = IPMI_SE_PLATFORM_EVENT;
	msg.evt_msg_rev = IPMI_EVENT_MESSAGE_REVISION;
	msg.sensor_type = IPMI_SENSOR_HOT_SWAP;
	msg.sensor_number = 0;
	msg.evt_direction = IPMI_EVENT_TYPE_GENERIC_AVAILABILITY;
	msg.evt_data1 = 0;		/* Event Data 1
					   [7:4] = Ah (OEM code in Event Data 2,
					   OEM code in Event Data 3)
					   [3:0] = Current State  */
	msg.evt_data2 = 0;		/* Event Data 2
					   [7:4] = Cause of state change. 
					   See Table 3-20, “Cause of State Change values,” 
					   for values.
					   [3:0] = Previous State */
	msg.evt_data3 = 0;		/* Event Data 3
					   [7:0] = FRU Device ID */
	msg.evt_data1 = 0xa << 4 | FRU_STATE_M5_DEACTIVATION_REQUEST;
	msg.evt_data2 = STATE_CH_NORMAL << 4 | FRU_STATE_M4_ACTIVE;
	msg.evt_data3 = controller_fru_dev_id;

	/* dispatch message */
	ipmi_send_event_req( ( unsigned char * )&msg, sizeof( FRU_HOT_SWAP_EVENT_MSG_REQ ), 0 );

}

void
picmg_m6_state( unsigned fru_id )
{
	FRU_HOT_SWAP_EVENT_MSG_REQ msg;

	dprintf( DBG_IPMI | DBG_INOUT, "picmg_m5_state: ingress\n" );
	
	/* set state */
	fru[fru_id].state = FRU_STATE_M6_DEACTIVATION_IN_PROGRESS;
	
	/* architecture dependent - turn payload power off */

	/* turn blue LED on */
	gpio_led_on( GPIO_FRU_LED_BLUE );	

	/* send transition msg to shelf controler */
	msg.command = IPMI_SE_PLATFORM_EVENT;
	msg.evt_msg_rev = IPMI_EVENT_MESSAGE_REVISION;
	msg.sensor_type = IPMI_SENSOR_HOT_SWAP;
	msg.sensor_number = 0;
	msg.evt_direction = IPMI_EVENT_TYPE_GENERIC_AVAILABILITY;
	msg.evt_data1 = 0;		/* Event Data 1
					   [7:4] = Ah (OEM code in Event Data 2,
					   OEM code in Event Data 3)
					   [3:0] = Current State  */
	msg.evt_data2 = 0;		/* Event Data 2
					   [7:4] = Cause of state change. 
					   See Table 3-20, “Cause of State Change values,” 
					   for values.
					   [3:0] = Previous State */
	msg.evt_data3 = 0;		/* Event Data 3
					   [7:0] = FRU Device ID */
	msg.evt_data1 = 0xa << 4 | FRU_STATE_M6_DEACTIVATION_IN_PROGRESS;
	msg.evt_data2 = STATE_CH_NORMAL << 4 | FRU_STATE_M5_DEACTIVATION_REQUEST;
	msg.evt_data3 = controller_fru_dev_id;

	/* dispatch message */
	ipmi_send_event_req( ( unsigned  char * )&msg, sizeof( FRU_HOT_SWAP_EVENT_MSG_REQ ), 0 );

	picmg_m1_state( fru_id );
}

void
picmg_set_power_level( IPMI_PKT *pkt )
{
	SET_POWER_LEVEL_CMD_REQ		*req = ( SET_POWER_LEVEL_CMD_REQ * )pkt->req;
	SET_POWER_LEVEL_CMD_RESP	*resp = ( SET_POWER_LEVEL_CMD_RESP * )pkt->resp;
	unsigned		power_level;
	
	dprintf( DBG_IPMI | DBG_INOUT, "picmg_set_power_level: ingress\n" );

	resp->completion_code = CC_NORMAL;
	resp->picmg_id = PICMG_ID;
	pkt->hdr.resp_data_len = sizeof( SET_POWER_LEVEL_CMD_RESP ) - 1;

	if ( req->fru_dev_id < MAX_FRU_DEV_ID + 1 ) {
		
		switch( req->power_level ) {
			case 0:
				/* arch dependent - power off */
				break;
			case 0xff:
				/* do not change power level */
				break;
			default:
				if( req->set_present_level == 1 )
					fru[req->fru_dev_id].power_level_steady_state = req->power_level;
				
				/* if activation in progress, change state to M4 */
				if( fru[req->fru_dev_id].state == FRU_STATE_M4_ACTIVE ) {
					picmg_m4_state( req->fru_dev_id );
				} else {
					/* architecture dependent - adjust power level */
					// SET_POWER(fru[req->fru_dev_id].power_level_steady_state)
				}
				
				break;
		}
	} else {
		pkt->hdr.resp_data_len = CC_PARAM_OUT_OF_RANGE;
		pkt->hdr.resp_data_len = 0;
	}
}

/*
The “Set FRU Activation Policy” command modifies the way a FRU’s operational state transitions
behave. The policy bits indicate whether the FRU is Locked or not and whether the FRU is
Deactivation-Locked or not. Conceptually, the Locked bit is like a software equivalent of the
Handle Switch. Similar to the situation with the hardware Handle Switch, the FRU cannot proceed
from state M1 to M2 if the Locked bit has a value 1b. The Deactivation-Locked bit indicates
whether the Extraction Criteria Met condition exists. The FRU can only proceed from M4 to M5 if
the Deactivation-Locked bit is cleared.
*/

void
picmg_set_fru_activation_policy( IPMI_PKT *pkt )
{
	SET_FRU_ACTIVATION_POLICY_CMD_REQ	*req = ( SET_FRU_ACTIVATION_POLICY_CMD_REQ * )pkt->req;
	SET_FRU_ACTIVATION_POLICY_CMD_RESP	*resp = ( SET_FRU_ACTIVATION_POLICY_CMD_RESP * )pkt->resp;

	dprintf( DBG_IPMI | DBG_INOUT, "picmg_set_fru_activation_policy: ingress\n" );

	resp->completion_code = CC_NORMAL;
	resp->picmg_id = PICMG_ID;
	pkt->hdr.resp_data_len = sizeof( SET_FRU_ACTIVATION_POLICY_CMD_RESP ) - 1;

	if ( req->fru_dev_id < MAX_FRU_DEV_ID + 1 ) {
		if( req->act_policy_mask & FRU_ACTIVATION_POLICY_LOCK )
			fru[req->fru_dev_id].locked |= req->act_policy_set & FRU_ACTIVATION_POLICY_LOCK;
		else if( req->act_policy_mask & FRU_ACTIVATION_POLICY_DEACTIVATION_LOCK )
			fru[req->fru_dev_id].deactivation_locked |= req->act_policy_set & FRU_ACTIVATION_POLICY_DEACTIVATION_LOCK;
		resp->completion_code = CC_NORMAL;
	} else {
		resp->completion_code = CC_PARAM_OUT_OF_RANGE;
		pkt->hdr.resp_data_len = 0;
	}	
}

void
picmg_get_fru_activation_policy( IPMI_PKT *pkt )
{
	GET_FRU_ACTIVATION_POLICY_CMD_REQ	*req = ( GET_FRU_ACTIVATION_POLICY_CMD_REQ * )pkt->req;
	GET_FRU_ACTIVATION_POLICY_CMD_RESP	*resp = ( GET_FRU_ACTIVATION_POLICY_CMD_RESP *)pkt->resp;

	dprintf( DBG_IPMI | DBG_INOUT, "picmg_get_fru_activation_policy: ingress\n" );

	resp->completion_code = CC_NORMAL;
	resp->picmg_id = PICMG_ID;
	pkt->hdr.resp_data_len = sizeof( GET_FRU_ACTIVATION_POLICY_CMD_RESP ) - 1;

	if ( req->fru_dev_id < MAX_FRU_DEV_ID + 1 ) {
		resp->deactivation_locked = fru[req->fru_dev_id].deactivation_locked;
		resp->locked = fru[req->fru_dev_id].locked;
		resp->completion_code = CC_NORMAL;
	} else {
		resp->completion_code = CC_PARAM_OUT_OF_RANGE;
		pkt->hdr.resp_data_len = 0;
	}	
}


void
picmg_fru_control( IPMI_PKT *pkt )
{
	FRU_CONTROL_CMD_REQ	*req = ( FRU_CONTROL_CMD_REQ * )pkt->req;
	FRU_CONTROL_CMD_RESP	*resp = ( FRU_CONTROL_CMD_RESP * )pkt->resp;

	dprintf( DBG_IPMI | DBG_INOUT, "picmg_fru_control: ingress\n" );

	resp->completion_code = CC_NORMAL;
	resp->picmg_id = PICMG_ID;
	pkt->hdr.resp_data_len = sizeof( FRU_CONTROL_CMD_RESP ) - 1;

	if ( req->fru_dev_id < MAX_FRU_DEV_ID + 1 ) {
		switch( req->fru_control_options ) {
			case FRU_CONTROL_COLD_RESET:  /* add architecture dependent code here */
				module_cold_reset( req->fru_dev_id );
				break;
			case FRU_CONTROL_WARM_RESET:
				module_warm_reset( req->fru_dev_id );
				break;
			case FRU_CONTROL_GRACEFUL_REBOOT:
				module_graceful_reboot( req->fru_dev_id );
				break;
			case FRU_CONTROL_ISSUE_DIAG_INT:
				module_issue_diag_int( req->fru_dev_id );
				break;
			case FRU_CONTROL_QUIESCE:
				module_quiesce( req->fru_dev_id );
				break;
			default:
				resp->completion_code = CC_PARAM_OUT_OF_RANGE;
				pkt->hdr.resp_data_len = 0;
				break;
		}
	} else {
		resp->completion_code = CC_PARAM_OUT_OF_RANGE;
		pkt->hdr.resp_data_len = 0;
	}
}

void
picmg_get_fru_led_properties( IPMI_PKT *pkt )
{
	GET_LED_PROPERTIES_CMD_REQ	*req = ( GET_LED_PROPERTIES_CMD_REQ * )pkt->req;
	GET_LED_PROPERTIES_CMD_RESP	*resp = ( GET_LED_PROPERTIES_CMD_RESP * )pkt->resp;

	dprintf( DBG_IPMI | DBG_INOUT, "picmg_get_fru_led_properties: ingress\n" );

	resp->completion_code = CC_NORMAL;
	resp->picmg_id = PICMG_ID;
	pkt->hdr.resp_data_len = sizeof( GET_LED_PROPERTIES_CMD_RESP ) - 1;
	
	if ( req->fru_dev_id < MAX_FRU_DEV_ID + 1 ) {
		resp->gen_status_led_prop = fru[req->fru_dev_id].gen_status_led_prop;
		resp->app_spec_led_count = fru[req->fru_dev_id].app_spec_led_count;
	} else {
		resp->completion_code = CC_PARAM_OUT_OF_RANGE;
		pkt->hdr.resp_data_len = 0;
	}
}

void
picmg_get_led_color_capabilities( IPMI_PKT *pkt )
{
	GET_LED_COLOR_CAPABILITIES_CMD_REQ	*req = ( GET_LED_COLOR_CAPABILITIES_CMD_REQ * )pkt->req;
	GET_COLOR_CAPABILITIES_CMD_RESP		*resp = ( GET_COLOR_CAPABILITIES_CMD_RESP * )pkt->resp;

	dprintf( DBG_IPMI | DBG_INOUT, "picmg_get_led_color_capabilities: ingress\n" );
	
	resp->completion_code = CC_NORMAL;
	resp->picmg_id = PICMG_ID;
	pkt->hdr.resp_data_len = sizeof( GET_COLOR_CAPABILITIES_CMD_RESP ) - 1;

	if ( req->fru_dev_id < MAX_FRU_DEV_ID + 1 ) {
		resp->led_color_capabilities = fru[req->fru_dev_id].led_capabilities[req->led_id].led_color_capabilities;
		resp->default_led_color	= fru[req->fru_dev_id].led_capabilities[req->led_id].default_led_color;
		resp->default_led_color_override = fru[req->fru_dev_id].led_capabilities[req->led_id].default_led_color_override;
	} else {
		resp->completion_code = CC_PARAM_OUT_OF_RANGE;
		pkt->hdr.resp_data_len = 0;
	}
}

/*
This command is sent by the Shelf Manager to the IPM Controller to inform the
device that it should “lock” in its desired power and cooling levels.
Another step the IPM Controller takes when it receives the Compute Power Properties
is that it prepares for the receipt of the “Get Power Level” command.
*/
void
picmg_compute_power_properties( IPMI_PKT *pkt )
{
	COMPUTE_POWER_PROPERTIES_CMD_REQ	*req = ( COMPUTE_POWER_PROPERTIES_CMD_REQ *)pkt->req;
	COMPUTE_POWER_PROPERTIES_CMD_RESP	*resp = ( COMPUTE_POWER_PROPERTIES_CMD_RESP *)pkt->resp;

	dprintf( DBG_IPMI | DBG_INOUT, "picmg_compute_power_properties: ingress\n" );

	resp->completion_code = CC_NORMAL;
	resp->picmg_id = PICMG_ID;
	pkt->hdr.resp_data_len = sizeof( COMPUTE_POWER_PROPERTIES_CMD_RESP ) - 1;
	
	if ( req->fru_dev_id < MAX_FRU_DEV_ID + 1 ) {
		resp->num_slots = 1;
		resp->ipm_location = 0;
		// Compute power properties - Implementation dependent 
	} else {
		resp->completion_code = CC_PARAM_OUT_OF_RANGE;
		pkt->hdr.resp_data_len = 0;
	}
}

void
picmg_get_power_level( IPMI_PKT *pkt )
{
	GET_POWER_LEVEL_CMD_REQ		*req = ( GET_POWER_LEVEL_CMD_REQ * )pkt->req;
	GET_POWER_LEVEL_CMD_RESP	*resp = ( GET_POWER_LEVEL_CMD_RESP * )pkt->resp;

	dprintf( DBG_IPMI | DBG_INOUT, "picmg_get_power_level: ingress\n" );

	resp->picmg_id = PICMG_ID;
	resp->completion_code = CC_NORMAL;
	pkt->hdr.resp_data_len = sizeof( GET_POWER_LEVEL_CMD_RESP ) - 1;

 	if ( req->fru_dev_id > MAX_FRU_DEV_ID  ) {
		resp->completion_code = CC_PARAM_OUT_OF_RANGE;
		pkt->hdr.resp_data_len = 0;
		return;
	}

	/* Power Type.
	00h = Steady state power draw levels
	01h = Desired steady state draw levels
	02h = Early power draw levels
	03h = Desired early levels
	All other values reserved.
	*/

	/* Dynamic Power Configuration. Set to 1b if the FRU supports dynamic 
	   reconfiguration of power (i.e., the Payload service is uninterrupted 
	   when power levels are altered). */
	resp->dyn_power_config = fru[req->fru_dev_id].dyn_power_config;

	// The following fields are Implementation dependent and should be filled accordingly

	/* Power Level. When requesting “Steady state power draw levels”, 
	   this represents the power level of the FRU. When requesting desired power 
	   levels, this represents the power level the FRU would like to have. */
	// resp->power_level = fru[req->fru_dev_id].power_level[req->power_type];

	/* Delay to Stable Power. This byte shall be written as 00h when Power
	   Type is “Steady state power draw levels” or “Desired steady state draw levels”. 
	   Otherwise, this byte shall contain the amount of time before power transitions
	   from the early power levels to the normal levels. This value is returned in tenths
	   of a second. */
	resp->delay_to_stable_power = fru[req->fru_dev_id].delay_to_stable_power;
	
	/* Power Multiplier. This defines the number of tenths of a Watt by 
	   which to multiply all values held in bytes 6 and beyond. This is
	   included to allow a FRU that spans multiple locations to specify
	   higher power draws. For instance, if this byte holds a 50, then bytes 6
	   and beyond specify 5 W increments. */
	resp->power_multiplier = fru[req->fru_dev_id].power_multiplier;
	
	/* Power Draw[1..N]. The first entry reflects the lowest level of power
	   (minimum power level) used by the FRU’s Payload.  Power Draw[Max]. 
	   The last entry reflects the highest level of power used by the FRU’s
	   Payload. Everything is powered full capacity. Any bytes past the 6th
	   byte are optional. The maximum value of N is 25 (which corresponds 
	   to a Max value of 20) due to IPMI message size restrictions. */
	/* resp->power_draw[20]; */
	
}

void
picmg_get_shelf_address_info( IPMI_PKT *pkt )
{
	dprintf( DBG_IPMI | DBG_INOUT, "picmg_get_shelf_address_info: ingress\n" );
	// Implement for the shelf manager

}

void
picmg_set_shelf_address_info( IPMI_PKT *pkt )
{
	dprintf( DBG_IPMI | DBG_INOUT, "picmg_set_shelf_address_info: ingress\n" );
	// Implement for the shelf manager

}

void
picmg_set_fru_led_state( IPMI_PKT *pkt )
{
	SET_FRU_LED_STATE_CMD_REQ	*req = ( SET_FRU_LED_STATE_CMD_REQ * )pkt->req;
	SET_FRU_LED_STATE_CMD_RESP	*resp = ( SET_FRU_LED_STATE_CMD_RESP * )pkt->resp;
	uchar led_mask = 0;

	dprintf( DBG_IPMI | DBG_INOUT, "picmg_set_fru_led_state: ingress\n" );

	resp->picmg_id = PICMG_ID;
	resp->completion_code = CC_NORMAL;
	pkt->hdr.resp_data_len = sizeof( SET_FRU_LED_STATE_CMD_RESP ) - 1;

	if( ( req->fru_dev_id > MAX_FRU_DEV_ID ) || 
			( ( req->led_id > 3) && ( req->led_id != 0xff ) ) ) {
		resp->completion_code = CC_PARAM_OUT_OF_RANGE;
		pkt->hdr.resp_data_len = 0;
		return;
	}
	
	/* LED ID assignments (as defined in Section 2.2.8, “LEDs” */
	/* 
	 * LED_BLUE		0x00
	 * LED1			0x01
	 * LED2			0x02
	 * LED3			0x03
	 * LED_TEST_ALL		0xff	Lamp Test (All LEDs under 
	 * 				management control are addressed)
	 */
	if( LED_TEST_ALL == req->led_id ) {
		led_mask = GPIO_FRU_LED_BLUE | GPIO_FRU_LED1 | GPIO_FRU_LED2 | GPIO_FRU_LED3;

		fru[req->fru_dev_id].led_state[FRU_LED_BLUE].override_led_state_func = 
			fru[req->fru_dev_id].led_state[FRU_LED1].override_led_state_func =
			fru[req->fru_dev_id].led_state[FRU_LED2].override_led_state_func =
			fru[req->fru_dev_id].led_state[FRU_LED3].override_led_state_func = req->led_function;
		fru[req->fru_dev_id].led_state[FRU_LED_BLUE].override_state_on_duration = 
			fru[req->fru_dev_id].led_state[FRU_LED1].override_state_on_duration = 
			fru[req->fru_dev_id].led_state[FRU_LED2].override_state_on_duration =
			fru[req->fru_dev_id].led_state[FRU_LED3].override_state_on_duration = req->on_duration;
		fru[req->fru_dev_id].led_state[FRU_LED_BLUE].override_state_color =
			fru[req->fru_dev_id].led_state[FRU_LED1].override_state_color =
			fru[req->fru_dev_id].led_state[FRU_LED2].override_state_color =
			fru[req->fru_dev_id].led_state[FRU_LED3].override_state_color = req->color;
	} else {
		switch ( req->led_id ) {
			case FRU_LED_BLUE:
				led_mask = GPIO_FRU_LED_BLUE;
				break;
			case FRU_LED1:
				led_mask = GPIO_FRU_LED1;
				break;
			case FRU_LED2:
				led_mask = GPIO_FRU_LED2;
				break;
			case FRU_LED3:
				led_mask = GPIO_FRU_LED3;
				break;

			default:
				resp->completion_code = CC_PARAM_OUT_OF_RANGE;
				pkt->hdr.resp_data_len = 0;
				return;
		}

		fru[req->fru_dev_id].led_state[req->led_id].override_led_state_func = req->led_function;
		fru[req->fru_dev_id].led_state[req->led_id].override_state_on_duration = req->on_duration;
		fru[req->fru_dev_id].led_state[req->led_id].override_state_color = req->color;
	}


	switch( req->led_function ) {
		case 0x00:
			/* 00h = LED off override */
			gpio_led_off( led_mask );
			break;
		case 0xFB:
			/* FBh = LAMP TEST state. Turn on LED(s) specified in byte
			 * 3 for duration specified in byte 5 (in hundreds of 
			 * milliseconds) then return to the highest priority state. */
			gpio_led_blink( led_mask, req->on_duration/10, req->led_function/10, 1 ); 
			break;
		case 0xFC:
			/* FCh = LED state restored to Local Control state, the FRU_LED_STATE struct
			 * should have sufficient information to revert to the local control state */
			if( fru[req->fru_dev_id].led_state[req->led_id].led_states & 0x1 ) {
				/* if IPM Controller has a Local Control state */
				if( fru[req->fru_dev_id].led_state[req->led_id].local_control_led_func == 0 ) {
					/* LED is off */
					gpio_led_off( led_mask );
				} else if( fru[req->fru_dev_id].led_state[req->led_id].local_control_led_func == 0xff ) {
					/* LED is on */
					gpio_led_on( led_mask );
				} else {
					/* LED is blinking */
					gpio_led_blink( led_mask, 
						fru[req->fru_dev_id].led_state[req->led_id].local_control_on_duration/10,
						fru[req->fru_dev_id].led_state[req->led_id].local_control_led_func/10, 0 ); 
				}
			}
			break;
		case 0xFD:
		case 0xFE:
			/* FDh-FEh Reserved */
			break;
		case 0xFF:
			/* FFh = LED on override */ 
			gpio_led_on( led_mask );
			break;
		default:
			/* 01h - FAh = LED BLINKING override. The off duration
			 * is specified by the value of this byte and the on
			 * duration is specified by the value of byte 5. Both
			 * values specify the time in tens of milliseconds
			 * (10 ms –2.5 s) */
			gpio_led_blink( led_mask, req->on_duration/10, req->led_function/10, 0 ); 
			break;
	}
			
	/* On-duration: LED on-time in tens of milliseconds if (1 = Byte 4 = FAh)
	   Lamp Test time in hundreds of milliseconds if (Byte 4 = FBh. 
	   Lamp Test time value must be less than 128. Other values when Byte 4
	   = FBh are reserved. Otherwise, this field is ignored and shall be set to 0h. */

	/* Color when illuminated. This byte sets the override color when
	   LED Function is 01h–FAh and FFh. This byte sets the Local Control
	   color when LED Function is FCh. This byte may be ignored during 
	   Lamp Test or may be used to control the color during the lamp test when
	   LED Function is FBh. */

}

void
picmg_get_fru_led_state( IPMI_PKT *pkt )
{
	GET_FRU_LED_STATE_CMD_REQ *req = ( GET_FRU_LED_STATE_CMD_REQ * )pkt->req;
	GET_FRU_LED_STATE_CMD_RESP *resp = ( GET_FRU_LED_STATE_CMD_RESP * )pkt->resp;

	dprintf( DBG_IPMI | DBG_INOUT, "picmg_get_fru_led_state: ingress\n" );

	if( ( req->fru_dev_id > MAX_FRU_DEV_ID ) || ( req->led_id > 3 ) ) {
		resp->completion_code = CC_PARAM_OUT_OF_RANGE;
		pkt->hdr.resp_data_len = 0;
		return;
	}
	resp->completion_code = CC_NORMAL;	
	resp->picmg_id = PICMG_ID;
	resp->led_states = fru[req->fru_dev_id].led_state[req->led_id].led_states;
	resp->local_control_led_func = fru[req->fru_dev_id].led_state[req->led_id].local_control_led_func;
	resp->local_control_on_duration = fru[req->fru_dev_id].led_state[req->led_id].local_control_on_duration;
	resp->local_control_color = fru[req->fru_dev_id].led_state[req->led_id].local_control_color;
	resp->override_led_state_func = fru[req->fru_dev_id].led_state[req->led_id].override_led_state_func;
	resp->override_state_on_duration = fru[req->fru_dev_id].led_state[req->led_id].override_state_on_duration;
	resp->override_state_color = fru[req->fru_dev_id].led_state[req->led_id].override_state_color;
	resp->led_test_duration = fru[req->fru_dev_id].led_state[req->led_id].led_test_duration;
	pkt->hdr.resp_data_len = sizeof( GET_FRU_LED_STATE_CMD_RESP ) - 1;
}

void
picmg_set_ipmb_state( IPMI_PKT *pkt )
{
	SET_IPMB_STATE_CMD_REQ	*req = ( SET_IPMB_STATE_CMD_REQ * )pkt->req;
	SET_IPMB_STATE_CMD_RESP	*resp = ( SET_IPMB_STATE_CMD_RESP * )pkt->resp;

	dprintf( DBG_IPMI | DBG_INOUT, "picmg_set_ipmb_state: ingress\n" );

	if( req->ipmb_a_state )
		i2c_interface_enable_local_control( 0, req->ipmb_a_link_id );
	else
		i2c_interface_disable( 0, req->ipmb_a_link_id );

	if( req->ipmb_b_state )
		i2c_interface_enable_local_control( 1, req->ipmb_b_link_id );
	else
		i2c_interface_disable( 1, req->ipmb_b_link_id );

	resp->picmg_id = PICMG_ID;
	resp->completion_code = CC_NORMAL;
	pkt->hdr.resp_data_len = sizeof( SET_IPMB_STATE_CMD_RESP ) - 1;
}

void
picmg_get_device_locator_rec_id( IPMI_PKT *pkt )
{
	GET_DEVICE_LOCATOR_RECORD_ID_CMD_REQ	*req = ( GET_DEVICE_LOCATOR_RECORD_ID_CMD_REQ * )pkt->req;
	GET_DEVICE_LOCATOR_RECORD_ID_CMD_RESP	*resp = ( GET_DEVICE_LOCATOR_RECORD_ID_CMD_RESP * )pkt->resp;

	dprintf( DBG_IPMI | DBG_INOUT, "picmg_get_device_locator_rec_id: ingress\n" );

	if ( req->fru_dev_id > MAX_FRU_DEV_ID ) {
		resp->completion_code = CC_PARAM_OUT_OF_RANGE;
		pkt->hdr.resp_data_len = 0;
		return;
	}

	resp->record_id_LSB = fru[ req->fru_dev_id ].record_id_LSB;
	resp->record_id_MSB = fru[ req->fru_dev_id ].record_id_MSB;
	
	resp->picmg_id = PICMG_ID;
	resp->completion_code = CC_NORMAL;
	pkt->hdr.resp_data_len = sizeof( GET_DEVICE_LOCATOR_RECORD_ID_CMD_RESP ) - 1;
}

void
picmg_set_port_state( IPMI_PKT *pkt )
{
	SET_PORT_STATE_CMD_REQ	*req = ( SET_PORT_STATE_CMD_REQ * )pkt->req;
	PICMG_CMD_RESP	*resp = ( PICMG_CMD_RESP * )pkt->resp;
	int	i, found = 0;

	dprintf( DBG_IPMI | DBG_INOUT, "picmg_set_port_state: ingress\n" );

	resp->picmg_id = PICMG_ID;

	// search for an entry that matches interface & channel_number
	for( i = 0; i < NUM_LINK_INFO_ENTRIES; i++ ) {
		if( link_info_table[i].entry_in_use 
		  && ( link_info_table[i].link_info.interface == req->link_info.interface ) 
		  && ( link_info_table[i].link_info.channel_number == req->link_info.channel_number ) ) {
			found++;
		} 	
	}
	
	if( !found ) {
		// use a free entry
		for( i = 0; i < NUM_LINK_INFO_ENTRIES; i++ ) {
			if( !link_info_table[i].entry_in_use ) {
			       found++;
			       link_info_table[i].entry_in_use = 1;
			}
		}
	}	
	
	if( found ) {
		link_info_table[i].link_info.link_grouping_id = req->link_info.link_grouping_id;
		link_info_table[i].link_info.link_type_extension = req->link_info.link_type_extension;
		link_info_table[i].link_info.link_type = req->link_info.link_type;
		link_info_table[i].link_info.port3_bit_flag = req->link_info.port3_bit_flag;
		link_info_table[i].link_info.port2_bit_flag = req->link_info.port2_bit_flag;
		link_info_table[i].link_info.port1_bit_flag = req->link_info.port1_bit_flag;
		link_info_table[i].link_info.port0_bit_flag = req->link_info.port0_bit_flag;
		link_info_table[i].state = req->state;
		resp->completion_code = CC_NORMAL;
		pkt->hdr.resp_data_len = sizeof( PICMG_CMD_RESP ) - 1;
	} else {
		// did not find an entry and no free entries in link_info_table 
		resp->completion_code = CC_PARAM_OUT_OF_RANGE;
		pkt->hdr.resp_data_len = 0;
	}		
}

void
picmg_get_port_state( IPMI_PKT *pkt )
{
	GET_PORT_STATE_CMD_REQ	*req = ( GET_PORT_STATE_CMD_REQ * )pkt->req;
	GET_PORT_STATE_CMD_RESP	*resp = ( GET_PORT_STATE_CMD_RESP * )pkt->resp;
	int	i, found = 0;
	
	dprintf( DBG_IPMI | DBG_INOUT, "picmg_get_port_state: ingress\n" );

	resp->picmg_id = PICMG_ID;
	
	// search for an entry that matches interface & channel_number
	for( i = 0; i < NUM_LINK_INFO_ENTRIES; i++ ) {
		if( link_info_table[i].entry_in_use 
		  && ( link_info_table[i].link_info.interface == req->interface ) 
		  && ( link_info_table[i].link_info.channel_number == req->channel_number ) ) {
			found++;
		} 	
	}

	if( found ) {
		resp->link_info1 = link_info_table[i].link_info;
		resp->state1 = link_info_table[i].state;
		resp->completion_code = CC_NORMAL;
		pkt->hdr.resp_data_len = sizeof( GET_PORT_STATE_CMD_RESP ) - 1;

	} else {
		resp->completion_code = CC_PARAM_OUT_OF_RANGE;
		pkt->hdr.resp_data_len = 0;
	}
}

void
picmg_renegotiate_power( IPMI_PKT *pkt )
{
	dprintf( DBG_IPMI | DBG_INOUT, "picmg_renegotiate_power: ingress\n" );
	// Implement for the shelf manager
}

void
picmg_get_fan_speed_properties( IPMI_PKT *pkt )
{
	GET_FAN_SPEED_PROPERTIES_CMD_REQ	*req = ( GET_FAN_SPEED_PROPERTIES_CMD_REQ * )pkt->req;
	GET_FAN_SPEED_PROPERTIES_CMD_RESP	*resp = ( GET_FAN_SPEED_PROPERTIES_CMD_RESP * )pkt->resp;

	dprintf( DBG_IPMI | DBG_INOUT, "picmg_get_fan_speed_properties: ingress\n" );

	if ( req->fru_dev_id < MAX_FRU_DEV_ID + 1 ) {
		/* Minimum Speed Level. This field describes the minimum setting that
		   is accepted by the Set Fan Level command. */
		resp->min_speed_level = fru_fan[req->fru_dev_id].min_speed_level;
		
		/* Maximum Speed Level. This field describes the maximum setting that
		   is accepted by the Set Fan Level command. */	
		resp->max_speed_level = fru_fan[req->fru_dev_id].max_speed_level;
		
		/* Normal Operating Level. This field represents the default 
		   normal fan speed recommended by the fan manufacturer. */
		resp->norm_operating_level = fru_fan[req->fru_dev_id].norm_operating_level;
		
		/* Fan Tray Properties. This field holds properties of the Fan Tray.
		   [Bit 7] – Local Control Mode Supported.  This bit is set to 1b if the Fan
		   Tray supports automatic adjustment of the fan speed.
		   [Bits 6:0] – Reserved. */
		resp->fan_tray_prop = fru_fan[req->fru_dev_id].fan_tray_prop;
	} else {
		resp->completion_code = CC_PARAM_OUT_OF_RANGE;
		pkt->hdr.resp_data_len = 0;
		return;
	}

	resp->picmg_id = PICMG_ID;
	resp->completion_code = CC_NORMAL;
	pkt->hdr.resp_data_len = sizeof( GET_FAN_SPEED_PROPERTIES_CMD_RESP ) - 1;
}

void
picmg_set_fan_level( IPMI_PKT *pkt )
{
	SET_FAN_LEVEL_CMD_REQ	*req = ( SET_FAN_LEVEL_CMD_REQ * )pkt->req;
	SET_FAN_LEVEL_CMD_RESP	*resp = ( SET_FAN_LEVEL_CMD_RESP * )pkt->resp;

	dprintf( DBG_IPMI | DBG_INOUT, "picmg_set_fan_level: ingress\n" );

	/* For Fan Level to be accepted, its value shall be: 
	   1) greater than or equal to Minimum Speed Level and
	   less than or equal to Maximum Speed Level, or 
	   2) FEh (Emergency Shut Down) or 3) FFh (Local Control). */
	if ( ( req->fru_dev_id < MAX_FRU_DEV_ID + 1 ) && 
			( (req->fan_level >= fru_fan[req->fru_dev_id].min_speed_level ) || 
			  (req->fan_level <= fru_fan[req->fru_dev_id].max_speed_level ) ) ) {
		switch (req->fan_level ) {
			case 0xFE:
				/* emergency shut down */
				fan_shutdown( req->fru_dev_id );
				break;
			case 0xFF:
				/* local control */
				fru_fan[req->fru_dev_id].fan_control = FAN_CONTROL_LOCAL;
				fan_set_speed( req->fru_dev_id, fru_fan[req->fru_dev_id].local_control_fan_level );
				break;
			default:
				fru_fan[req->fru_dev_id].override_fan_level = req->fan_level;
				fru_fan[req->fru_dev_id].fan_control = FAN_CONTROL_OVERRIDE;
				fan_set_speed( req->fru_dev_id, req->fan_level );
				break;
		}
	} else {
		resp->completion_code = CC_PARAM_OUT_OF_RANGE;
		pkt->hdr.resp_data_len = 0;
		return;
	}

	resp->picmg_id = PICMG_ID;
	resp->completion_code = CC_NORMAL;
	pkt->hdr.resp_data_len = sizeof( SET_FAN_LEVEL_CMD_RESP ) - 1;
}

void
picmg_get_fan_level( IPMI_PKT *pkt )
{
	GET_FAN_LEVEL_CMD_REQ	*req = ( GET_FAN_LEVEL_CMD_REQ * )pkt->req;
	GET_FAN_LEVEL_CMD_RESP	*resp = ( GET_FAN_LEVEL_CMD_RESP * )pkt->resp;

	dprintf( DBG_IPMI | DBG_INOUT, "picmg_get_fan_level: ingress\n" );

	if( req->fru_dev_id < MAX_FRU_DEV_ID + 1 ) {
		/* Override Fan Level indicates the fan level that the Shelf Manager
		   has selected, which must be in the range Minimum Speed Level to
		   Maximum Speed Level, or equal to FEh or FFh.
		   FEh = Fan has been placed in “Emergency Shut Down” by the 
		   Shelf Manager 
		   FFh = Fan operating in Local Control mode */
		switch( fru_fan[req->fru_dev_id].fan_control ) {
			case FAN_CONTROL_LOCAL:
				resp->override_fan_level = 0xFF;
				break;
			case FAN_CONTROL_OVERRIDE:
				resp->override_fan_level = fru_fan[req->fru_dev_id].override_fan_level;
				break;
			case FAN_CONTROL_SHUTDOWN:	
				resp->override_fan_level = 0xFE;
				break;
		}
		
		/* Local Control Fan Level - This byte is optional if the Fan Tray does
		   not support Local Control. When present, this byte always indicates
		   the Local Control fan level as determined by the Fan Tray controller.
		   When Local Control is supported, the actual fan level is: 
		   1) the value of this byte if Override Fan Level is FFh, or
		   2) the larger of this byte and Override Fan Level, or 
		   3) Emergency Shut Down if either Override Fan Level or this byte 
		      has a value FEh. */
		resp->local_control_fan_level = fru_fan[req->fru_dev_id].local_control_fan_level;
	} else {
		resp->completion_code = CC_PARAM_OUT_OF_RANGE;
		pkt->hdr.resp_data_len = 0;
		return;
	}
	
	resp->picmg_id = PICMG_ID;
	resp->completion_code = CC_NORMAL;
	pkt->hdr.resp_data_len = sizeof( GET_FAN_LEVEL_CMD_RESP ) - 1;
}

/*
This command is used between Boards and the Shelf Manager to implement the transfer of control
over a bused resource to and from a Board. A single command is used for all the bused E-Keying
operations on all the bus types. The command values have different interpretations depending on
whether the command is being sent from the Shelf Manager to a Board, or from a Board to the
Shelf Manager.
A Board wishing to use a bus sends a “Bused Resource Control (Request)” command to the Shelf
Manager. If the Shelf Manager does not wish to grant the bus to the Board (e.g., for security
reasons), it sends a Deny Status response to the Board. If the bus is available, the Shelf Manager
returns a Status of Grant.

*/
void
picmg_bused_resource_control( IPMI_PKT *pkt )
{
	dprintf( DBG_IPMI | DBG_INOUT, "picmg_bused_resource: ingress\n" );
	// Implement for the shelf manager
}

/*
The “Get IPMB Link Info” command provides the mapping between the IPMB-0
Link Number and the IPMB-0 Sensor Number. When provided an IPMB-0 Link Number, the
command returns the IPMB-0 Sensor Number that is monitoring the associated IPMB-0 Link.
Conversely, when provided the IPMB-0 Sensor Number, the command returns the associated
IPMB-0 Link Number that is being monitored.
*/
void
picmg_get_ipmb_link_info( IPMI_PKT *pkt )
{
	GET_IPMB_LINK_INFO_CMD_REQ	*req = ( GET_IPMB_LINK_INFO_CMD_REQ * )pkt->req;
	GET_IPMB_LINK_INFO_CMD_RESP	*resp = ( GET_IPMB_LINK_INFO_CMD_RESP * )pkt->resp;
	short i, found = 0;

	dprintf( DBG_IPMI | DBG_INOUT, "picmg_get_ipmb_link_info: ingress\n" );

	if( req->qualifier == IPMB_LINK_INFO_QUAL_LINK_NUMBER ) {
		/* req->link_info_key contains an IPMB-0 Link Number, return the Sensor Number */
		resp->link_number = req->link_info_key;
		for( i = 0; i < NUM_IPMB_SENSORS; i++ ) {
			if( ipmb_sensor[i].link_number == resp->link_number ) {
				resp->sensor_number = ipmb_sensor[i].sensor_number;
				found++;
			}
		}
	} else {
		/* req->link_info_key contains an IPMB-0 Sensor Number, return the Link Number */
		resp->sensor_number = req->link_info_key;
		for( i = 0; i < NUM_IPMB_SENSORS; i++ ) {
			if( ipmb_sensor[i].sensor_number == resp->sensor_number ) {
				resp->link_number = ipmb_sensor[i].link_number;
				found++;
			}
		}

	}
	resp->picmg_id = PICMG_ID;

	if( found ) {
		resp->completion_code = CC_NORMAL;
		pkt->hdr.resp_data_len = sizeof( GET_IPMB_LINK_INFO_CMD_RESP ) - 1;
	} else {
		resp->completion_code = CC_PARAM_OUT_OF_RANGE;
		pkt->hdr.resp_data_len = 0;
	}
}
#endif
