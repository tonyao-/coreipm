/*
-------------------------------------------------------------------------------
coreIPM/sensor.c

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
#include "debug.h"
#include "ipmi.h"
#include "event.h"
#include "sensor.h"

#define MAX_SDR_COUNT		8
#define MAX_SENSOR_COUNT	8

// in this implementation: number of sensors == number of SDRs
unsigned char current_sensor_count = 0;

SDR_ENTRY sdr_entry_table[MAX_SDR_COUNT];

unsigned sdr_reservation_id = 0;
SENSOR_DATA *sensor[MAX_SENSOR_COUNT];

/*======================================================================*/
int
sensor_add(
	FULL_SENSOR_RECORD *sdr, 
	SENSOR_DATA *sensor_data ) 
{
	if( current_sensor_count + 1 > MAX_SENSOR_COUNT )
		return( -1 );
	
	sdr_entry_table[current_sensor_count].record_ptr = ( uchar * )sdr;
	((FULL_SENSOR_RECORD *)(sdr_entry_table[current_sensor_count].record_ptr))->sensor_number = current_sensor_count;
	((FULL_SENSOR_RECORD *)(sdr_entry_table[current_sensor_count].record_ptr))->record_id[0] = current_sensor_count;
	sensor[current_sensor_count] = sensor_data;
	current_sensor_count++;

	return( 0 );
}



/*======================================================================*/
/*
 *   Sensor Device Commands
 *
 *   Mandatory Commands
 *   	Get Device SDR Info
 *   	Get Device SDR
 *   	Reserve Device SDR Repository
 *   	Get Sensor Reading
 *
 *   	Using NETFN_EVENT_REQ/NETFN_EVENT_RESP
 */
/*======================================================================*/
void
ipmi_get_device_sdr_info( IPMI_PKT *pkt )
{
	GET_DEVICE_SDR_INFO_CMD	*req = (GET_DEVICE_SDR_INFO_CMD *)pkt->req;
	GET_DEVICE_SDR_INFO_RESP *resp = (GET_DEVICE_SDR_INFO_RESP *)(pkt->resp);
	unsigned char	lun = pkt->hdr.lun;

	dputstr( DBG_IPMI | DBG_INOUT, "ipmi_get_device_sdr_info: ingress\n" );

	/* operation bit [0] 
	 	 1b = Get SDR count. This returns the total number of SDRs in
		      the device.
		 0b = Get Sensor count. This returns the number of sensors
		      implemented on LUN this command was addressed to */
	if( req->operation & 0x01 ) {
		resp->num = current_sensor_count;
	} else {
		if( lun == 0 )
			resp->num = current_sensor_count;
		else
			resp->num = 0;
	}

	/* Flags:
	   0b = static sensor population. The number of sensors handled by this
	   device is fixed, and a query shall return records for all sensors.
	   1b = dynamic sensor population. This device may have its sensor 
	   population vary during ‘run time’ (defined as any time other that
	   when an install operation is in progress). */	
	resp->flags = 0;
	
	/* Device LUNs
	   [3] - 1b = LUN 3 has sensors
	   [2] - 1b = LUN 2 has sensors
	   [1] - 1b = LUN 1 has sensors
	   [0] - 1b = LUN 0 has sensors */
	resp->device_luns = 1;

	/* Four byte timestamp, or counter. Updated or incremented each time
	    the sensor population changes. This field is not provided if the
	    flags indicate a static sensor population.*/
	/* resp->sensor_population_change_indicator; */
	
	resp->completion_code = CC_NORMAL;
	pkt->hdr.resp_data_len = sizeof( GET_DEVICE_SDR_INFO_RESP ) - 1;	
}	

