/*
-------------------------------------------------------------------------------
coreIPM/event.c

Author: Gokhan Sozmen
-------------------------------------------------------------------------------
Copyright (C) 2007-2008  Gokhan Sozmen
-------------------------------------------------------------------------------
coreIPM is free software; you can redistribute it and/or modify it under the 
terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later 
version.

coreIPM is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with 
coreIPM; if not, write to the Free Software Foundation, Inc., 51 Franklin
Street, Fifth Floor, Boston, MA  02110-1301, USA.
-------------------------------------------------------------------------------
See http://www.coreipm.com for documentation, latest information, license and
contact details.
-------------------------------------------------------------------------------
*/
#include "debug.h"
#include "ipmi.h"
#include "event.h"
#include "sensor.h"
#include "rtc.h"
#include "gpio.h"
#include "i2c.h"
#include "timer.h"
#include "ws.h"
#include "module.h"
#include <string.h>

#define PEF_PENDING_EVENT 0

EVENT_CONFIG evt_config;
EVENTS_PROCESSED evt_processed;

struct {
	uchar last_evt_rcvd;
	EVENT_LOG_ENTRY evt_msg[32];
} evt_log;

uchar pef_postpone_timer_handle;

void ipmi_event_handler( IPMI_PKT *pkt );
void ipmi_event_init( void );
void pef_postpone_timer_expired( unsigned char *arg );

/*======================================================================*/
/*======================================================================*/
/*			NETFN_EVENT commands
 *			
 * Relating to the configuration and transmission of Event Messages and 
 * system Sensors. 
 */
/*======================================================================*/
/*======================================================================*/

void ipmi_event_init( void )
{
	evt_log.last_evt_rcvd= 0xff;

#if defined (IPMC) || defined (MCMC)
	// set BMC as default event receiver, local i2c address will get routed directly
	evt_config.receiver_slave_addr = module_get_i2c_address( I2C_ADDRESS_LOCAL );
#else
	evt_config.receiver_slave_addr = module_get_i2c_address( I2C_ADDRESS_REMOTE );
#endif	
	evt_config.receiver_lun = 0;
	evt_config.evt_enabled = 1;
}
	
void
ipmi_process_event_req( IPMI_PKT *pkt )
{
	dputstr( DBG_IPMI | DBG_INOUT, "ipmi_process_event_req: ingress\n" );

	switch( pkt->req->command )
	{
		case IPMI_SE_CMD_GET_PEF_CAPABILITIES:
			ipmi_get_pef_capabilities( pkt );
			break;
			
		case IPMI_SE_CMD_ARM_PEF_POSTPONE_TIMER:
			ipmi_arm_pef_postpone_timer( pkt );
			break;
			
		case IPMI_SE_CMD_SET_PEF_CONFIG_PARAMS:
			ipmi_set_pef_config_params( pkt );
			break;
			
		case IPMI_SE_CMD_GET_PEF_CONFIG_PARAMS:
			ipmi_get_pef_config_params( pkt );
			break;
			
		case IPMI_SE_CMD_SET_LAST_PROCESSED_EVENT:
			ipmi_set_last_processed_event( pkt );
			break;
			
		case IPMI_SE_CMD_GET_LAST_PROCESSED_EVENT:
			ipmi_get_last_processed_event( pkt );
			break;
		
		case IPMI_SE_CMD_SET_EVENT_RECEIVER:
			ipmi_set_event_receiver( pkt );
			break;
			
		case IPMI_SE_CMD_GET_EVENT_RECEIVER:
			ipmi_get_event_receiver( pkt );
			break;
			
		case IPMI_SE_CMD_GET_DEVICE_SDR_INFO:
			ipmi_get_device_sdr_info( pkt );
			break;
			
		case IPMI_SE_CMD_GET_DEVICE_SDR:
			ipmi_get_device_sdr( pkt );
			break;
			
		case IPMI_SE_CMD_RSV_DEVICE_SDR_REPOSITORY:
			ipmi_reserve_device_sdr_repository( pkt );
			break;
			
		case IPMI_SE_CMD_GET_SENSOR_READING:
			ipmi_get_sensor_reading( pkt );
			break;

		case IPMI_SE_PLATFORM_EVENT:
			ipmi_platform_event( pkt );
			break;

		case IPMI_SE_CMD_GET_SENSOR_READING_FACTORS:
		case IPMI_SE_CMD_SET_SENSOR_HYSTERESIS:
		case IPMI_SE_CMD_GET_SENSOR_HYSTERESIS:
		case IPMI_SE_CMD_SET_SENSOR_THRESHOLD:
		case IPMI_SE_CMD_GET_SENSOR_THRESHOLD:
		case IPMI_SE_CMD_SET_SENSOR_EVENT_ENABLE:
		case IPMI_SE_CMD_GET_SENSOR_EVENT_ENABLE:
		case IPMI_SE_CMD_REARM_SENSOR_EVENTS:
		case IPMI_SE_CMD_GET_SENSOR_EVENT_STATUS:
		case IPMI_SE_CMD_SET_SENSOR_TYPE:
		case IPMI_SE_CMD_GET_SENSOR_TYPE:
		case IPMI_SE_CMD_ALERT_IMMEDIATE:
		case IPMI_SE_CMD_PET_ACKNOWLEDGE:
		default:
			pkt->resp->completion_code = CC_INVALID_CMD;
			pkt->hdr.resp_data_len = 0;
			break;
	}
}


