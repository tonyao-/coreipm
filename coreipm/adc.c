/*
-------------------------------------------------------------------------------
coreIPM/adc.c

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
#include "arch.h"
#include "ipmi.h"
#include "sensor.h"
#include "debug.h"

#define VREF  3

FULL_SENSOR_RECORD adsr;

void
adc_ch0_read( SENSOR_DATA *sensor )
{
	unsigned int val;
	
	/* Setup the A/D converter */
//	VPBDIV = 0x02;		               /*Set the Pclk to 30 Mhz              */
	AD0CR   = 0x00210601;                   /* Setup A/D: 10-bit AIN0 @ 3MHz */

	AD0CR  |= 0x01000000;			/* Start A/D Conversion */
	while ((AD0DR0 & 0x80000000) == 0);	/* Wait for the conversion to complete */
	val = ((AD0DR0 >> 6) & 0x03FF);		/* Extract the result */

	sensor->last_sensor_reading = val;

	dprintf( DBG_GPIO | DBG_LVL1, "A/D reading %4u = %01u.%04u Volts\r", val,
		(val * VREF) >> 10,                          /* Integer */
		((val * VREF * 10000UL) >> 10UL) % 10000);   /* Decimal */
}

void
adc_init_sensor_record( void )
{
	adsr.record_id[0] = 1;
	adsr.sdr_version = 0x51;
	adsr.record_type = 1;	/* Record Type Number = 01h, Full Sensor Record */
	adsr.record_len = 0;	/* Number of remaining record bytes following. */
	adsr.owner_id = 0;	/* 7-bit system software ID */
	adsr.id_type = 1;	/* System software type */
	adsr.channel_num = 0;
	adsr.sensor_owner_lun = 0; 
	adsr.sensor_number = 1;
	adsr.entity_id = ENTITY_ID_POWER_SUPPLY; /* physical entity the sensor is monitoring */

	adsr.entity_type = 0;	/* treat entity as a physical entity */
	adsr.entity_instance_num = 0;
	adsr.init_scanning = 1;	/* the sensor accepts the ‘enable/disable scanning’ bit in the 
				   Set Sensor Event Enable command). */
	adsr.init_events = 0;
	adsr.init_thresholds = 0;
	adsr.init_hysteresis = 0;
	adsr.init_sensor_type = 0;

	/* Sensor Default (power up) State */
	adsr.powerup_evt_generation = 0;	/* event generation disabled */
	adsr.powerup_sensor_scanning = 1;	/* sensor scanning enabled */
	adsr.ignore_sensor = 1;			/* Ignore sensor if entity is not present or disabled. */

	/* Sensor Auto Re-arm Support */
	adsr.sensor_manual_support = 1;		/* automatically rearms itself when the event clears */
				    
	/* Sensor Hysteresis Support */
	adsr.sensor_hysteresis_support = 0; 	/* No hysteresis */
					
	/* Sensor Threshold Access Support */
	adsr.sensor_threshold_access = 0;	/* no thresholds */
				     
	/* Sensor Event Message Control Support */
	adsr.event_msg_control = 1;			/* entire sensor only (implies that global
						   disable is also supported) */

	adsr.sensor_type = ST_VOLTAGE;		/* From Table 42-3, Sensor Type Codes */
	adsr.event_type_code = 0;		/* unspecified */
	adsr.event_mask = 0;
	adsr.deassertion_event_mask = 0;
	adsr.reading_mask = 0;
	adsr.analog_data_format = 0;		/* unsigned */
	adsr.rate_unit = 0;			/* none */
	adsr.modifier_unit = 0;			/* 00b = none */
	adsr.percentage = 0;			/* not a percentage value */
	adsr.sensor_units2 = SENSOR_UNIT_VOLTS;	/*  Base Unit */
	adsr.sensor_units3 = 0;		/* no modifier unit */
	adsr.linearization = 0;		/* Linear */
	adsr.M = 0;		
	adsr.M_tolerance = 0;
	adsr.B = 0;
	adsr.B_accuracy = 0;
	adsr.accuracy = 0;
	adsr.R_B_exp = 0;
	adsr.analog_characteristic_flags = 0;
	adsr.nominal_reading;
	adsr.normal_maximum = 0;
	adsr.normal_minimum;
	adsr.sensor_maximum_reading = 0xff;
	adsr.sensor_minimum_reading = 0;
	adsr.upper_non_recoverable_threshold = 0;
	adsr.upper_critical_threshold = 0;
	adsr.upper_non_critical_threshold = 0;
	adsr.lower_non_recoverable_threshold = 0;	
	adsr.lower_critical_threshold = 0;
	adsr.lower_non_critical_threshold;
	adsr.positive_going_threshold_hysteresis_value;
	adsr.negative_going_threshold_hysteresis_value;
	adsr.reserved2 = 0;
	adsr.reserved3 = 0;
	adsr.oem = 0;
	adsr.id_string_type = 3;	/* 11 = 8-bit ASCII + Latin 1. */ 
	adsr.id_string_length = 11; /* length of following data, in characters */
	memcpy( adsr.id_string_bytes, "PSU voltage", 11 ); /* Sensor ID String bytes. */

}