/*
Get Device SDR Command (section 35.3)

The ‘Get Device SDR’ command allows SDR information for sensors for a Sensor
Device (typically implemented in a satellite management controller) to be
returned. The Get Device SDR Command can return any type of SDR, not just 
Types 01h and 02h. This is an optional command for Static Sensor Devices, and
mandatory for Dynamic Sensor Devices (also mandatory for ATCA). The format and
action of this command is similar to that for the ‘Get SDR’ command for SDR
Repository Devices.

A Sensor Device shall always utilize the same sensor number for a particular
sensor. This is mandatory to keep System Event Log information consistent.

Sensor Devices that support the ‘Get Device SDR’ command return SDR Records
that match the SDR Repository formats. See section 43.

*/
void 
ipmi_get_device_sdr( IPMI_PKT *pkt )
{
	GET_DEVICE_SDR_CMD *req = (GET_DEVICE_SDR_CMD *)( pkt->req );
	GET_DEVICE_SDR_RESP *resp = (GET_DEVICE_SDR_RESP *)( pkt->resp );
	unsigned short record_id, i, found = 0;

	/* if offset into record is zero we don't have to worry about the
	 * reservation ids */
	if( req->offset != 0 ) {
		/* Otherwise check to see if we have the reservation */
		if( sdr_reservation_id != ( req->reservation_id_msb << 8 | req->reservation_id_lsb ) ) {
			resp->completion_code = CC_RESERVATION;
			pkt->hdr.resp_data_len = 0;
			return;
		}
	}
	/* check if we have a valid record ID */	
	record_id = req->record_id_msb << 8 | req->record_id_lsb;
	for( i = 0; i < current_sensor_count; i++ ) {
		if( sdr_entry_table[i].record_id == record_id ) {
			found++;
			break;
		}
	}

	/* using record ID, offset and bytes to read fields, fill in the req_bytes field */
	if( found ) {

		/* fill in the Record ID for next record */
		if( i + 1 < current_sensor_count ) {
			resp->rec_id_next_lsb = sdr_entry_table[i+1].record_id & 0xf;
			resp->rec_id_next_msb = sdr_entry_table[i+1].record_id >> 8;
		} else {
			resp->rec_id_next_lsb = 0xff;
			resp->rec_id_next_msb = 0xff;
		}

		/* SDR Data goes in here */
		/* check req->bytes_to_read. FFh means read entire record. */
		if( req->bytes_to_read + req->offset > sdr_entry_table[i].rec_len ) {
			memcpy( resp->req_bytes, sdr_entry_table[i].record_ptr + req->offset, sdr_entry_table[i].rec_len - req->offset);
	       		pkt->hdr.resp_data_len = sdr_entry_table[i].rec_len - req->offset + 2;
		} else {
			memcpy( resp->req_bytes, sdr_entry_table[i].record_ptr + req->offset, req->bytes_to_read );
	       		pkt->hdr.resp_data_len = req->bytes_to_read + 2;
		}
		
		/* TODO return a 80h = record changed status if any of the record contents
		have been altered since the last time the Requester issued the request 
		with 00h for the ‘Offset into SDR’ field. This can be implemented by adding
	        last_query_ts, and record_change_ts timestamps to the SDR_ENTRY struct.	
		Q: shouldn't the reservation scheme take care of this ? */
		resp->completion_code = CC_NORMAL;
	} else {
		resp->completion_code = CC_REQ_DATA_NOT_AVAIL; 
		pkt->hdr.resp_data_len = 0;
	}
}

/*
This command is used to obtain a Reservation ID. The Reservation ID is part of a
mechanism that is used to notify the Requester that a record may have changed 
during the process of a multi-part read. See Reserve SDR Repository, for more
information on the function and use of Reservation IDs.
*/ 
void
ipmi_reserve_device_sdr_repository( IPMI_PKT *pkt )
{
	RESERVE_DEVICE_SDR_REPOSITORY_RESP *resp = ( RESERVE_DEVICE_SDR_REPOSITORY_RESP * )(pkt->resp);

	if( !++sdr_reservation_id )
		sdr_reservation_id++;	

	resp->reservation_id_lsb = 0xff & sdr_reservation_id;
	resp->reservation_id_msb = sdr_reservation_id >> 8;
	resp->completion_code = CC_NORMAL;
	pkt->hdr.resp_data_len = 2;

}

