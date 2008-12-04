
/*
-------------------------------------------------------------------------------
coreIPM/req.h

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


typedef struct req_params {
	void(*completion_function)( void *, int );
	unsigned char req_data_len;

	// info about the device we're directly talking to
	unsigned char outgoing_protocol;
	unsigned char outgoing_medium;
	unsigned char addr_out[4];
	unsigned char netfn;

	// info about the target device if bridged
	unsigned char bridged;
	unsigned char bridge_protocol;
	unsigned char bridge_medium;
	unsigned char bridge_addr[4];
} REQ_PARAMS;

int req_send( GENERIC_CMD_REQ *cmd_req, REQ_PARAMS *params );

