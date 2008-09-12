/*
-------------------------------------------------------------------------------
coreIPM/amc.h

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
#define uint16_t unsigned short
#define uint32_t unsigned int


/* Table 3-3 Carrier Information Table - this is maintained on the carrier */
typedef struct carrier_information_table {
	uchar	record_type_id;	/* Record Type ID. For all records defined
				   in this specification a value of C0h (OEM)
				   shall be used. */
	uchar 	eol:1,		/* [7:7] End of list. Set to one for the last record */
	      	reserved:3,	/* [6:4] Reserved, write as 0h.*/
		version:4;	/* [3:0] record format version (2h for this definition) */
	uchar	record_len;	/* Record Length. */
	uchar	record_cksum;	/* Record Checksum. Holds the zero checksum of
				   the record. */
	uchar	header_cksum;	/* Header Checksum. Holds the zero checksum of
				   the header. */
	uchar	manuf_id[3];	/* Manufacturer ID. LS Byte first. Write as the
				   three byte ID assigned to PICMG®. For this
				   specification, the value 12634 (00315Ah) shall
				   be used. */
	uchar	picmg_rec_id;	/* PICMG Record ID. For the Shelf Power 
				   Distribution Record, the value 11h shall be
				   used. */
	uchar	rec_fmt_ver;	/* Record Format Version. For this specification,
				   the value 0h shall be used. */
	/* AMC.0 Extension Version. Indicates the version of AMC.0 specification
	   extensions implemented by the IPMC. IPMCs must report a value of 02h 
	   for AMC.0 R2.0 */
	uchar	amc_ext_ver_minor:4,	/* [7:4] = BCD encoded minor version */
		amc_ext_ver_major:4;	/* [3:0] = BCD encoded major version */
	uchar	site_num_count;	/* Carrier Site Number Count. Indicates the number
				   of entries in the Carrier Site Numbers array. */
	uchar	site_number_array[4];	/* Carrier Site Numbers. An array of 
				  Carrier Site Numbers. Each entry must be one
				  byte in length. The array contains a list of
				  the Site Numbers of all AMC Slots supported by
				  the Carrier. */
} CARRIER_INFORMATION_TABLE;


/* Table 3-10 Module Current Requirements record */
typedef struct module_current_requirements_record {
	uchar	rec_type_id;	/* Record Type ID. For all records 
				   defined in this specification,
				   a value of C0h (OEM) must be used. */
#ifdef BF_MS_FIRST
	uchar	end_list:1,	/* [7] – End of List. Set to one for 
				   the last record */
		:3,		/* [6:4] – Reserved, write as 0h */
		rec_format:4;	/* [3:0] – Record format version 
				   (= 2h for this definition) */
#else
	uchar	rec_format:4,
			:3,
			end_list:1;
#endif
	uchar	rec_length;	/* Record Length */
	uchar	rec_cksum;	/* Record Checksum. Holds the zero 
				   checksum of the record. */
	uchar	hdr_cksum;	/* Header Checksum. Holds the zero 
				   checksum of the header. */
	uchar	manuf_id_lsb;	/* Manufacturer ID. Least significant 
				   byte first. Write as the three byte ID
				   assigned to PICMG. For this specification
				   the value 12634 (00315Ah) must be used. */
	uchar	manuf_id_midb;
	uchar	manuf_id_msb;
	uchar	picmg_rec_id;	/* PICMG Record ID. For the Module Power Descriptor
				   table, the value 16h must be used. */
	uchar	rec_fmt_ver;	/* Record Format Version. For this specification, 
				   the value 0h must be used. */
	uchar	curr_draw;	/* Current Draw. This field holds the Payload 
				   Power (PWR) requirement of the Module given
				   as current requirement in units of 0.1A at 12V. 
				   (This equals the value of the power in W 
				   divided by 1.2.) */
} MODULE_CURRENT_REQUIREMENTS_RECORD;