/* 
Get Sensor Reading Command
This command returns the present reading for sensor. The sensor device may return
a stored version of a periodically updated reading, or the sensor device may scan
to obtain the reading after receiving the request.
The meaning of the state bits returned by Discrete sensors is based on the 
Event/Reading Type code from the SDR for the sensor. This can also be obtained
directly from the controller if the optional Get Sensor Type command is supported
for the sensor. Refer to Section 41.2 in the IPMI spec, Event/Reading Type Code,
for information on interpreting Event/Reading Type codes when used for present
readings.
*/ 
void
ipmi_get_sensor_reading( IPMI_PKT *pkt )
{
	GET_SENSOR_READING_CMD_REQ *req = ( GET_SENSOR_READING_CMD_REQ * )(pkt->req);
	GET_SENSOR_READING_RESP *resp = ( GET_SENSOR_READING_RESP * )(pkt->resp);
	int i, found = 0;
	
	/* Given the req->sensor_number return the reading */
	for( i = 0; i < current_sensor_count; i++ ) {
		if( sensor[i]->sensor_id == req->sensor_number ) {
			found++;
			break;
		}
	}

	/* if this is a non-periodically scanned sensor, call the sensor scan 
	 * function to update the sensor reading*/
	if( found && !sensor[i]->scan_period )
		sensor[i]->scan_function((void *)&sensor[i]);
	
	if( found ) {
		resp->completion_code = CC_NORMAL;
		resp->sensor_reading = sensor[i]->last_sensor_reading;
		resp->event_messages_enabled = sensor[i]->event_messages_enabled;
		resp->sensor_scanning_enabled = sensor[i]->sensor_scanning_enabled;
		resp->unavailable =  sensor[i]->unavailable;
		pkt->hdr.resp_data_len = 2;
	} else {
		resp->completion_code = CC_REQ_DATA_NOT_AVAIL;
       		pkt->hdr.resp_data_len = 0;
	}
}

/* 
Get Sensor Reading Factors Command
This command returns the Sensor Reading Factors fields for the specified reading
value on the specified sensor. It is used for retrieving the conversion factors
for non-linear sensors that do not fit one of the generic linearization formulas.
See Non-Linear Sensors section.

This command is provided for ‘analog’ sensor devices that are capable of holding
a table of factors for linearization, but are incapable of performing the 
linearization calculations itself. Sensors that produce linear readings, but have
non-linear accuracy or resolution over their range can also use this command.
Note: the Response Data is based on the Version and Type of sensor record for 
the sensor. Only Type 01h record information is presently defined.
*/
void
ipmi_get_sensor_reading_factors( IPMI_PKT *pkt )
{
	GET_SENSOR_READING_FACTORS_CMD *req = ( GET_SENSOR_READING_FACTORS_CMD * )(pkt->req);
	GET_SENSOR_READING_FACTORS_RESP *resp = ( GET_SENSOR_READING_FACTORS_RESP * )(pkt->resp);


	/* TODO given the req->sensor_number & req->reading byte return the reading */

	/* Next reading field indicates the next reading for which a different set of
	sensor reading factors is defined. If the reading byte passed in the request
	does not match exactly to a table entry, the nearest entry will be returned, and
	this field will hold the reading byte value for which an exact table match would
	have been obtained. Once the ‘exact’ table byte has been obtained, this field
	will be returned with a value such that, if the returned value is used as the
	reading byte for the next request, the process can be repeated to cycle
	through all the Sensor Reading Factors in the device’s internal table. This
	process shall ‘wrap around’ such a complete list of the table values can be
	obtained starting with any reading byte value. */
	/*
	resp->next_reading =	
	resp->M_lsb =
	resp->M_msb =
	resp->tolerance = 
	resp->B_lsb =
	resp->B_msb =
	resp->accuracy_lsb =
	resp->accuracy_msb =
	resp->accuracy_exp =
	resp->R_exponent =
	resp->B_exponent =
	*/

}
/*

IPM Controllers are required to maintain Device Sensor Data Records for the 
sensors and objects they manage.

After a FRU is inserted, the System Manager, using the Shelf Manager, may gather
the various SDRs from the FRU’s IPM Controller to learn of the various objects
and how to use them. The System Manager would use the “Sensor Device Commands”
(Get Device SDR Info, Get Device SDR, Reserve Device SDR Repository, Get Sensor 
Reading) to gather this information. Thus, commands, such as “Get Device SDR Info”
and “Get Device SDR”, which are optional in the IPMI specification, are mandatory
in AdvancedTCA® systems.

The implementer may choose to have the Shelf Manager gather the individual 
Device Sensor Data Records into a centralized SDR Repository. This SDR Repository
may exist in either the Shelf Manager or System Manager. If the Shelf Manager 
implements the SDR Repository on-board, it shall also respond to “SDR Repository”
commands (ie. Get SDR Repository Info, Reserve SDR Repository, Get SDR etc)
*/

