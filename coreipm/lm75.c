
/*
-------------------------------------------------------------------------------
coreIPM/lm75.c

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

#include "string.h"
#include "ipmi.h"
#include "ws.h"
#include "sensor.h"
#include "timer.h"
#include "lm75.h"

#define MAX_LM75_SENSOR_COUNT 2
unsigned char lm75_sensor_count = 0;
unsigned char lm75_update_sensor_timer_handle;

typedef struct lm75_sensor_info {
	unsigned char sensor_id;
	unsigned char reading_hi;
	unsigned char reading_lo;
	unsigned char interface;
	unsigned char i2c_addr;
} LM75_SENSOR_INFO;

LM75_SENSOR_INFO lm75_sensor[MAX_LM75_SENSOR_COUNT];

void lm75_update_sensor( unsigned char *arg );

/*
GENERAL OPERATION

The 8-bit Pointer Register of the devices is used to address a given data
register. The Pointer Register uses the two LS bits to identify which of the
data registers should respond to a read or write command.

Accessing a particular register on the TMP175 and TMP75 is accomplished by
writing the appropriate value to the Pointer Register. The value for the
Pointer Register is the first byte transferred after the slave address byte
Every write operation to the TMP175 and TMP75 requires a value for the Pointer
Register.

When reading from the TMP175 and TMP75, the last value stored in the Pointer
Register by a write operation is used to determine which register is read by
a read operation. To change the register pointer for a read operation, a new
value must be written to the Pointer Register.

*/ 

// register select bits
#define REGSEL_TEMP	0	// Temperature Register (READ Only)
#define REGSEL_CONFIG	1	// Configuration Register (READ/WRITE)
#define REGSEL_TLOW	2	// TLOW Register (READ/WRITE)
#define REGSEL_THIGH	3	// THIGH Register (READ/WRITE)

// Pointer Register Byte
typedef struct pointer_register {
#ifdef BF_MS_FIRST
	unsigned char	register_select:2,	// REGSEL_xx
			:6;
#else
	unsigned char	:6,
			register_select:2;
#endif
} POINTER_REGISTER;


// configuration register
typedef struct configuration_register {
#ifdef BF_MS_FIRST
	unsigned char	shutdown_mode:1,
			thermostat_mode:1,
			polarity:1,
			fault_queue:2,
			conv_resolution:2,
			one_shot:1;
#else
	unsigned char	one_shot:1,
			conv_resolution:2,
			fault_queue:2,
			polarity:1,
			thermostat_mode:1,
			shutdown_mode:1;
#endif
} CONFIGURATION_REGISTER;

FULL_SENSOR_RECORD lm75sr;
SENSOR_DATA lm75sd;

void lm75_init_completion_function( IPMI_WS *ws, int status );
void lm75_update_sensor_completion_function( IPMI_WS *ws, int status );

/*
 * Initialization is a two step process; first write to the Configuration Register
 * to set the operating mode, when this completes send another write to set the
 * Pointer Register to Temperature Register. Subsequent reads will then return
 * the value of the Temperature Register.
 *
 * Returns the instance number of the sensor
 */
int
lm75_init( 
	unsigned char interface,
	unsigned char i2c_addr )
{
	POINTER_REGISTER *preg;
	CONFIGURATION_REGISTER *creg;
	IPMI_WS *req_ws;
	
	// lm75_init_sensor_record();

	if( !( req_ws = ws_alloc() ) ) {
		return( -1 );
	}
	
	if( lm75_sensor_count >= MAX_LM75_SENSOR_COUNT )
		return( -1 );
	
	lm75_sensor_count++;
	
	// keep track of initialized sensors
	lm75_sensor[lm75_sensor_count - 1].interface = interface;
	lm75_sensor[lm75_sensor_count - 1].i2c_addr = i2c_addr;
	lm75_sensor[lm75_sensor_count - 1].sensor_id = lm75_sensor_count - 1;
	
	// we're going to do a write of two byes, first byte is the pointer reg,
	// the second the config register
	preg = ( POINTER_REGISTER * )&( req_ws->pkt_out[0] );
	preg->register_select = REGSEL_CONFIG;
	
	creg = ( CONFIGURATION_REGISTER * )&( req_ws->pkt_out[1] );
	creg->shutdown_mode = 0;	// disable shutdown
	creg->thermostat_mode = 0;	// use comparator mode
	creg->polarity = 0;		// ALERT active low
	creg->fault_queue = 1;		// 2 consecutive faults 
	creg->conv_resolution = 3;	// 12 bits
	creg->one_shot = 0;

	req_ws->outgoing_protocol = IPMI_CH_PROTOCOL_IPMB;
	req_ws->outgoing_medium = IPMI_CH_MEDIUM_IPMB;
	req_ws->ipmi_completion_function = lm75_init_completion_function;
	req_ws->addr_out = i2c_addr;
	req_ws->interface = interface;
	req_ws->len_out = 2;

	ws_set_state( req_ws, WS_ACTIVE_MASTER_WRITE );

	return( lm75_sensor_count - 1 );
}