/* AMC Table 3-12 Module Activation and Current Descriptor */
typedef struct module_activation_current_descr {
	uchar	local_ipmb_addr;
				/* Local IPMB Address. This is IPMB-L address of
				   the AMC Slot. The order of this record in the
				   table determines the power-on sequencing.*/
	uchar	max_module_current;	
				/* Maximum Module Current. This field holds the
				   value for the maximally allowed power draw by
				   the AMC Slot identified by the Local IPMB Address,
				   given as a current value in 0.1A at 12V. (This
				   equals the value of the maximum power in W 
				   divided by 1.2.) */
	uchar	params;		/* This byte is intended to contain activation
				   and power configuration parameters for the AMC
				   Slot identified by Local IPMB Address. This
				   byte is not implemented in the current version
				   of the specification. This byte must be set 
				   to FFh. */
} MODULE_ACTIVATION_CURRENT_DESCR;



/* AMC Table 3-11 Carrier Activation and Current Management record */
typedef struct carrier_activation_current_mgmt_record {
	uchar	record_type_id;	/* Record Type ID. For all records defined
				   in this specification a value of C0h (OEM)
				   shall be used. */
#ifdef BF_MS_FIRST
	uchar 	eol:1,		/* [7:7] End of list. Set to one for the last record */
	      	reserved:3,	/* [6:4] Reserved, write as 0h.*/
		version:4;	/* [3:0] record format version (2h for this definition) */
#else
	uchar	version:4,
		reserved:3,
		eol:1;
#endif
	uchar	record_len;	/* Record Length. */
	uchar	record_cksum;	/* Record Checksum. Holds the zero checksum of 
				   the record. */
	uchar	header_cksum;	/* Header Checksum. Holds the zero checksum of 
				   the header. */
	uchar	manuf_id[3];	/* Manufacturer ID. LS Byte first. Write as the
				   three byte ID assigned to PICMG®. For this
				   specification, the value 12634 (00315Ah) shall
				   be used. */
	uchar	picmg_rec_id;	/* PICMG Record ID. For the Shelf Power 
				   Distribution Record, the value 11h shall be
				   used. */
	uchar	rec_fmt_ver;	/* Record Format Version. For this specification,
				   the value 0h shall be used. */
	uchar	max_internal_current_lsb;
	uchar	max_internal_current_msb;
				/* Maximum Internal Current. Least significant
				   byte first. This field holds the value of the
				   total Payload Power (PWR) available on the 
				   Carrier to the entire set of AMC Slots, given
				   as a current value in units of 0.1A at 12V.
				   (This equals the value of the available power
				   in W divided by 1.2.) */
	uchar	t_allowance;	/* Allowance for Module Activation Readiness.
				   This field contains the number of seconds after
				   Carrier start-up that Modules have to transition
				   to state M3 and maintain their power up sequence
				   position. */
	uchar	entry_count;	/* Module Activation and Current Descriptor Count.
				   This contains a count of the number of entries
				   (M) in the Module Activation and Current 
				   Descriptor Table. */
	MODULE_ACTIVATION_CURRENT_DESCR descriptor[4];
				/* Module Activation and Current Descriptors. 
				   This is an array of activation and current
				   descriptors for each AMC Slot implemented on the
				   Carrier. Each descriptor is 3 bytes in size and
				   follows the format found in Table 3-12,
				   “Module Activation and Current Descriptor.” */
} CARRIER_ACTIVATION_CURRENT_MGMT_RECORD;




/* AMC Table 3-15 Point-to-Point Port Descriptor */
typedef struct p2p_port_descriptor {
#ifdef BF_MS_FIRST
	uint16_t 	:6,	/* [23:18] Reserved. Must be 0. */
		local_port:5,	/* [17:13] Local Port. Indicates the Port number
				   within the local AMC Slot or on-Carrier device. */
		remote_port:5;	/* [12:8]  Remote Port. Indicates the Port number
				   within the remote AMC Slot or on-Carrier device
				   ID to which this point-to-point connection
				   is routed. */
				/* [7:0] Remote Resource ID. In AMC.0 systems: */
#else
	uint16_t remote_port:5,
		local_ports:5,
		:6;
#endif
#ifdef BF_MS_FIRST
	uchar	resource_type:1,/* [7] Resource Type.
				   	1 AMC, 
					0 indicates on-Carrier device */
		:3,		/* [6:4] Reserved; write 0h */
		amc_site_num:4;	/* [3:0] On-Carrier device ID or AMC Site Number */
#else
	uchar	amc_site_num:4,
		:3,
		resource_type:1;
#endif
} P2P_PORT_DESCRIPTOR;