/*======================================================================*/
/*
 *    SDR Repository Commands
 *
 *    Mandatory Commands
 *    	Get SDR Repository Info
 *    	Reserve SDR Repository
 *    	Get SDR
 *    	Exit SDR Repository Update Mode
 */
/*======================================================================*/
void
get_sdr_repository_info( IPMI_PKT *pkt )
{
	GET_SDR_REPOSITORY_INFO_CMD_RESP *resp = ( GET_SDR_REPOSITORY_INFO_CMD_RESP *)(pkt->resp);
		
	resp->sdr_version = 0x51; /* SDR Version - version number of the SDR
				   command set for the SDR Device.
				   51h for this specification. (BCD encoded 
				   with bits 7:4 holding the Least Significant
				   digit of the revision and bits 3:0 holding
				   the Most Significant bits.) */
	/* TODO create SDR repository infrastructure */
	/* fill in the number of records in the SDR Repository */
	resp->record_count_lsb = 0;	
	resp->record_count_msb = 0;

	/* fill in the Free Space in bytes 0000h indicates ‘full’, FFFEh indicates
	   64KB-2 or more available. FFFFh indicates ‘unspecified’. */
	resp->free_space_lsb = 0;
	resp->free_space_msb = 0;
	
	/* Most recent addition timestamp. */
	resp->most_recent_addition_timestamp[0] = 0;	/*  LS byte first. */
	resp->most_recent_addition_timestamp[1] = 0;
	resp->most_recent_addition_timestamp[2] = 0;
	resp->most_recent_addition_timestamp[3] = 0;

	/*  11:14 Most recent erase (delete or clear) timestamp. */ 
	resp->most_recent_erase[0] = 0;	/* LS byte first. */
	resp->most_recent_erase[1] = 0;
	resp->most_recent_erase[2] = 0;
	resp->most_recent_erase[3] = 0;
	
	/* Operation Support
	[7] - Overflow Flag. 1=SDR could not be written due to lack of space in the
	SDR Repository.
	[6:5] - 00b = modal/non-modal SDR Repository Update operation unspecified
	        01b = non-modal SDR Repository Update operation supported
		10b = modal SDR Repository Update operation supported
		11b = both modal and non-modal SDR Repository Update supported
	[4] - reserved. Write as 0b
	[3] - 1b=Delete SDR command supported
	[2] - 1b=Partial Add SDR command supported
	[1] - 1b=Reserve SDR Repository command supported
	[0] - 1b=Get SDR Repository Allocation Information command supported */
	resp->operation_support = 0;

	resp->completion_code = CC_NORMAL;
	pkt->hdr.resp_data_len = sizeof( GET_SDR_REPOSITORY_INFO_CMD_RESP ) - 1;
}