/*======================================================================*/
/*
 *  PEF and Alerting Mandatory Commands
 *  	Get PEF Capabilities
 *  	Arm PEF Postpone Timer
 *  	Set PEF Configuration Parameters
 *  	Get PEF Configuration Parameters
 *  	Set Last Precessed Event ID
 *  	Get Last processed Event ID
 * 
 *  Using NETFN_EVENT_REQ/NETFN_EVENT_RESP
 */
/*======================================================================*/
/*
Platform Event Filtering (PEF) provides a mechanism for configuring the BMC
to taking selected actions on event messages that it receives or has internally
generated. These actions include operations such as system power-off, system 
reset, as well as triggering the generation of an alert.

The BMC maintains an event filter table that is used to select which events
trigger an action and which actions to perform. Each time the BMC receives
an event message (either externally or internally generated) it compares the
event data against the entries in the event filter table. The BMC scans all
entries in the table and collects a set of actions to be performed as 
determined by the entries that were matched.

A match occurs when there are event filter table matches (exact or wild-carded) 
for all compared fields in the event message.


Alert Policy Table
------------------
Platform Event Filtering supports alerting as one of the selectable actions that
can occur when an event matches an event filter table entry. The alerting media
and the different alert destinations that are tried are determined by the 
settings in the Alert Policy Table.

*/

/*
  we maintain the following data structures :
 - pef_config struct
 - pef_capabilities struct
 - an event filter table array
 - an alert strings array
 - an alert policy table array

*/
#define	PEF_EVT_FILTER_TABLE_ENTRIES	16
#define ALERT_STRING_ENTRIES		16
#define ALERT_POLICY_ENTRIES		16

struct pef_config {
	PEF_SET_IN_PROGRESS	progress;
	PEF_CONTROL		control;
	PEF_ACTION_GLOBAL_CONTROL	global_control;
	PEF_STARTUP_DELAY	startup_delay;
	PEF_ALERT_STARTUP_DELAY	alert_startup_delay;
	PEF_NUM_EVENT_FILTERS	num_filters;
//	PEF_NUM_ALERT_POLICY	num_alert_policy;
};

PEF_CAPABILITIES pef_capabilities = { 
	0x51,	/* PEF Version (BCD encoded, LSN first, 51h for this specification.
		 *  51h == version 1.5) */
	PEF_ACTION_DIAG_INTERRUPT | PEF_ACTION_OEM_ACTION | PEF_ACTION_POWER_CYCLE |
	PEF_ACTION_RESET | PEF_ACTION_POWER_DOWN | PEF_ACTION_ALERT,
	PEF_EVT_FILTER_TABLE_ENTRIES,
	0,	/* pef postpone timeout */
	0	/* config param data */
};