/* AMC Table 3-14 Point-to-Point AdvancedMC Resource Descriptor */
typedef struct p2p_amc_resource_descr {
	uchar	resource_id;	/* Resource ID. Indicates the AMC Slot ID or 
				   on-Carrier device. */
#ifdef BF_MS_FIRST
	uchar	resource_type:1, /* [7] Resource Type. 
				    	1 AMC, 
					0 indicates on-Carrier device ID */
		:3,		/* [6:4] Reserved; write 0h */
		dev_id:4;	/* [3:0] On-Carrier device ID or AMC Site Number */
#else
	uchar	dev_id:4,
		:3,
		resource_type:1;
#endif
	       uchar	p2p_port_count;	/* Point-to-Point Port Count. Indicates the number
				   of point-to-point Ports associated with this 
				   Resource. */
	P2P_PORT_DESCRIPTOR	p2p_port[8];
				/* 3*n Point-to-Point Port Descriptors. An array 
				   of n Point-to-Point Port Descriptors (each with
				   Least significant byte first) (see Table 3-15)
				   where n is specified in the Point-to-Point Port
				   Count byte. TODO check size of array */
} P2P_AMC_RESOURCE_DESCR;



/* AMC Table 3-13 Carrier Point-to-Point Connectivity record  */

typedef struct carrier_p2p_conn_record {
	uchar	record_type_id;	/* Record Type ID. For all records defined
				   in this specification a value of C0h (OEM)
				   shall be used. */
#ifdef BF_MS_FIRST
	uchar 	eol:1,		/* [7:7] End of list. Set to one for the last record */
	      	reserved:3,	/* [6:4] Reserved, write as 0h.*/
		version:4;	/* [3:0] record format version (2h for this definition) */
#else
	uchar	version:4,
		reserved:3,
		eol:1;
#endif
	uchar	record_len;	/* Record Length. */
	uchar	record_cksum;	/* Record Checksum. Holds the zero checksum of
				   the record. */
	uchar	header_cksum;	/* Header Checksum. Holds the zero checksum of
				   the header. */
	uchar	manuf_id[3];	/* Manufacturer ID. LS Byte first. Write as the
				   three byte ID assigned to PICMG®. For this
				   specification, the value 12634 (00315Ah) shall
				   be used. */
	uchar	picmg_rec_id;	/* PICMG Record ID. For the Shelf Power 
				   Distribution Record, the value 11h shall be
				   used. */
	uchar	rec_fmt_ver;	/* Record Format Version. For this specification,
				   the value 0h shall be used. */
	P2P_AMC_RESOURCE_DESCR	resource_descr_list; 
				/* Point-to-Point AMC Resource Descriptor List. 
				   A list of variable length Point-to-Point AMC
				   Resource Descriptors (see Table 3-14) totaling
				   m bytes in length. Each Point-to-Point AMC 
				   Resource Descriptor describes the number of 
				   Ports and the connectivity thereof from one
				   AMC Slot or on-Carrier device. */
} CARRIER_P2P_CONN_RECORD;




/* AMC Table 3-19 AMC Link Descriptor. TODO: fix bit packing */
typedef struct amc_link_descr {
	uchar	reserved:6,	/* [39:34] Reserved. Must be 111111b = 3f. */
		asym_match:2;	/* [33:32] AMC Asymmetric Match. Indicates
				   whether exact or asymmetric match is required,
				   and if asymmetric further defines which end of
				   an asymmetric Link this Descriptor represents.
				   See Table 3-18. */
	uchar	link_grouping_id;
				/* [31:24] Link Grouping ID. Indicates whether the
				   Ports of this Channel are operated together
				   with Ports in other Channels. A value of 0 always
				   indicates a Single-Channel Link. A common, non-zero
				   Link Grouping ID in multiple Link Descriptors
				   indicates that the Ports covered by those Link
				   Descriptors must be operated together. A unique
				   non-zero Link Grouping ID also indicates Single-
				   Channel Link. */
	uint32_t link_type_ext:4,
				/* [23:20] AMC Link Type Extension. Identifies the
				   subset of a subsidiary specification that is
				   implemented and is defined entirely by the 
				   subsidiary specification identified in the Link
				   Type field. */
		link_type:8,	/* [19:12] AMC Link Type. Identifies the AMC.x
				   subsidiary specification that governs this
				   description or identifies the description as
				   proprietary; see Table 3-21, “AMC Link Type.” */
				/* [11:0] AMC Link Designator. Identifies the AMC
				   Channel and the Ports within the AMC Channel
				   that are being described; see Table 3-20,
				   “AMC Link Designator.” */
		/* Bit flag values: 1 = Lane Included; 0 = Lane Excluded) */
		lane_3_bit_flag:1, /* [11] Lane 3 Bit Flag */
		lane_2_bit_flag:1, /* [10] Lane 3 Bit Flag */
		lane_1_bit_flag:1, /* [9] Lane 3 Bit Flag */
		lane_0_bit_flag:1, /* [8] Lane 3 Bit Flag */
		amc_channel_id:8;  /* [7:0] AMC Channel ID. Identifies an AMC
				      Channel Descriptor defined in an AMC 
				      Point-to-Point Connectivity record. */
} AMC_LINK_DESCR;