/*
Get SDR Command
Returns the sensor record specified by ‘Record ID’. The command also accepts a 
‘byte range’ specification that allows just a selected portion of the record to
be retrieved (incremental read). 

The Requester must first reserve the SDR Repository using the ‘Reserve SDR Repository’
command in order for an incremental read to an offset other than 0000h to be
accepted. (It is also recommended that an application use the Get SDR Repository
Info command to verify the version of the SDR Repository before it sends any 
other SDR Repository commands. This is important since the SDR Repository command
format and operation can change between versions.)

If ‘Record ID’ is specified as 0000h, this command returns the Record Header 
for the ‘first’ SDR in the repository. FFFFh specifies that the ‘last’ SDR in
the repository should be listed. If ‘Record ID’ is non-zero, the command returns
the information from the matching record, and the Record ID for the next SDR in
the repository.

An application that wishes to retrieve the full set of SDR Records must first
issue the Get SDR starting with 0000h as the Record ID to get the first record.
The Next Record ID is extracted from the response and this is then used as the
Record ID in a Get SDR request to get the next record. This is repeated until
the ‘Last Record ID’ value (FFFFh) is returned in the ‘Next Record ID’ field of
the response.

A partial read from offset 0000h into the record can be used to extract the 
header and associated ‘Key Fields’ for the specified Sensor Data Record in the 
SDR Repository. An application can use the command in this manner to get a list
of what records are in the SDR and to identify the instances of each type. It
can also be used to search for an particular sensor record.

Note: to support future extensions, applications should check the SDR Version
byte prior to interpreting any of the data that follows.

If you issue a Get SDR command (storage 23h) with a 'bytes to read' size of 
'FFh' - meaning 'read entire record'. Avalue of 'FFh' will cause an error in
most cases, since SDRs are bigger than the buffer sizes for the typical system
interface implementation. The controller therefore returns an error completion
code if the number of record bytes exceeds the maximum transfer length for the
interface. The completion code CAh that indicates that the number of requested
bytes cannot be returned. Returning this code is recommended, although a controller
could also return an 'FFh' completion code. In either case, the algorithm for
handling this situation is to "default to using partial reads if the 'read entire
record' operation fails" (that is, if you get a non-zero completion code).

Reading the SDR Repository
An application that retrieves records from the SDR Repository must first read
them out sequentially. This is accomplished by using the Get SDR command to 
retrieve the first SDR of the desired type. The response to this command returns
the requested record and the Record ID of the next SDR in sequence in the
repository. Note that Record IDs are not required to be sequential or consecutive.
Applications should not assume that SDR Record IDs will follow any particular
numeric ordering.

The application retrieves succeeding records by issuing a Get SDR command using
the ‘next’ Record ID that was returned with the response of the previous Get SDR
command. This is continued until the ‘End of Records’ ID is encountered.

Once the application has read out the desired records, it can then randomly access
the records according to their Record ID. An application that seeks to access 
records randomly must save a data structure that retains the Record Key information
according to Record ID. Since it is possible for Record IDs to change with time,
it is important for applications to first verify that the Record Key information
matches up with the retrieved record. If the Record Key information doesn’t match,
then the Record ID is no longer valid for that Record Key, and the SDR Records
must again be accessed sequentially until the record that matches the Record Key
is located.

An application can also tell whether records have changed by examining the ‘most
recent addition’ timestamp using the Get SDR Repository Info command.

If record information has changed, an application does not need to list out the
entire contents of all records. The Get SDR command allows a partial read of the
SDR. Thus, an application can search for a given Record Key by just retrieving 
that portion of the record.
 
SDR ‘Record IDs’
In order to generalize SDR access, Sensor Data Records are accessed using a 
‘Record ID’ number. There are a fixed number of possible Record IDs for a given
implementation of the SDR Repository.

The most common implementation of ‘Record IDs’ is as a value that translates
directly to an ‘index’ or ‘offset’ into the SDR Repository. However, it is also
possible for an implementation to provide a level of indirection, and implement
Record IDs as ‘handles’ to the Sensor Data Records.

Record ID values may be ‘recycled’. That is, the Record ID of a previously deleted
SDR can be used as the Record ID for a new SDR. The requirement is that, at any
given time, the Record IDs are unique for all SDRs in the repository.

Record IDs can be reassigned by the SDR Repository Device as needed when records
are added or deleted. An application that uses a Record ID to directly access a
record should always verify that the retrieved record information matches up 
with the ID information (slave address, LUN, sensor ID, etc.) of the desired
sensor. An application that finds that the SDR at a given ‘Record ID’ has moved
will need to re-enumerate the SDRs by listing them out using a series of Get SDR
commands. Note that it is not necessary to read out the full record data to see
if the Record ID for a particular record has changed. Software can determine
whether a given record has been given a different Record ID by examining just 
the SDR’s header and record key bytes.
*/
void
get_sdr( IPMI_PKT *pkt )
{
	GET_SDR_CMD_REQ *req = ( GET_SDR_CMD_REQ * )(pkt->req);
	GET_SDR_CMD_RESP *resp = ( GET_SDR_CMD_RESP * )(pkt->resp);

	/* if there is an incremental read to an offset other than 0000h
	 * the requestor should have reserved the SDR Repository */
//	req->reservation_id_lsb; 
//	req->reservation_id_msb;
//	req->record_id_lsb;	/* Record ID of record to Get, LS Byte */
//	req->record_id_msb;	/* Record ID of record to Get, MS Byte */
//	req->offset;		/* Offset into record */
//	req->bytes_to_read;	/* Bytes to read. FFh means read entire record. */


//	resp->completion_code;
//	resp->record_id_next_lsb;	/* Record ID for next record, LS Byte */
//	resp->record_id_next_msb;	/* Record ID for next record, MS Byte */
//	resp->record_data[20];	/* 4:3+N Record Data */
	
}