PEF_EVENT_FILTER_TABLE		pef_filter_table[PEF_EVT_FILTER_TABLE_ENTRIES];
PEF_ALERT_STRINGS		pef_alert_string_table[ALERT_STRING_ENTRIES];
PEF_ALERT_POLICY_TABLE_ENTRY	pef_alert_policy_table[ALERT_POLICY_ENTRIES];

/*
 * ipmi_get_pef_capabilities()
 *
 * This command returns the information about the implementation of PEF on
 * the BMC.
 */
void
ipmi_get_pef_capabilities( IPMI_PKT *pkt ) 
{
	GET_PEF_CAPABILITIES_CMD_RESP *resp = (GET_PEF_CAPABILITIES_CMD_RESP *)(pkt->resp);

	dputstr( DBG_IPMI | DBG_INOUT, "get_pef_capabilities: ingress\n" );
	
	resp->pef_version = pef_capabilities.pef_version;
	
	resp->action_support = pef_capabilities.action_support;
	
	/* Number of event filter table entries (1 based) */
	resp->num_evt_filter_tbl_entries = pef_capabilities.num_evt_filter_tbl_entries;

	resp->completion_code = CC_NORMAL;
	pkt->hdr.resp_data_len = 3;
}


/* This command is used by software to enable and arm the PEF Postpone Timer. 
 * The command can also be used by software to disable PEF indefinitely during
 * run-time. Once enabled, the timer automatically starts counting down whenever
 * the last software-processed event Record ID is for a record that is not equal
 * to the most recent (last) SEL record. The countdown will begin immediately
 * if the Record IDs are already different when the timer is armed.
 * In order to keep the PEF Postpone Timer from expiring, software must use the
 * Set Last Processed Event ID command to update the last software-processed 
 * Record ID to match the value for the last SEL record. This will cause the 
 * BMC to stop the timer and rearm it to start counting down from the value 
 * that was passed in the Arm PEF Postpone Timer command.
 */
void
ipmi_arm_pef_postpone_timer( IPMI_PKT *pkt ) 
{
	ARM_PEF_POSTPONE_TIMER_CMD_REQ 	*req = (ARM_PEF_POSTPONE_TIMER_CMD_REQ *)(pkt->req);
	ARM_PEF_POSTPONE_TIMER_CMD_RESP	*resp = (ARM_PEF_POSTPONE_TIMER_CMD_RESP *)(pkt->resp);

	dputstr( DBG_IPMI | DBG_INOUT, "arm_pef_postpone_timer: ingress\n" );

	resp->present_timer_countdown_value = 0;
	
	pef_capabilities.pef_postpone_timeout_value = req->pef_postpone_timeout;
	switch( pef_capabilities.pef_postpone_timeout_value ) {
		case 0x00:		/* 00h = disable Postpone Timer */
			timer_remove_callout_queue( &pef_postpone_timer_handle );
			break;
		case 0xfe:		/* FEh = Temporary PEF disable */
			timer_remove_callout_queue( &pef_postpone_timer_handle );
			break;
		case 0xff:		/* FFh = get present countdown value */
			resp->present_timer_countdown_value = 
				timer_get_expiration_time( &pef_postpone_timer_handle );
			break;
		default:		/* 01h - FDh = arm timer */
			timer_add_callout_queue( ( void * )&pef_postpone_timer_handle, 
					pef_capabilities.pef_postpone_timeout_value * HZ, 
					pef_postpone_timer_expired,
					0 );
			break;
	}
	
	resp->completion_code = CC_NORMAL;
}

void
pef_postpone_timer_expired( unsigned char *arg )
{
	GENERIC_EVENT_MSG *evt_msg;
	
	evt_msg = PEF_PENDING_EVENT;
	
//	ipmi_event_handler( evt_msg );	   TODO fix
}



/* 
 * This command is used for setting parameters such as PEF enable/disable and for
 * entering the configuration of the Event Filter table and the Alert Strings. 
 *
 * Completion Codes. Generic plus the following command-specific completion codes:
 * 	80h = parameter not supported.
 * 	81h = attempt to set the ‘set in progress’ value (in parameter #0) 
 * 	when not in the ‘set complete’ state. (This completion code provides
 * 	a way to recognize that another party has already ‘claimed’ the parameters)
 * 	82h = attempt to write read-only parameter.
 */