/* AMC Table 3-17 AMC Channel Descriptor */
typedef struct amc_channel_descriptor {
#ifdef BF_MS_FIRST
	uint32_t reserved:4,	/* 23:20 Reserved. Must be 1111b. */
		lane_3_port_num:5, /* [19:15] Lane 3 Port Number. 
				   The Port within this AMC resource that 
				   functions as Lane 3 of this AMC Channel. */
		lane_2_port_num:5, /* [14:10] Lane 2 Port Number.
				   The Port within this AMC resource that
				   functions as Lane 2 of this AMC Channel. */
		lane_1_port_num:5, /* [9:5] Lane 1 Port Number. 
				   The Port within this AMC resource that
				   functions as Lane 1 of this AMC Channel. */
		lane_0_port_num:5, /* [4:0] Lane 0 Port Number. 
				   The Port within this AMC resource that
				   functions as Lane 0 of this AMC Channel. */
		:8;
#else
	uint32_t lane_0_port_num:5,
		lane_1_port_num:5,
		lane_2_port_num:5,
		lane_3_port_num:5,
		reserved:4,
		:8;
#endif
} AMC_CHANNEL_DESCRIPTOR;





/* AMC Table 3-16 AdvancedMC Point-to-Point Connectivity record */
typedef struct amc_p2p_conn_record {
	uchar	record_type_id;	/* Record Type ID. For all records defined
				   in this specification a value of C0h (OEM)
				   shall be used. */
#ifdef BF_MS_FIRST
	uchar 	eol:1,		/* [7:7] End of list. Set to one for the last record */
	      	reserved:3,	/* [6:4] Reserved, write as 0h.*/
		version:4;	/* [3:0] record format version (2h for this definition) */
#else
	uchar	version:4,
		reserved:3,
		eol:1;
#endif
	uchar	record_len;	/* Record Length. # of bytes following rec cksum */
	uchar	record_cksum;	/* Record Checksum. Holds the zero checksum of
				   the record. */
	uchar	header_cksum;	/* Header Checksum. Holds the zero checksum of 
				   the header. */
	uchar	manuf_id[3];	/* Manufacturer ID. LS Byte first. Write as the
				   three byte ID assigned to PICMG®. For this
				   specification, the value 12634 (00315Ah) shall
				   be used. */
	uchar	picmg_rec_id;	/* PICMG Record ID. For the AMC Point-to-Point
				   Connectivity record, the value 19h must be used  */
	uchar	rec_fmt_ver;	/* Record Format Version. For this specification,
				   the value 0h shall be used. */
	uchar	oem_guid_count;	/* OEM GUID Count. The number, n, of OEM GUIDs
				   defined in this record. */
//TODO	OEM_GUID oem_guid_list[n];
				/* A list 16*n bytes of OEM GUIDs. */
#ifdef BF_MS_FIRST
	uchar	record_type:1,	/* [7] Record Type – 1 AMC Module, 0 On-Carrier device */
		:3,		/* [6:4] Reserved; write as 0h */
		conn_dev_id:4;	/* [3:0] Connected-device ID if Record Type = 0,
				   Reserved, otherwise. */
#else
	uchar	conn_dev_id:4,
		:3,
		record_type:1;
#endif
	uchar	ch_descr_count;	/* AMC Channel Descriptor Count. The number, m, 
				   of AMC Channel Descriptors defined in this record. */
//TODO	AMC_CHANNEL_DESCR ch_descr[m]; 
				/* AMC Channel Descriptors. A variable length 
				   list of m three-byte AMC Channel Descriptors,
				   each defining the Ports that make up an AMC
				   Channel (least significant byte first).*/
//TODO	AMC_LINK_DESCR link_desrc[p];
				/* AMC Link Descriptors. A variable length list
				   of p five-byte AMC Link Descriptors (Least
				   significant byte first) (see Table 3-19, “AMC
				   Link Descriptor”, Table 3-20, “AMC Link Designator”,
				   and Table 3-21, “AMC Link Type”) totaling 5 * p
				   bytes in length. The value of p and the length
				   of the list are implied by Record Length, since
				   the list is at the end of this record.
				   Each AMC Link Descriptor details one type of
				   point-to-point protocol supported by the
				   referenced Ports. */
} AMC_P2P_CONN_RECORD;




