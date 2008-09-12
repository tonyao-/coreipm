/*
-------------------------------------------------------------------------------
coreIPM/sensor.h

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

typedef struct sensor_data {
	uchar	sensor_id; 
	uchar	last_sensor_reading;
	uchar	scan_period;		/* time between each sensor scan in seconds, 0 = no scan */
	void(*scan_function)( void * );	/* the routine that does the sensor scan */
#ifdef BF_MS_FIRST
	uchar	event_messages_enabled:1,	/* 0b = All Event Messages disabled from this sensor */
		sensor_scanning_enabled:1,	/* 0b = sensor scanning disabled */
		unavailable:1,			/* 1b = reading/state unavailable */
		reserved;
#else
	uchar	reserved:5,
		unavailable:1,
		sensor_scanning_enabled:1,
		event_messages_enabled:1;
#endif 
} SENSOR_DATA;

typedef struct sdr_entry {
	unsigned short	record_id;
	uchar	rec_len;
	uchar	*record_ptr;
} SDR_ENTRY;


void ipmi_get_device_sdr_info( IPMI_PKT *pkt );
void ipmi_get_device_sdr( IPMI_PKT *pkt );
void ipmi_reserve_device_sdr_repository( IPMI_PKT *pkt );
void ipmi_get_sensor_reading( IPMI_PKT *pkt );
int  sensor_add( FULL_SENSOR_RECORD *sdr, SENSOR_DATA *sensor_data ); 