void
ipmi_set_pef_config_params( IPMI_PKT *pkt )
{
	SET_PEF_CONFIG_PARAMS_CMD_REQ	*req = (SET_PEF_CONFIG_PARAMS_CMD_REQ *)pkt->req;
	SET_PEF_CONFIG_PARAMS_CMD_RESP	*resp =  (SET_PEF_CONFIG_PARAMS_CMD_RESP *)(pkt->resp);

	dputstr( DBG_IPMI | DBG_INOUT, "ipmi_set_pef_config_params: ingress\n" );
	
	/* Configuration parameter data, per Table 30-6, PEF Configuration Parameters. */
	switch( req->param_selector ) {
		case PEF_CONFIG_PARAM_SET_IN_PROGRESS:
			break;
			
		case PEF_CONFIG_PARAM_PEF_CONTROL:
			break;
			
		case PEF_CONFIG_PARAM_PEF_ACTION_GLOBAL_CONTROL:
			break;
			
		case PEF_CONFIG_PARAM_PEF_STARTUP_DELAY:
			break;
			
		case PEF_CONFIG_PARAM_PEF_ALERT_STARTUP_DELAY:
			break;
			
		case PEF_CONFIG_PARAM_NUMBER_OF_EVENT_FILTERS:
			break;
			
		case PEF_CONFIG_PARAM_EVENT_FILTER_TABLE:
			break;
			
		case PEF_CONFIG_PARAM_EVENT_FILTER_TABLE_DATA_1:
			break;
			
		case PEF_CONFIG_PARAM_NUMBER_OF_ALERT_POLICY_ENTRIES:
			break;
			
		case PEF_CONFIG_PARAM_ALERT_POLICY_TABLE:
			break;
			
		case PEF_CONFIG_PARAM_SYSTEM_GUID:
			break;
			
		case PEF_CONFIG_PARAM_NUMBER_OF_ALERT_STRINGS:
			break;
			
		case PEF_CONFIG_PARAM_ALERT_STRING_KEYS:
			break;
			
		case PEF_CONFIG_PARAM_ALERT_STRINGS:
			break;
			
		case PEF_CONFIG_PARAM_NUM_GROUP_CONTROL_TABLE_ENTRIES:
			break;
			
		case PEF_CONFIG_PARAM_GROUP_CONTROL_TABLE:
			break;
			
		default:
			resp->completion_code = 0x80; /* 80h = parameter not supported. */

	}			

	resp->completion_code = CC_NORMAL;
}

/* 
 *
 */
void
ipmi_get_pef_config_params( IPMI_PKT *pkt ) 
{
	GET_PEF_CONFIG_PARAMS_CMD_REQ	*req = (GET_PEF_CONFIG_PARAMS_CMD_REQ *)pkt->req;
	GET_PEF_CONFIG_PARAMS_CMD_RESP	*resp =  (GET_PEF_CONFIG_PARAMS_CMD_RESP *)(pkt->resp);
		
	dputstr( DBG_IPMI | DBG_INOUT, "get_pef_config_params: ingress\n" );

	switch( req->param_selector ) {
		case PEF_CONFIG_PARAM_SET_IN_PROGRESS:
			break;
			
		case PEF_CONFIG_PARAM_PEF_CONTROL:
			break;
			
		case PEF_CONFIG_PARAM_PEF_ACTION_GLOBAL_CONTROL:
			break;
			
		case PEF_CONFIG_PARAM_PEF_STARTUP_DELAY:
			break;
			
		case PEF_CONFIG_PARAM_PEF_ALERT_STARTUP_DELAY:
			break;
			
		case PEF_CONFIG_PARAM_NUMBER_OF_EVENT_FILTERS:
			break;
			
		case PEF_CONFIG_PARAM_EVENT_FILTER_TABLE:
			break;
			
		case PEF_CONFIG_PARAM_EVENT_FILTER_TABLE_DATA_1:
			break;
			
		case PEF_CONFIG_PARAM_NUMBER_OF_ALERT_POLICY_ENTRIES:
			break;
			
		case PEF_CONFIG_PARAM_ALERT_POLICY_TABLE:
			break;
			
		case PEF_CONFIG_PARAM_SYSTEM_GUID:
			break;
			
		case PEF_CONFIG_PARAM_NUMBER_OF_ALERT_STRINGS:
			break;
			
		case PEF_CONFIG_PARAM_ALERT_STRING_KEYS:
			break;
			
		case PEF_CONFIG_PARAM_ALERT_STRINGS:
			break;
			
		case PEF_CONFIG_PARAM_NUM_GROUP_CONTROL_TABLE_ENTRIES:
			break;
			
		case PEF_CONFIG_PARAM_GROUP_CONTROL_TABLE:
			break;
	}			

}