/* Table 3-21 AMC Link Type */
/*
0x00	Reserved
0x01	Reserved
*/
#define AMC_LINK_PCI_EXPRESS		0x02	// AMC.1 PCI Express 
#define AMC_LINK_PCI_EXPRESS_AS		0x03	// AMC.1 PCI Express Advanced Switching
#define AMC_LINK_PCI_EXPRESS_AS2	0x04	// AMC.1 PCI Express Advanced Switching
#define AMC_LINK_ETHERNET		0x05	// AMC.2 Ethernet
#define AMC_LINK_SERIAL_RAPID_IO	0x06	// AMC.4 Serial RapidIO
#define AMC_LINK_STORAGE		0x07	// AMC.3 Storage
/*
0x08...0xEF	Reserved
0xF0...0xFE	E-Keying OEM GUID DefinitionFFhReserved
*/

/*
3.14 AMC.0 FRU records, sensors, and entity IDs

Table 3-50 PICMG AMC.0 FRU records: type ID = C0h (OEM). 
Records present in Module FRU.

Record description	PICMG AMC.0 			PICMG record ID
-----------------------------------------------------------------------
Module Current		Table 3-10, “Module Current	16h		
Requirements		Requirements record” 				

AMC Point-to-Point	Table 3-16, “AdvancedMC 	19h 	
Connectivity		Point-to-Point Connectivity
			record”
			
Clock configuration 	Table 3-35, “Clock 		2Dh 
			Configuration record” 



Table 3-51 PICMG AMC.0 sensors: Event/ reading type code = sensor specific (6Fh)

Sensor description	PICMG AMC.0			Sensor type code
------------------------------------------------------------------------
Module Hot Swap		Table 3-8, “Module Hot Swap	F2h
			event message”


			
Table 3-52 PICMG AMC.0 entity IDs
			
Entity type description		Entity ID
-----------------------------------------
PICMG AMC Module		C1h
*/




/* Table 3-27 Set AMC Port State command */
typedef struct set_amc_port_state_cmd_req {
	uchar	command;
	uchar	picmg_id;		/* PICMG Identifier. Indicates that 
					   this is a PICMG®-defined group
					   extension command. A value of
					   00h must be used. */
					/* Link Info fields.  Describes the
					   Link that should be enabled or disabled. */
#ifdef BF_MS_FIRST
	unsigned link_grp_id:8,		/* [31:24] – Link Grouping ID */
		link_type_ext:4,	 /*[23:20] – Link Type Extension */
		link_type:8,		/* [19:12] – Link Type */
		lane_3_bit_flag:1,	/* [11] – Lane 3 Bit Flag */
		lane_2_bit_flag:1,	/* [10] – Lane 2 Bit Flag */
		lane_1_bit_flag:1,	/* [9] – Lane 1 Bit Flag */
		lane_0_bit_flag:1,	/* [8] – Lane 0 Bit Flag */
		amc_channel_id:8;	/* [7:0] – AMC Channel ID */
#else
	unsigned amc_channel_id:8,
		lane_0_bit_flag:1,	
		lane_2_bit_flag:1,
		lane_1_bit_flag:1,
		lane_3_bit_flag:1,				
		link_type:8,
		link_type_ext:4,
		link_grp_id:8;				      
#endif
	uchar	state;			/* State. Indicates the desired state of the
					   Link as described by Link Info.
					   	00h = Disable
					   	01h = Enable 
					   All other values reserved. */
					/* This field is present if AMC Channel
					   ID is associated with an on-Carrier 
					   device, absent otherwise. */
#ifdef BF_MS_FIRST
	uchar	:4,			/* [7:4] Reserved; write as 0h */
		on_carrier_dev_id:4;	/* [3:0] On-Carrier device ID. 
					   Identifies the on-Carrier device
					   to which the described AMC Channel
					   is connected. */
#else
	uchar	on_carrier_dev_id:4,
		:4;
#endif
} SET_AMC_PORT_STATE_CMD_REQ;