void
lm75_init_sensor_record( void )
{
	lm75sr.record_id[0] = 1;
	lm75sr.sdr_version = 0x51;
	lm75sr.record_type = 1;	/* Record Type Number = 01h, Full Sensor Record */
	lm75sr.record_len = 0;	/* Number of remaining record bytes following. */
	lm75sr.owner_id = 0;	/* 7-bit system software ID */
	lm75sr.id_type = 1;	/* System software type */
	lm75sr.channel_num = 0;
	lm75sr.sensor_owner_lun = 0; 
	lm75sr.sensor_number = 0;	/* this will get replaced by the actual sensor number when we register the SDR */
	lm75sr.entity_id = ENTITY_ID_SYSTEM_BOARD; /* physical entity the sensor is monitoring */

	lm75sr.entity_type = 0;	/* treat entity as a physical entity */
	lm75sr.entity_instance_num = 0;
	lm75sr.init_scanning = 1;	/* the sensor accepts the ‘enable/disable scanning’ bit in the 
				   Set Sensor Event Enable command). */
	lm75sr.init_events = 0;
	lm75sr.init_thresholds = 0;
	lm75sr.init_hysteresis = 0;
	lm75sr.init_sensor_type = 0;

	/* Sensor Default (power up) State */
	lm75sr.powerup_evt_generation = 0;	/* event generation disabled */
	lm75sr.powerup_sensor_scanning = 1;	/* sensor scanning enabled */
	lm75sr.ignore_sensor = 1;			/* Ignore sensor if entity is not present or disabled. */

	/* Sensor Auto Re-arm Support */
	lm75sr.sensor_manual_support = 1;		/* automatically rearms itself when the event clears */
				    
	/* Sensor Hysteresis Support */
	lm75sr.sensor_hysteresis_support = 0; 	/* No hysteresis */
					
	/* Sensor Threshold Access Support */
	lm75sr.sensor_threshold_access = 0;	/* no thresholds */
				     
	/* Sensor Event Message Control Support */
	lm75sr.event_msg_control = 1;			/* entire sensor only (implies that global
						   disable is also supported) */

	lm75sr.sensor_type = ST_TEMPERATURE;		/* From Table 42-3, Sensor Type Codes */
	lm75sr.event_type_code = 0;		/* unspecified */
	lm75sr.event_mask = 0;
	lm75sr.deassertion_event_mask = 0;
	lm75sr.reading_mask = 0;
	lm75sr.analog_data_format = 0;		/* unsigned */
	lm75sr.rate_unit = 0;			/* none */
	lm75sr.modifier_unit = 0;			/* 00b = none */
	lm75sr.percentage = 0;			/* not a percentage value */
	lm75sr.sensor_units2 = SENSOR_UNIT_DEGREES_CELSIUS;	/*  Base Unit */
	lm75sr.sensor_units3 = 0;		/* no modifier unit */
	lm75sr.linearization = 0;		/* Linear */
	lm75sr.M = 0;		
	lm75sr.M_tolerance = 0;
	lm75sr.B = 0;
	lm75sr.B_accuracy = 0;
	lm75sr.accuracy = 0;
	lm75sr.R_B_exp = 0;
	lm75sr.analog_characteristic_flags = 0;
	lm75sr.nominal_reading;
	lm75sr.normal_maximum = 0;
	lm75sr.normal_minimum;
	lm75sr.sensor_maximum_reading = 0xff;
	lm75sr.sensor_minimum_reading = 0;
	lm75sr.upper_non_recoverable_threshold = 0;
	lm75sr.upper_critical_threshold = 0;
	lm75sr.upper_non_critical_threshold = 0;
	lm75sr.lower_non_recoverable_threshold = 0;	
	lm75sr.lower_critical_threshold = 0;
	lm75sr.lower_non_critical_threshold = 0;
	lm75sr.positive_going_threshold_hysteresis_value = 0;
	lm75sr.negative_going_threshold_hysteresis_value = 0;
	lm75sr.reserved2 = 0;
	lm75sr.reserved3 = 0;
	lm75sr.oem = 0;
	lm75sr.id_string_type = 3;	/* 11 = 8-bit ASCII + Latin 1. */ 
	lm75sr.id_string_length = 11; /* length of following data, in characters */
	memcpy( lm75sr.id_string_bytes, "Board Temperature", 17 ); /* Sensor ID String bytes. */

	sensor_add( &lm75sr, &lm75sd );
}