void
ipmi_set_last_processed_event( IPMI_PKT *pkt ) 
{
	SET_LAST_PROCESSED_EVENT_ID_CMD_REQ	*req = (SET_LAST_PROCESSED_EVENT_ID_CMD_REQ *)pkt->req;
	SET_LAST_PROCESSED_EVENT_ID_CMD_RESP	*resp = (SET_LAST_PROCESSED_EVENT_ID_CMD_RESP *)pkt->resp;

	dputstr( DBG_IPMI | DBG_INOUT, "set_last_processed_event: ingress\n" );

	if( req->record_id ) {
		/* set Record ID for last record processed by BMC. */
		evt_processed.last_sw_proc_evt_rec_id = req->rec_id_msb << 8 | req->rec_id_lsb;
	} else {
		/* set Record ID for last record processed by software. */
		evt_processed.last_sw_proc_evt_rec_id = req->rec_id_msb << 8 | req->rec_id_lsb;
	}
	
	resp->completion_code = CC_NORMAL;
	pkt->hdr.resp_data_len = 0;
}

void
ipmi_get_last_processed_event( IPMI_PKT *pkt) 
{
	GET_LAST_PROCESSED_EVENT_ID_CMD_RESP *resp = (GET_LAST_PROCESSED_EVENT_ID_CMD_RESP *)pkt->resp;

	dputstr( DBG_IPMI | DBG_INOUT, "get_last_processed_event: ingress\n" );
	
	/* Most recent addition timestamp. LS byte first. */
	resp->most_recent_timestamp[0] = evt_processed.last_evt_rec_timestamp & 0xf; 
	resp->most_recent_timestamp[1] = ( evt_processed.last_evt_rec_timestamp >> 8 ) & 0xf; 
	resp->most_recent_timestamp[2] = ( evt_processed.last_evt_rec_timestamp >> 16 ) & 0xf; 
	resp->most_recent_timestamp[3] = ( evt_processed.last_evt_rec_timestamp >> 24 ) & 0xf; 

	/* Record ID for last record in SEL. Returns FFFFh if SEL is empty. */
	resp->record_id[0] = evt_processed.last_evt_rec_id & 0xf;
	resp->record_id[1] = evt_processed.last_evt_rec_id >> 8;
	
	resp->last_sw_proc_evt_rec_id[0] = 
		evt_processed.last_sw_proc_evt_rec_id & 0xf; /* LSB:Last SW Processed Event Record ID. */
	resp->last_sw_proc_evt_rec_id[1] = evt_processed.last_sw_proc_evt_rec_id >> 8; /* MSB */
	
	/* Last BMC Processed Event Record ID. Returns 0000h when event has been
	   processed but could not be logged because the SEL is full or logging 
	   has been disabled. */
	resp->last_bmc_proc_evt_rec_id[0] = evt_processed.last_bmc_proc_evt_rec_id & 0xf;
	resp->last_bmc_proc_evt_rec_id[1] = evt_processed.last_bmc_proc_evt_rec_id >> 8;
	
	resp->completion_code = CC_NORMAL;	/* special case for this command: 
						   81h = cannot execute command, 
						   SEL erase in progress */
	pkt->hdr.resp_data_len = 0;

}

/* Process an event message.
 *
 * An event message can be from an external source or internally generated.
 * Each time the BMC receives an event message (either externally or internally
 * generated) it compares the event data against the entries in the event filter
 * table. The BMC scans all entries in the table and collects a set of actions
 * to be performed as determined by the entries that were matched. Actions will
 * then be executed in priority order. An alert action can occur in combination
 * with any other action (in priority order). The power down, power cycle, and
 * reset actions are mutually exclusive. If a combination of power down, power
 * cycle, and/or reset actions results, only the highest priority action will
 * be taken.
 */