typedef struct set_amc_port_state_cmd_resp {
	uchar	completion_code;	/* Completion Code. */
	uchar	picmg_id;		/* PICMG Identifier. Indicates that 
					   this is a PICMG®-defined group
					   extension command. A value of 
					   00h shall be used. */
} SET_AMC_PORT_STATE_CMD_RESP;


typedef struct amc_port_state {
	unsigned link_grp_id:8,		/* [31:24] – Link Grouping ID */
		link_type_ext:4,	 /*[23:20] – Link Type Extension */
		link_type:8,		/* [19:12] – Link Type */
		lane_3_bit_flag:1,	/* [11] – Lane 3 Bit Flag */
		lane_2_bit_flag:1,	/* [10] – Lane 2 Bit Flag */
		lane_1_bit_flag:1,	/* [9] – Lane 1 Bit Flag */
		lane_0_bit_flag:1,	/* [8] – Lane 0 Bit Flag */
		amc_channel_id:8;	/* [7:0] – AMC Channel ID */
	uchar	state;			/* State. Indicates the desired state of the
					   Link as described by Link Info.
					   	00h = Disable
					   	01h = Enable 
					   All other values reserved. */
					/* This field is present if AMC Channel
					   ID is associated with an on-Carrier 
					   device, absent otherwise. */
	uchar	:4,			/* [7:4] Reserved; write as 0h */
		on_carrier_dev_id:4;	/* [3:0] On-Carrier device ID. 
					   Identifies the on-Carrier device
					   to which the described AMC Channel
					   is connected. */
} AMC_PORT_STATE;




/* Table 3-28 Get AMC Port State command */
typedef struct get_amc_port_state_cmd_req {
	uchar	command;
	uchar	picmg_id;		/* PICMG Identifier. Indicates that 
					   this is a PICMG®-defined group
					   extension command. A value of
					   00h must be used. */
	uchar	amc_channel_id;		/* AMC Channel ID. Identifies the AMC
					   Channel being queried. */
					/* The following field is present if AMC
					   Channel ID is associated with an 
					   on-Carrier device, absent otherwise. */
#ifdef BF_MS_FIRST
	uchar	:4,			/* [7:4] Reserved; write as 0h */
		on_carrier_device_id:4;	/* [3:0] On-Carrier device ID. Identifies
					   the on-Carrier device to which the
					   described AMC Channel is connected. */
#else
	uchar	on_carrier_device_id:4,
		:4;
#endif
} GET_AMC_PORT_STATE_CMD_REQ;




typedef struct amc_link_info {
	uchar	link_grouping_id;	/* [23:16] – Link Grouping ID */
#ifdef BF_MS_FIRST
	uint16_t link_type_extension:4,	/* [15:12] – Link Type Extension */
		link_type:8,		/* [11:4] – Link Type */
		lane_3_port:1,		/* [3] – Lane 3 Port */
		lane_2_port:1,		/* [2] – Lane 2 Port */
		lane_1_port:1,		/* [1] – Lane 1 Port */
		lane_0_port:1;		/* [0] – Lane 0 Port */
#else
	uint16_t lane_0_port:1,
		lane_1_port:1,
		lane_2_port:1,
		lane_3_port:1,
		link_type:8,
		link_type_extension:4;
#endif
} AMC_LINK_INFO;