int
lm75_update_sensor_reading( 
	unsigned char interface,
	unsigned char i2c_addr, 
	unsigned char *sensor_data )
{
	IPMI_WS *req_ws;

	if( !( req_ws = ws_alloc() ) ) {
		return( -1 );
	}
	req_ws->outgoing_protocol = IPMI_CH_PROTOCOL_IPMB;
	req_ws->incoming_protocol = IPMI_CH_PROTOCOL_IPMB;
	req_ws->outgoing_medium = IPMI_CH_MEDIUM_IPMB;
	req_ws->outgoing_channel = IPMI_CH_NUM_PRIMARY_IPMB;
	req_ws->addr_out = i2c_addr;
	req_ws->interface = interface;
	req_ws->ipmi_completion_function = lm75_update_sensor_completion_function;
	req_ws->len_rcv = 2;	/* amount of data we want to read */
	
	/* dispatch the request */
	ws_set_state( req_ws, WS_ACTIVE_MASTER_READ );
		
	return( 0 );
}

/* This function handles completion for two events:
 * 	- initial config write to lm75
 * 	- write to switch the register selector to the temperature register
 */
void
lm75_init_completion_function( IPMI_WS *ws, int status )
{
	POINTER_REGISTER *preg;
	unsigned char sensor_id;

	preg = (POINTER_REGISTER *)&(ws->pkt_out[0]);
	
	if( preg->register_select == REGSEL_CONFIG ) {
		// if we completed initial config write to lm75, 
		// switch the register selector to the temperature register
		preg->register_select = REGSEL_TEMP;
		ws->outgoing_protocol = IPMI_CH_PROTOCOL_IPMB;
		ws->outgoing_medium = IPMI_CH_MEDIUM_IPMB;
		ws->ipmi_completion_function = lm75_init_completion_function;
		ws->len_out = 1;
		ws_set_state( ws, WS_ACTIVE_MASTER_WRITE );
	} else 	if( preg->register_select == REGSEL_TEMP ) {
		// we completed a write to switch the register 
		// selector to the temperature register
		
		// find sensor id 
		for( sensor_id = 0; sensor_id < lm75_sensor_count; sensor_id++ ) {
			if( ( lm75_sensor[sensor_id].interface == ws->interface ) &&
			    ( lm75_sensor[sensor_id].i2c_addr == ws->addr_out ) ) {
				break;
			    }
		}
		ws_free( ws );
		// Register a callout to read temp sensors periodically			
		timer_add_callout_queue( ( void * )&lm75_update_sensor_timer_handle,
	       		10*HZ, lm75_update_sensor, ( unsigned char * )&lm75_sensor[sensor_id] ); /* 10 sec timeout */
	}
}

/*
The Temperature Register is a 12-bit, read-only register that stores the output
of the most recent conversion. Two bytes must be read to obtain data.
Byte 1 is the most significant byte, followed by byte 2, the least significant
byte. Following power-up or reset, the Temperature Register will read 0°C until
the first conversion is complete.
*/
void
lm75_update_sensor( unsigned char *arg )
{
	IPMI_WS *req_ws;
	LM75_SENSOR_INFO *lm75_sensor = ( LM75_SENSOR_INFO * )arg;

	if( !( req_ws = ws_alloc() ) ) {
		return;
	}

	req_ws->outgoing_protocol = IPMI_CH_PROTOCOL_IPMB;
	req_ws->outgoing_medium = IPMI_CH_MEDIUM_IPMB;
	req_ws->ipmi_completion_function = lm75_update_sensor_completion_function;
	req_ws->addr_out = lm75_sensor->i2c_addr;
	req_ws->interface = lm75_sensor->interface;
	req_ws->len_in = 2;

	ws_set_state( req_ws, WS_ACTIVE_MASTER_READ );
}


void
lm75_update_sensor_completion_function( IPMI_WS *ws, int status )
{
	unsigned char sensor_id;

	// find sensor id 
	for( sensor_id = 0; sensor_id < lm75_sensor_count; sensor_id++ ) {
		if( ( lm75_sensor[sensor_id].interface == ws->interface ) &&
		    ( lm75_sensor[sensor_id].i2c_addr == ws->addr_out ) ) {
			break;
	    	}
	}
	
	switch ( status ) {
		case XPORT_REQ_NOERR:
			lm75_sensor[sensor_id].reading_hi = ws->pkt_in[0]; 
			lm75_sensor[sensor_id].reading_lo = ws->pkt_in[1]; 			
			break;
		case XPORT_REQ_ERR:
		case XPORT_RESP_NOERR:
		case XPORT_RESP_ERR:
		default:
			lm75_sensor[sensor_id].reading_hi = 0; 
			lm75_sensor[sensor_id].reading_lo = 0; 			
			break;
	}
	ws_free( ws );
}