void
ipmi_platform_event( IPMI_PKT *pkt ) 
{
	PLATFORM_EVENT_MESSAGE_CMD_REQ	*req = ( PLATFORM_EVENT_MESSAGE_CMD_REQ * )pkt->req;

	dputstr( DBG_IPMI | DBG_INOUT, "ipmi_platform_event: ingress\n" );

	// copy event to log */
	if( ( evt_log.last_evt_rcvd == 0xff ) || ( evt_log.last_evt_rcvd == 31 ) )
		evt_log.last_evt_rcvd = 0;
	else 
		evt_log.last_evt_rcvd++;

	memcpy( &evt_log.evt_msg[evt_log.last_evt_rcvd],  &( req->event_data1 ), sizeof( GENERIC_EVENT_MSG ) );
	
	// check if PEF postpone is in effect 
	
	// call event handler 
	ipmi_event_handler( pkt );

}

void
ipmi_event_handler( IPMI_PKT *pkt )
{
	PLATFORM_EVENT_MESSAGE_CMD_REQ	*req = ( PLATFORM_EVENT_MESSAGE_CMD_REQ * )pkt->req;
	GENERIC_EVENT_MSG *evt_msg = ( GENERIC_EVENT_MSG * )&( req->EvMRev );
	
	int action[PEF_EVT_FILTER_TABLE_ENTRIES];
	unsigned char action_sum = 0;
	int i;

	/* find all the filters that match */
	for( i = 0; i < PEF_EVT_FILTER_TABLE_ENTRIES; i++ ) {
		if( event_data_compare( evt_msg->evt_data1, 
			( PEF_MASK * )&pef_filter_table[i].filter_data.event_data1_and_mask ) )
		{
			action[i] = pef_filter_table[i].filter_data.event_filter_action;
		} else {
			action[i] = 0;
		}
	}
	/* execute actions in priority sequence */
	for( i = 0; i < PEF_EVT_FILTER_TABLE_ENTRIES; i++ ) {
		action_sum |= action[i];
	}
	if( action_sum & PEF_ACTION_POWER_DOWN ) {
		// power down
	} else if( action_sum & PEF_ACTION_POWER_CYCLE ) {
		// power cycle
	} else if( action_sum & PEF_ACTION_RESET ) {
		// reset
	} else if( action_sum & PEF_ACTION_DIAG_INTERRUPT ) {
		// interrupt
	}

	/* Now handle the alerts if any */
	module_event_handler( pkt );
}

	
/*
The Event Data 1 Event Offset Mask field in the Event Filter is used to match
multiple bits in the Event Offset field of the Event Data 1 byte of an event. 
The least significant nibble of event data 1 typically holds an event offset
value. This offset selects among different possible events for a sensor. 
For example, a ‘button’ sensor supports a set of sensor-specific event 
offsets: 0 for Power Button pressed, 1 for Sleep Button pressed, and 2 for
Reset Button pressed. When an event is generated, it could have a 0, 1, or 
2 in the event offset field depending on what button press occurred.

The Event Offset Mask makes it simple to have a filter match a subset of the
possible event offset values. Each bit in the mask corresponds to a different
offset values starting with bit 0 in the mask corresponding to offset 0. For
example, if it is desired to have a filter match offsets 0 and 2, but not 1,
the mask would be configured to 000_0000_0000_0101b.
*/