typedef struct get_amc_port_state_cmd_resp {
	uchar	completion_code;	/* Completion Code. */
	uchar	picmg_id;		/* PICMG Identifier. Indicates that 
					   this is a PICMG®-defined group
					   extension command. A value of 
					   00h shall be used. */
					/* The following bytes are optional */
	AMC_LINK_INFO	link_info1;	/* Bytes 3-5: Link info 1. Least significant
					   byte first. This optional field 
					   describes information about the first
					   Link associated with the specified AMC
					   Channel ID, if any. If this set of 
					   bytes is not provided, the AMC Channel
					   ID does not have any defined Link. */
	uchar	state1;			/* State 1. Must be present if Link 
					   Info 1 is present. Indicates the
					   first state of the Link.
					   	00h = Disable
						01h = Enable
					   All other values reserved. */
	AMC_LINK_INFO	link_info2;	/* (7:9)Link Info 2. Least significant
					   byte first. Optional. Bit assignments
					   identical to Link Info 1. Used for
					   cases where a second Link has been
					   established. */
	uchar	state2;			/* (10)State 2. Bit assignments identical to 
					   State 1. */
	AMC_LINK_INFO	link_info3;	/* (11:13)Link Info 3. Least significant
					   byte first. Optional. Bit assignments
					   identical to Link Info 1. Used for
					   cases where a third Link has been 
					   established. */
	uchar	state3;			/* (14)State 3. Bit assignments identical 
					   to State 1. */
	AMC_LINK_INFO	link_info4;	/* (15:17)Link Info 4. Least significant
					   byte first. Optional. Bit assignments
					   identical to Link Info 1. Used for 
					   cases where a fourth Link has been 
					   established. */
	uchar	state4;			/* (18)State 4. Bit assignments identical 
					   to State 1. */
} GET_AMC_PORT_STATE_CMD_RESP;




/* The “Get Clock State” command is used to query the current state of a clock 
 * resource from its respective management controller. For Carriers, the clocks
 * are identified by combining the Clock Resource ID with the Clock ID. For AMC
 * Modules, the AMC clocks are identified by the Clock ID. For any enabled clock,
 * the “Get Clock State” command provides the clock state as well as the clock
 * attributes in the response. If the clock is disabled, the response reports
 * that the clock is disabled with the remaining fields not present. 
 * Table 3-45, “ Get Clock State command” describes the details of this command.
 */

/* Table 3-44 Set Clock State command */
typedef struct set_clock_state_cmd_req {
	uchar	command;
	uchar	picmg_id;		/* PICMG Identifier. Indicates that 
					   this is a PICMG®-defined group
					   extension command. A value of
					   00h must be used. */
	uchar	clock_id;		/* Clock ID. Identifies the clock
					   being configured. See Table 3-33,
					   “Predefined Clock IDs for AMC clocks”
					   and Table 3-34, “Predefined Clock IDs
					   for ATCA Backplane clocks.” */
	uchar	clock_config_index;	/* Clock Configuration Descriptor Index.
					   This field identifies one element in
					   an array of Direct or Indirect Clock
					   descriptors in a particular Clock
					   Configuration descriptor. The Clock
					   Configuration descriptor is uniquely
					   identified by the Clock Resource ID
					   field, if present, and the Clock ID. */
					/* Clock Setting */
#ifdef BF_MS_FIRST
	uchar	:4,			/* [7:4] Reserved, write as 0h. */
		clock_state:1,		/* [3] - Clock State 
					   	0b = Disable 
						1b = Enable */
		clock_direction:1,	/* [2] - Clock Direction 
					   	0b = Clock receiver 
						1b = Clock source */
		pll_control:2;		/* [1:0] - PLL Control 
					   	00b = Default state (Command
						receiver decides the state) 
						01b = Connect through PLL 10b = 
						Bypass PLL (No action if no PLL used)
					       	11b = Reserved */
#else
	uchar	pll_control:2,
		clock_direction:1,
		clock_state:1,
		:4;
#endif
	uchar	clock_family;		/* (5)Clock Family. See Table 3-39, “Clock
					   Family definition.” Present if the clock
					   is enabled, otherwise absent. */
	uchar	clock_accuracy_level;	/* (6)Clock Accuracy Level. This field
					   has different definitions, depending
					   on the Clock Family. Present if the
					   clock is enabled, otherwise absent. */
	unsigned clock_freq;		/* (7-10)Clock Frequency in Hz. Least 
					   Significant Byte First. Present if 
					   the clock is enabled, otherwise absent. */
	uchar	clock_resource_id;	/* (11)Clock Resource ID. Present if Clock
					   ID is associated with an on-Carrier
					   device or ATCA Backplane clocks, absent
					   otherwise. See Table 3-31, “Clock 
					   Resource ID definition.” */
} SET_CLOCK_STATE_CMD_REQ;


typedef struct set_clock_state_cmd_resp {
	uchar	completion_code;	/* Completion Code. */
	uchar	picmg_id;		/* PICMG Identifier. Indicates that 
					   this is a PICMG®-defined group
					   extension command. A value of 
					   00h shall be used. */
} SET_CLOCK_STATE_CMD_RESP;




/*
REQ 3.182 Carriers and Modules shall support the “Set Clock State” command 
defined in Table 3-44, “ Set Clock State command” for all implemented clocks.
REQ 3.183 When the clock E-Keying process is performed by the Carrier IPMC,
the PLL Control field in the “Set Clock State” command shall be set to 
“Default state (00b)”.
*/

/* Table 3-45 Get Clock State command */

typedef struct get_clock_state_cmd_req {
	uchar	command;
	uchar	picmg_id;		/* PICMG Identifier. Indicates that 
					   this is a PICMG®-defined group
					   extension command. A value of
					   00h must be used. */
	uchar	clock_id;		/* Clock ID. Identifies the clock being
					   queried. See Table 3-33, “Predefined
					   Clock IDs for AMC clocks” and Table
					   3-34, “Predefined Clock IDs for ATCA
					   Backplane clocks.” */
	uchar	clock_resource_id;	/* Clock Resource ID. Present if Clock
					   ID is associated with an on-Carrier
					   device or ATCA Backplane clocks,
					   absent otherwise. See Table 3-31,
					   “Clock Resource ID definition.” */
} GET_CLOCK_STATE_CMD_REQ;




typedef struct get_clock_state_cmd_resp {
	uchar	completion_code;	/* Completion Code. */
	uchar	picmg_id;		/* PICMG Identifier. Indicates that 
					   this is a PICMG®-defined group
					   extension command. A value of 
					   00h shall be used. */
					/* Clock Setting */
#ifdef BF_MS_FIRST
	uchar	:4,			/* [7:4] Reserved, write as 0h. */
		clock_state:1,		/* [3] - Clock State
					   	0b = Disable
						1b = Enable */
		clock_direction:1,	/* [2] - Clock Direction
					   	0b = Clock receiver
						1b = Clock source */
		pll_control:2;		/* [1:0] - PLL Control
					   	00b = Default state (Command 
						      receiver decides the state)
						01b = Connect through PLL
						10b = Bypass PLL (No action if 
						      no PLL used)
						11b = Reserved */
#else
	uchar	pll_control:2,
		clock_direction:1,
		clock_state:1,
		:4;
#endif
			    
	uchar	clock_config_index;	/* (4) Clock Configuration Descriptor Index.
					   Present if the clock is enabled, 
					   otherwise absent. */
	uchar	clock_family;		/* (5) Clock Family 
					   See Table 3-39, “Clock Family 
					   definition.” Present if the clock
					   is enabled, otherwise absent. */
	uchar	clock_accuracy_level;	/* (6) Clock Accuracy Level.
					   Present if the clock is enabled,
					   otherwise absent. */
					/* The following field is defined based
					   on the Clock Family. */
	unsigned clock_frequency;	/* (7-10) Clock Frequency in Hz. 
					   Least Significant Byte First.
					   Present if clock is enabled,
					   otherwise absent. */
} GET_CLOCK_STATE_CMD_RESP;


/* Table 3-26 (PICMG 3.0 Revision 3.0) FRU Control Capabilities command */

typedef struct fru_control_capabilities_cmd_req {
	uchar	command;
	uchar	picmg_id;		/* PICMG Identifier. Indicates that 
					   this is a PICMG®-defined group
					   extension command. A value of
					   00h must be used. */
	uchar	fru_dev_id;		/* FRU Device ID */
} FRU_CONTROL_CAPABILITIES_CMD_REQ;


typedef struct fru_control_capabilities_cmd_resp {
	uchar	completion_code;	/* Completion Code. */
	uchar	picmg_id;		/* PICMG Identifier. Indicates that 
					   this is a PICMG®-defined group
					   extension command. A value of 
					   00h shall be used. */
	/* FRU Control Capabilities Mask*/
	uchar	:4,			/* [4-7] - Reserved */
		diag_int:1,		/* [33] - 1b - Capable of issuing a diagnostic interrupt */
		graceful_reboot:1,	/* [2] - 1b - Capable of issuing a graceful reboot */
		warm_reset:1, 		/* [1] - 1b - Capable of issuing a warm reset */
		:1;			/* [0] - Reserved */
} FRU_CONTROL_CAPABILITIES_CMD_RESP;