/*
The AND Mask and the Compare 1 and Compare 2 fields are used in combination to
allow wildcarding, ‘one or more bit(s)’, and exact comparisons to be made between
bits in the corresponding event data byte. One way to understanding the bits is
to look at the way they’re used in combination. First the AND Mask is applied to
the test value. The result, referred to below as the test value, is then bit-wise
matched based on the values in the Compare 1 and Compare 2 fields.
*/
int event_data_compare( uchar test_value, PEF_MASK *pef_mask )
{
	uchar	temp1, temp2;
	int	match = 0;

	temp1 = ( test_value & pef_mask->AND_mask );
	
	if ( ( temp1 & pef_mask->compare1 ) == ( pef_mask->compare2 & pef_mask->compare1 ) ) { 
		match = 0; 
	
		if ( pef_mask->compare1 != 0xFF ) { 
			temp2 = temp1 & !pef_mask->compare1;

			if ( pef_mask->compare2 != 0x00 ) {
				if ( !( temp2 & pef_mask->compare2 ) ) 
					match = 0; 
			};

			if ( pef_mask->compare2 != 0xFF ) {
				if ( !( temp2 & !pef_mask->compare2 ) )
					match = 0; 
			};
		};
	} else 
		match = 0; 
	
	return( match );
}

/*======================================================================*/
/* 
 *  Event Mandatory Commands
 *  	Set Event Receiver
 *  	Get Event Receiver
 *  	Platform Event (aka Event Message)
 * 
 *  Using NETFN_EVENT_REQ/NETFN_EVENT_RESP
 */
/*======================================================================*/

/* This global command tells a controller where to send Event Messages.
 * This command is only applicable to management controllers that act as 
 * IPMB Event Generators.
 * 
 * A device that receives a ‘Set Event Receiver’ command shall ‘re-arm’ 
 * event generation for all its internal sensors. This means internally 
 * re-scanning for the event condition, and updating the event status 
 * based on the result. This will cause devices that have any pre-existing
 * event conditions to transmit new event messages for those events.
 */
void
ipmi_set_event_receiver( IPMI_PKT *pkt )
{
	SET_EVENT_RECEIVER_CMD_REQ	*req = ( SET_EVENT_RECEIVER_CMD_REQ * )pkt->req;
	SET_EVENT_RECEIVER_CMD_RESP	*resp = ( SET_EVENT_RECEIVER_CMD_RESP * )pkt->resp;

	dputstr( DBG_IPMI | DBG_INOUT, "ipmi_set_event_receiver: ingress\n" );

	if( evt_config.receiver_slave_addr != 0xff ) {
		evt_config.receiver_slave_addr = req->evt_receiver_slave_addr;
		evt_config.receiver_lun = req->evt_receiver_lun;
		evt_config.evt_enabled = 1;
	} else {
		evt_config.evt_enabled = 0;
	}
	
	resp->completion_code = CC_NORMAL;

	module_rearm_events();
	
	dputstr( DBG_IPMI | DBG_INOUT, "ipmi_set_event_receiver: egress\n" );
}

/* This global command is used to retrieve the present setting for the Event
 * Receiver Slave Address and LUN. This command is only applicable to 
 * management controllers that act as IPMB Event Generators.
 */
void
ipmi_get_event_receiver( IPMI_PKT *pkt )
{
	GET_EVENT_RECEIVER_CMD_RESP	*resp = ( GET_EVENT_RECEIVER_CMD_RESP * )pkt->resp;
	
	dputstr( DBG_IPMI | DBG_INOUT, "ipmi_get_event_receiver: ingress\n" );

	/* Event Receiver Slave Address. 0FFh indicates Event Message 
	   Generation has been disabled. Otherwise
	   [7:1] IPMB (I2C) Slave Address
	   [0] always 0b when [7:1] hold I2C slave address */
	if( evt_config.evt_enabled ) {
		resp->evt_receiver_slave_addr = evt_config.receiver_slave_addr;	
	} else {
		resp->evt_receiver_slave_addr = 0xFF;
	}

	resp->evt_receiver_lun = evt_config.receiver_lun;		/* [1:0] - Event Receiver LUN */

	resp->completion_code = CC_NORMAL;
	
	dputstr( DBG_IPMI | DBG_INOUT, "ipmi_get_event_receiver: egress\n" );
}


int
ipmi_send_event_req( uchar *msg_cmd, unsigned msg_len, void(*ipmi_completion_function)( void *, int ) )
{
	IPMI_PKT *pkt;
	IPMI_WS *req_ws;
	uchar seq;
	uchar responder_slave_addr;

	ipmi_event_init();

	if( !( req_ws = ws_alloc() ) ) {
		return( -1 );
	}
	
	pkt = &req_ws->pkt;
	pkt->hdr.ws = (char *)req_ws;
	pkt->hdr.req_data_len = msg_len - 1;
	
	ipmi_get_next_seq( &seq );
	
	req_ws->outgoing_protocol = IPMI_CH_PROTOCOL_IPMB;
	req_ws->outgoing_medium = IPMI_CH_MEDIUM_IPMB;
	req_ws->ipmi_completion_function = ipmi_completion_function;
	
	switch( req_ws->outgoing_protocol ) {
		case IPMI_CH_PROTOCOL_IPMB: {
			IPMI_IPMB_REQUEST *ipmb_req = ( IPMI_IPMB_REQUEST * )&( req_ws->pkt_out );
			req_ws->addr_out = evt_config.receiver_slave_addr;
			pkt->req = ( IPMI_CMD_REQ * )&( ipmb_req->command );

			memcpy( pkt->req, msg_cmd, msg_len ); // memcpy ( destination, source, size ); 

			ipmb_req->requester_slave_addr = module_get_i2c_address( I2C_ADDRESS_LOCAL );
			ipmb_req->netfn = NETFN_EVENT_REQ;
			ipmb_req->requester_lun = 0;
//			ipmb_req->responder_slave_addr = evt_config.receiver_slave_addr;
			responder_slave_addr = evt_config.receiver_slave_addr;
			ipmb_req->header_checksum = -( *( char * )ipmb_req + responder_slave_addr );
			ipmb_req->req_seq = seq;
			ipmb_req->responder_lun = evt_config.receiver_lun;
			ipmb_req->command = IPMI_SE_PLATFORM_EVENT;
			/* The location of data_checksum field is bogus.
			 * It's used as a placeholder to indicate that a checksum follows the data field.
			 * The location of the data_checksum depends on the size of the data preceeding it.*/
			ipmb_req->data_checksum = 
				ipmi_calculate_checksum( &ipmb_req->requester_slave_addr, 
					pkt->hdr.req_data_len + 3 ); 
			req_ws->len_out = sizeof( IPMI_IPMB_REQUEST ) 
				- IPMB_REQ_MAX_DATA_LEN  +  pkt->hdr.req_data_len;
			/* Assign the checksum to it's proper location */
			*( (uchar *)ipmb_req + req_ws->len_out - 1) = ipmb_req->data_checksum; 
			}			
			break;
		
		case IPMI_CH_PROTOCOL_TMODE: {		/* Terminal Mode */
			IPMI_TERMINAL_MODE_REQUEST *tm_req = 
				( IPMI_TERMINAL_MODE_REQUEST * )&( req_ws->pkt_in );

			pkt->req = ( IPMI_CMD_REQ * )&( tm_req->command );

			memcpy( pkt->req, msg_cmd, msg_len ); // memcpy ( destination, source, size ); 

			tm_req->netfn = NETFN_EVENT_REQ;
			tm_req->responder_lun = evt_config.receiver_lun;
			tm_req->req_seq = seq;
			tm_req->bridge = 0; 
			tm_req->command = IPMI_SE_PLATFORM_EVENT;
			req_ws->len_out = sizeof(IPMI_TERMINAL_MODE_RESPONSE)
				- TERM_MODE_RESP_MAX_DATA_LEN + pkt->hdr.req_data_len;
			}
			break;
		
		case IPMI_CH_PROTOCOL_ICMB:		/* ICMB v1.0 */
		case IPMI_CH_PROTOCOL_SMB:		/* IPMI on SMSBus */
		case IPMI_CH_PROTOCOL_KCS:		/* KCS System Interface Format */
		case IPMI_CH_PROTOCOL_SMIC:		/* SMIC System Interface Format */
		case IPMI_CH_PROTOCOL_BT10:		/* BT System Interface Format, IPMI v1.0 */
		case IPMI_CH_PROTOCOL_BT15:		/* BT System Interface Format, IPMI v1.5 */
			/* Unsupported protocol */
			ws_free( req_ws );
			return( -1 );
	}
	
	ws_set_state( req_ws, WS_ACTIVE_MASTER_WRITE );

	return( seq );
}

