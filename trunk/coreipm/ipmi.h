/*
-------------------------------------------------------------------------------
coreIPM/ipmi.h

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

#define uchar unsigned char

/*======================================================================*/
/*			General Power State				*/
/*======================================================================*/
#define POWER_STATE_OFF		0
#define POWER_STATE_ON		1
#define POWER_STATE_SLP		3
#define POWER_STATE_S1		4
#define POWER_STATE_S2		5
#define POWER_STATE_S3		6
#define POWER_STATE_S4		7

#define IPMI_SENSOR_HOT_SWAP	0xf0	
#define IPMI_SENSOR_MODULE_HOT_SWAP	0xf2	
#define IPMI_EVENT_TYPE_GENERIC_AVAILABILITY	0x6f

/*======================================================================*/
/*			Network Function Codes				*/
/*======================================================================*/

/* 00h identifies the message as a command/request and 01h as a
response, relating to the common chassis control and status functions. */
#define NETFN_CHASSIS_REQ	0x00
#define NETFN_CHASSIS_RESP	0x01

/* 02h (request) or 03h (response) identifies the message as containing
data for bridging to the next bus. This data is typically another
message, which may also be a bridging message. This function is
present only on bridge nodes. */
#define NETFN_BRIDGE_REQ	0x02
#define NETFN_BRIDGE_RESP	0x03

/* This functionality can be present on any node. 04h identifies the
message as a command/request and 05h as a response, relating to
the configuration and transmission of Event Messages and system
Sensors. */
#define NETFN_EVENT_REQ		0x04
#define NETFN_EVENT_RESP	0x05

/* 06h identifies the message as an application command/request and
07h a response. The exact format of application messages is
implementation-specific for a particular device, with the exception of
App messages that are defined by the IPMI specifications.
Note that it is possible that new versions of this specification will
identify new App commands. To avoid possible conflicts with future
versions of this specification, it is highly recommended that the
OEM/Group network functions be used for providing ‘value added’
functions rather than the App network function code. */
#define	NETFN_APP_REQ		0x06
#define	NETFN_APP_RESP		0x07

/* The format of firmware transfer requests and responses matches the
format of Application messages. The type and content of firmware
transfer messages is defined by the particular device. */
#define NETFN_FW_REQ		0x08
#define NETFN_FW_RESP		0x09

/* This functionality can be present on any node that provides nonvolatile
storage and retrieval services. */
#define NETFN_NVSTORE_REQ	0x0A
#define NETFN_NVSTORE_RESP	0x0B

/* Requests (0Ch) and responses (0Dh) for IPMI-specified messages that
are media-specific configuration and operation, such as configuration
of serial and LAN interfaces. */
#define NETFN_MEDIA_SPECIFIC_REQ	0x0C
#define NETFN_MEDIA_SPECIFIC_RESP	0x0D


/* 0Eh-2Bh Reserved - reserved (30 Network Functions [15 pairs]) */


/* Group Extension Network Function
The first data byte position in requests and responses under this
network function identifies the defining body that specifies command
functionality. Software assumes that the command and completion
code field positions will hold command and completion code values.
The following values are used to identify the defining body:

  00h** PICMG - PCI Industrial Computer Manufacturer’s Group.

  01h DMTF Pre-OS Working Group ASF Specification

  all other Reserved

When this network function is used, the ID for the defining body
occupies the first data byte in a request, and the second data byte
(following the completion code) in a response. */
#define NETFN_GROUP_EXTENSION_REQ	0x2C
#define NETFN_GROUP_EXTENSION_RESP	0x2D


/* Functions defined in the PICMG v3 specification. */
#define NETFN_PICMG_REQ			0x2C
#define NETFN_PICMG_RESP		0x2D

/* OEM/Group OEM/Non-IPMI group Requests and Response
The first three data bytes of requests and responses under this
network function explicitly identify the OEM or non-IPMI group that
specifies the command functionality. While the OEM or non-IPMI group
defines the functional semantics for the cmd and remaining data fields,
the cmd field is required to hold the same value in requests and
responses for a given operation in order to be supported under the
IPMI message handling and transport mechanisms.
When this network function is used, the IANA Enterprise Number for
the defining body occupies the first three data bytes in a request, and
the first three data bytes following the completion code position in a
response. */
#define NETFN_OEM_REQ			0x2E
#define NETFN_OEM_RESP			0x2F

/* Controllerspecific OEM/Group 30h-3Fh 
- Vendor specific (16 Network Functions [8 pairs]). The Manufacturer ID
associated with the controller implementing the command identifies the
vendor or group that specifies the command functionality. While the
vendor defines the functional semantics for the cmd and data fields,
the cmd field is required to hold the same value in requests and
responses for a given operation in order for the messages to be
supported under the IPMI message handling and transport
mechanisms. */

#define	CHECKSUM_OK		0x0
#define CHECKSUM_HEADER_ERROR	0x1
#define	CHECKSUM_DATA_ERROR	0x2

/*======================================================================*/
/*				Completion Codes			*/
/*======================================================================*/

#define CC_NORMAL		0x00	/* Command Completed Normally. */
#define CC_BUSY			0xC0	/* Node Busy. Command could not 
					   be processed because command 
					   processing resources are 
					   temporarily unavailable. */
#define CC_INVALID_CMD		0xC1	/* Invalid Command. Used to 
					   indicate an unrecognized 
					   or unsupported command. */
#define	CC_INVALID_CMD_FOR_LUN	0xC2	/* Command invalid for given LUN. */
#define	CC_TIMEOUT		0xC3	/* Timeout while processing command. 
					   Response unavailable. */
#define CC_OUT_OF_SPACE		0xC4	/* Out of space. Command could not 
					   be completed because of a lack 
					   of storage space required to 
					   execute the given command operation. */
#define	CC_RESERVATION		0xC5	/* Reservation Canceled or 
					   Invalid Reservation ID. */
#define CC_RQST_DATA_TRUNCATED	0xC6	/* Request data truncated.*/
#define	CC_RQST_DATA_LEN_INVALID 0xC7	/* Request data length invalid. */
#define CC_DATA_LEN_EXCEEDED	0xC8	/* Request data field length limit exceeded.*/
#define	CC_PARAM_OUT_OF_RANGE	0xC9	/* Parameter out of range. One or more 
					   parameters in the data field of the
					   Request are out of range. This is 
					   different from ‘Invalid data field’ 
					   (CCh) code in that it indicates 
					   that the erroneous field(s) has a 
					   contiguous range of possible values. */
#define	CC_CANT_RETURN_REQ_BYTES 0xCA	/* Cannot return number of requested data bytes. */
#define CC_REQ_DATA_NOT_AVAIL	0xCB	/* Requested Sensor, data, or record not present. */
#define CC_INVALID_DATA_IN_REQ	0xCC	/* Invalid data field in Request */
#define CC_CMD_ILLEGAL		0xCD	/* Command illegal for specified 
					   sensor or record type. */
#define CC_CMD_RESP_NOT_PROVIDED 0xCE	/* Command response could not be provided. */
#define CC_CANT_EXECUTE_DUP_REQ	0xCF	/* Cannot execute duplicated request. 
					   This completion code is for devices which
					   cannot return the response that was returned 
					   for the original instance of the
					   request. Such devices should provide 
					   separate commands that allow the
					   completion status of the original request 
					   to be determined. An Event Receiver
					   does not use this completion code, but
					   returns the 00h completion code in the
					   response to (valid) duplicated requests. */
#define	CC_SDR_IN_UPDATE_MODE	0xD0	/* Command response could not be provided. 
					   SDR Repository in update mode. */
#define CC_FW_IN_UPDATE_MODE	0xD1	/* Command response could not be provided. 
					   Device in firmware update mode. */
#define CC_INITIALIZATION	0xD2	/* Command response could not be provided. 
					   BMC initialization or initialization
					   agent in progress. */
#define CC_DEST_UNAVAILABLE	0xD3	/* Destination unavailable. Cannot deliver 
					   request to selected destination. E.g. this
					   code can be returned if a request message 
					   is targeted to SMS, but receive
					   message queue reception is disabled for 
					   the particular channel. */
#define	CC_SECURITY_RESTRICTION	0xD4	/* Cannot execute command due to insufficient 
					   privilege level or other security based
					   restriction (e.g. disabled for ‘firmware 
					   firewall’). */
#define	CC_NOT_SUPPORTED	0xD5	/* Cannot execute command. Command, or 
					   request parameter(s), not supported in
					   present state. */
#define	CC_PARAM_ILLEGAL	0xD6	/* Cannot execute command. Parameter is 
					   illegal because command sub-function
					   has been disabled or is unavailable 
					   (e.g. disabled for ‘firmware firewall’). */
#define CC_UNSPECIFIED_ERROR	0xFF	/* Unspecified error. */

/* DEVICE-SPECIFIC (OEM) CODES 01h-7Eh */
/*
 * 01h-7Eh Device specific (OEM) completion codes. This range is used for 
 * command specific codes that are also specific for a particular device 
 * and version. A-priori knowledge of the device command set is required 
 * for interpretation of these codes. 
 */ 
#define CC_DELAYED_COMPLETION	0x7e	/* we use this to flag that the command 
					   response will be sent later */
/* COMMAND-SPECIFIC CODES 80h-BEh */
/*
 * 80h-BEh Standard command-specific codes. This range is reserved for 
 * command specific completion codes for commands specified in this 
 * document. 
 */
#define	CC_UNINITIALIZED_WATCHDOG	0x80 /* Attempt to start un-initialized watchdog */

/* IPMI_CMD_SEND_MESSAGE command-specific codes */
#define CC_LOST_ARBITRATION		0x81 /* Lost Arbitration */
#define CC_BUS_ERROR			0x82 /* Bus Error */
#define CC_NAK_ON_WRITE			0x83 /* NAK on Write */

/* transport layer completion codes */
#define XPORT_REQ_NOERR 	0 
#define XPORT_REQ_ERR		1
#define XPORT_RESP_NOERR	2
#define XPORT_RESP_ERR		3

#define IPMI_IF_IPMB		0
#define IPMI_IF_TERMINAL	1
/*======================================================================*/
/*		General IPMI request/reply formats			*/
/*======================================================================*/
typedef struct ipmi_cmd_req {
	uchar command;
	uchar data;
} IPMI_CMD_REQ;


typedef struct ipmi_cmd_resp {
	uchar completion_code;
	uchar data;
} IPMI_CMD_RESP;

/* CHANNEL NUMBERS */
/* Each interface has a channel number that is used when configuring the channel
 * and for routing messages between channels. Only the channel number assignments
 * for the primary IPMB and the System Interface are fixed, the assignment of 
 * other channel numbers can vary on a per-platform basis. Software uses a Get 
 * Channel Info command to determine what types of channels are available and 
 * what channel number assignments are used on a given platform. 
 */

/* AMC REQ 3.151: Carrier IPMCs and MMCs shall implement IPMB-L as IPMI
 * messaging channel number 7.
 */

#define IPMI_CH_NUM_PRIMARY_IPMB	0x0
#define IPMI_CH_NUM_CONSOLE		0x1
#define IPMI_CH_NUM_LOCAL		0x2
#define IPMI_CH_NUM_IPMBL		0x7
#define IPMI_CH_NUM_PRESENT_INTERFACE	0xE
#define IPMI_CH_NUM_SYS_INTERFACE	0xF


/* CHANNEL PROTOCOL TYPE */
/* The protocol used for transferring IPMI messages on a given channel is 
 * identified using a channel protocol type number. 
 */
#define IPMI_CH_PROTOCOL_IPMB	0x1	/* used for IPMB, serial/modem Basic Mode, and LAN */
#define IPMI_CH_PROTOCOL_ICMB	0x2	/* ICMB v1.0 */
#define IPMI_CH_PROTOCOL_SMB	0x4	/* IPMI on SMSBus */
#define IPMI_CH_PROTOCOL_KCS	0x5	/* KCS System Interface Format */
#define IPMI_CH_PROTOCOL_SMIC	0x6	/* SMIC System Interface Format */
#define IPMI_CH_PROTOCOL_BT10	0x7	/* BT System Interface Format, IPMI v1.0 */
#define IPMI_CH_PROTOCOL_BT15	0x8	/* BT System Interface Format, IPMI v1.5 */
#define IPMI_CH_PROTOCOL_TMODE	0x9	/* Terminal Mode */
#define IPMI_CH_PROTOCOL_NONE	0xf	/* Invalid protocol type */

/* CHANNEL MEDIUM TYPE */
/* The Channel Medium Type number is a seven-bit value that identifies the 
 * general class of medium that is being used for the channel.
 */ 
#define IPMI_CH_MEDIUM_IPMB	0x1	/* IPMB (I2C) 				*/
#define IPMI_CH_MEDIUM_ICMB10	0x2	/* ICMB v1.0 				*/
#define IPMI_CH_MEDIUM_ICMB09	0x3	/* ICMB v0.9 				*/
#define IPMI_CH_MEDIUM_LAN	0x4	/* 802.3 LAN 				*/
#define IPMI_CH_MEDIUM_SERIAL	0x5	/* Asynch. Serial/Modem (RS-232) 	*/
#define IPMI_CH_MEDIUM_LAN_AUX	0x6	/* Other LAN				*/
#define IPMI_CH_MEDIUM_PCI_SMB	0x7	/* PCI SMBus				*/
#define IPMI_CH_MEDIUM_SMB_1x	0x8	/* SMBus v1.0/1.1			*/
#define IPMI_CH_MEDIUM_SMB_20	0x9	/* SMBus v2.0				*/
#define IPMI_CH_MEDIUM_USB_1x	0xA	/* reserved for USB 1.x			*/
#define IPMI_CH_MEDIUM_USB_20	0xB	/* reserved for USB 2.x			*/
#define IPMI_CH_MEDIUM_SYS	0xC	/* System Interface (KCS, SMIC, or BT)	*/
					/* 60h-7Fh OEM, all other reserved	*/
typedef struct channel {
	unsigned short protocol;
	unsigned short medium;
} CHANNEL;

typedef struct pkt_hdr {
	unsigned char	lun;
	unsigned	req_data_len;
	unsigned	resp_data_len;
//	unsigned	req_ch_medium_type;	/* request channel medium type: IPMI_CH_MEDIUM_ */
//	unsigned	resp_if;	/* IPMI interface to send response */
	unsigned	cmd_len;
	uchar		netfn;
	uchar		responder_lun;
	char		*ws;
} PKT_HDR;

typedef struct ipmi_pkt {
	PKT_HDR		hdr;
	IPMI_CMD_REQ	*req;
	IPMI_CMD_RESP	*resp;
} IPMI_PKT;

//#define WS_ARRAY_SIZE	8
#define WS_BUF_LEN 32

typedef struct list_hdr {
	struct list_hdr *next;
} LIST_HDR;

/* ipmi working set */
typedef struct ipmi_ws {
//	LIST_HDR hdr;
	unsigned ws_state;
	unsigned len_rcv;		/* requested length of incoming pkt */
	unsigned len_in;		/* lenght of incoming pkt */
	unsigned len_out;		/* length of outgoing pkt */
	unsigned len_sent;		/* length of pkt actually sent */
	unsigned timestamp;		/* last access time to this ws element */
	unsigned char flags;		/* protocol dependent i.e. WS_FL_xx */
	unsigned char addr_in;		/* protocol dependent */
	unsigned char addr_out;
	unsigned char incoming_channel;
	unsigned char outgoing_channel;
	unsigned char incoming_protocol;
	unsigned char outgoing_protocol;
	unsigned char incoming_medium;
	unsigned char outgoing_medium;
	unsigned char interface;
	unsigned char seq_out;		/* sequence number */
	unsigned char delivery_attempts;
	void *bridged_ws;		/* the ws we're bridging */
	void(*xport_completion_function)( void *, int );
	void(*ipmi_completion_function)( void *, int );
	IPMI_PKT pkt;
	unsigned char pkt_in[WS_BUF_LEN];
	unsigned char pkt_out[WS_BUF_LEN];
} IPMI_WS;

#define WS_FL_GENERAL_CALL	1


/*======================================================================*/
/* 				IPMI IPMB INTERFACE 			*/
/*======================================================================*/

/* General IPMB request format : used for IPMB, serial/modem Basic Mode, and LAN*/
/* TODO 
 * 1) check responder_slave_addr field
 * 2) remove LAN req/resp definitions, they are duplicate */

typedef struct ipmi_ipmb_hdr {
//	uchar	slave_addr;		/* Requester/Responder Slave Address. */
#ifdef BF_MS_FIRST
	uchar	netfn:6,		/* NetFn. For a response command, 
					   the NetFn is always odd and one greater 
					   than the NetFn provided in the original 
					   request. The IPMI and IPMB specifications 
					   define the legal NetFns. */
		lun:2;			/* The Requester/Responder LUN */
#else
	uchar	lun:2,
		netfn:6;
#endif
} IPMI_IPMB_HDR;

#define IPMB_REQ_MAX_DATA_LEN 25

typedef struct ipmi_ipmb_request {
//	uchar	responder_slave_addr;
				 	/* This is the IPMB address of the device that
					   is expected to respond to the message. */
#ifdef BF_MS_FIRST  /* bit fields assigned left to right (MS bits first) */
	uchar	netfn:6, 		/* This contains the network function of 
                                           the message. For a request command, the 
                                           NetFn is always even. The IPMI and IPMB 
                                           specifications define the legal NetFns. 
                                           Of particular interest is NetFn 2Ch (Request) 
                                           and 2Dh (Response) for the commands stated 
                                           in the PICMG v3 specification. */
		responder_lun:2;	/* The Responder LUN (Logical Unit Number) 
					   defines which unit is meant to respond to 
					   the message (see the IPMI specification for 
					   further definition). */
#else /* bit fields assigned right to left (LS bits first) */
	uchar	responder_lun:2,
		netfn:6;
#endif
	uchar	header_checksum;	/* 2’s complement checksum of preceding 
					   bytes in the connection header. */
	uchar	requester_slave_addr;	/* Requester Slave Address. This is the IPMB 
					   address of the requesting device. */
#ifdef BF_MS_FIRST
	uchar	req_seq:6;		/* Request Sequence/Requester LUN. The Request 
					   Sequence identifier is used by the device(s) 
					   to determine if duplicate requests/responses
					   are received.*/
       		requester_lun:2;	/* The Requester LUN provides 
					   the LUN that should receive the response. */
#else
	uchar	requester_lun:2,
		req_seq:6;
#endif
	uchar	command;		/* Command. This defines the command within 
					   the NetFn to execute. */
	uchar	data[IPMB_REQ_MAX_DATA_LEN];		/* 7:N Data Bytes. The Command may be followed 
					   by zero or more data bytes that are command 
					   specific. The maximum N value by IPMI 
					   definition is 31; thus, 25 bytes of 
					   request data are allowed in each request. */
	uchar	data_checksum;		/* Data Checksum. 2’s complement checksum 
					   of preceeding bytes back to, but not 
					   including, the Header Checksum. Note that this 
					   position is bogus and is used as a placeholder
					   to indicate that a checksum follows the data field.
					   The location of the data_checksum depends on
					   the size of the data preceeding it. */
} IPMI_IPMB_REQUEST;

#define IPMB_RESP_MAX_DATA_LEN	24

/* General IPMI response format */
typedef struct ipmi_ipmb_response {
//	uchar	requester_slave_addr;	
					/* Requester Slave Address. This is the 
					   IPMB address of the requesting device 
					   and describes which device receives the 
					   response. */
#ifdef BF_MS_FIRST
	uchar	netfn:6,		/* NetFn/Requester LUN (6 bits/2 bits). 
					   This contains the network function of
					   the message. For a response command, 
					   the NetFn is always odd and one greater 
					   than the NetFn provided in the original 
					   request. The IPMI and IPMB specifications 
					   define the legal NetFns. */
		requester_lun:2;	/* The Requester LUN defines which unit 
					   is meant to receive the response
					   (see the IPMI specification for 
					   further definition). */
#else
	uchar	requester_lun:2,
		netfn:6;
#endif
	uchar	header_checksum;	/* Header Checksum. 2’s complement checksum 
					   of preceding bytes in the connection 
					   header. */
	uchar	responder_slave_addr;	/* Responder Slave Address. This is 
					   the IPMB address of the device that is
					   responding to the message. */
#ifdef BF_MS_FIRST
	uchar	req_seq:6,		/* Request Sequence/Responder LUN. 
					   The Request Sequence identifier is
					   used by the device(s) to determine 
					   if duplicate requests/responses are
					   received. */
		responder_lun:2;	/* The Responder LUN provides the LUN 
					   that sent the response. */
#else
	uchar	responder_lun:2,
      		req_seq:6;
#endif
	uchar	command;		/* Command. This defines the command 
					   within the NetFn that was requested. */
	uchar	completion_code;	/* Completion Code. This is a response 
					   code that defines whether the
					   command executed successfully or not. */
	uchar	data[IPMB_RESP_MAX_DATA_LEN];		/* 8:N Data Bytes. The Command may 
					   require zero or more additional response
					   bytes. The maximum N value by IPMI 
					   definition is 31; thus, 24 bytes of
					   response data are allowed in each 
					   response. */
	uchar	data_checksum;		/* Data Checksum. 2’s complement 
					   checksum of preceeding bytes back to,
					   but not including, the Header Checksum.
					   Note that this position is bogus and 
					   is used as a placeholder to indicate
					   that a checksum follows the data field.
					   The location of the data_checksum depends
					   on the size of the data preceeding it. */
} IPMI_IPMB_RESPONSE;

/*======================================================================*/
/* 			KCS/SMIC Interface 				*/
/*======================================================================*/
typedef struct ipmi_kcs_request {
#ifdef BF_MS_FIRST  
	uchar	netfn:6, 		/* Network Function code. This provides
					   the first level of functional routing
					   for messages received by the BMC via
					   the KCS Interface. The NetFn field
					   occupies the most significant six bits
					   of the first message byte. Even NetFn
					   values are used for requests to the BMC,
					   and odd NetFn values are returned in
					   responses from the BMC. */

		responder_lun:2;	/* Logical Unit Number. This is a sub-
					   address that allows messages to be 
					   routed to different ‘logical units’
					   that reside behind the same physical
					   interface. The LUN field occupies the
					   least significant two bits of the
					   first message byte. */
#else
	uchar	responder_lun:2,
		netfn:6;
#endif
	uchar	command;
	uchar	data[25];		/* TODO check size */
} IPMI_KCS_REQUEST;

typedef struct ipmi_kcs_response {
#ifdef BF_MS_FIRST
	uchar	netfn:6,		/* Network Function. This is a return 
					   of the NetFn code that was passed 
					   in the Request Message. Except that
					   an odd NetFn value is returned. */

		requester_lun:2;	/* Logical Unit Number. This is a 
					   return of the LUN that was passed 
					   in the Request Message. */
#else
	uchar	requester_lun:2,
		netfn:6;
#endif
	uchar	command;		/* Command. This is a return of the 
					   Cmd code that was passed in the
					   Request Message. */
	uchar	completion_code;
	uchar	data[25];		/* TODO check size */

} IPMI_KCS_RESPONSE;

/*======================================================================*/
/* 				BT Interface 				*/
/*======================================================================*/
typedef struct ipmi_bt_request {

	uchar	length;			/* This is not actually part of the 
					   message, but part of the framing
					   for the BT Interface. This value
					   is the 1-based count of message
					   bytes following the length byte.
					   The minimum length byte alue for
					   a command to the BMC would be 3
					   to cover the NetFn/LUN, Seq, and
					   Cmd bytes. */
#ifdef BF_MS_FIRST  
	uchar	netfn:6, 		/* Network Function code. This provides
					   the first level of functional routing
					   for messages received by the BMC via
					   the BT Interface. The NetFn field
					   occupies the most significant six bits
					   of the first message byte. Even NetFn
					   values are used for requests to the BMC,
					   and odd NetFn values are returned in
					   responses from the BMC. */
		responder_lun:2;	/* Logical Unit Number. This is a sub-
					   address that allows messages to be 
					   routed to different ‘logical units’
					   that reside behind the same physical
					   interface. The LUN field occupies the
					   least significant two bits of the
					   first message byte. */
#else
	uchar	responder_lun:2,
		netfn:6;
#endif
	uchar	command;
	uchar	data[25];		/* TODO check size */
} IPMI_BT_REQUEST;

typedef struct ipmi_bt_response {
	uchar	length;			/* This is not actually part of the 
					   message, but part of the framing
					   for the BT Interface. This value
					   is the 1-based count of message
					   bytes following the length byte.
					   The minimum length byte value for
					   a response from the BMC would be 
					   4 to cover the NetFn/LUN, Seq, 
					   Cmd, and Completion Code bytes. */
#ifdef BF_MS_FIRST
	uchar	netfn:6,		/* Network Function. This is a return 
					   of the NetFn code that was passed 
					   in the Request Message. Except that
					   an odd NetFn value is returned. */

		requester_lun:2;	/* Logical Unit Number. This is a 
					   return of the LUN that was passed 
					   in the Request Message. */
#else
	uchar	requester_lun:2,
		netfn:6;
#endif
	uchar	command;		/* Command. This is a return of the 
					   Cmd code that was passed in the
					   Request Message. */
	uchar	completion_code;
	uchar	data[25];		/* TODO check size */

} IPMI_BT_RESPONSE;

/*======================================================================*/
/* 				IPMI LAN INTERFACE 			*/
/*======================================================================*/

/*
	IPMI LAN Packet Layering
	========================

	Ethernet Framing
	----------------
	MAC Address

		IP/UDP
		------
		IP Address, RMCP Port #

			RMCP message
			------------
			Class = 07 forIPMI
			RMCP Sequence# = FFh for IPMI

				IPMI v1.5 or IPMI v2.0+ Session Wrapper
				---------------------------------------
				Payload type
				Payload (per payload type)
				
					IPMI LAN Message
					----------------
					NetFn
					LUN
					Seq#
					CMD
					Data
					
					SOL
					---
*/

/* IPMI LAN MESSAGE FORMATS */

typedef struct ipmi_lan_request {
	uchar	responder_slave_addr;	/* Responder's Slave Address. LS bit is 0
					   for Slave Addresses and 1 for Software
					   IDs. Upper 7-bits hold Slave Address 
					   or Software ID, respectively. This byte
					   is always 20h when the BMC is the 
					   responder. */
#ifdef BF_MS_FIRST  
	uchar	netfn:6,
		responder_lun:2;
#else
	uchar	responder_lun:2,
		netfn:6;
#endif
	uchar	checksum1;		/* 2's complement checksum of preceding
					   bytes in the connection header or 
					   between the previous checksum. 8-bit
					   checksum algorithm: Initialize 
					   checksum to 0. For each byte, 
					   checksum = (checksum + byte) modulo 
					   256. Then checksum = - checksum. 
					   When the checksum and the bytes are
					   added together, modulo 256, the 
					   result should be 0. */
	
	uchar	requester_address;	/* Requester's Address. LS bit is 0 for 
					   Slave Addresses and 1 for Software IDs.
					   Upper 7-bits hold Slave Address or 
					   Software ID, respectively. This byte 
					   is always 20h when the BMC is the 
					   requester. */
#ifdef BF_MS_FIRST  
	uchar	requester_seq_num:6,	/* Sequence number, generated by the requester. */
		requester_lun:2;	/* Requester's LUN */
#else
	uchar	requester_lun:2,
		requester_seq_num:6;
#endif
	uchar	command;
	uchar	data[25];		/* TODO check size */
	uchar	checksum2;
} IPMI_LAN_REQUEST;

typedef struct ipmi_lan_response {
	uchar	requester_addr;
#ifdef BF_MS_FIRST  
	uchar	netfn:6,
		requester_lun:2;
#else
	uchar	requester_lun:2,
		netfn:6;
#endif
	uchar	checksum1;
	uchar	responder_address;
#ifdef BF_MS_FIRST  
	uchar	requester_seq_num:6,	/* Sequence number, generated by the requester. */
		responder_lun:2;	/* Responder's LUN */
#else
	uchar	responder_lun:2,
		requester_seq_num:6;
#endif
	uchar	command;
	uchar	completion_code;
	uchar	data[25];		/* TODO check size */
	uchar	checksum2;
} IPMI_LAN_RESPONSE;

/* IPMI OVER LAN PAYLOAD TYPES */
#define IOLAN_PAYLOAD_IPMI_MESSAGE	0x00
#define IOLAN_PAYLOAD_SOL		0x01
#define IOLAN_OEM_EXPLICIT		0x02
#define IOLAN_RMCPP_OPEN_SESSION_REQ	0x10
#define IOLAN_RMCPP_OPEN_SESSION_RESP	0x11
#define IOLAN_RAKP_MSG1			0x12
#define IOLAN_RAKP_MSG2			0x13
#define IOLAN_RAKP_MSG3			0x14
#define IOLAN_RAKP_MSG4			0x15
#define IOLAN_OEM0			0x20
#define IOLAN_OEM1			0x21
#define IOLAN_OEM2			0x22
#define IOLAN_OEM3			0x23
#define IOLAN_OEM4			0x24
#define IOLAN_OEM5			0x25
#define IOLAN_OEM6			0x26
#define IOLAN_OEM7			0x27


/*======================================================================*/
/* 			IPMI TERMINAL MODE INTERFACE 			*/
/*======================================================================*/

#define TERM_MODE_REQ_MAX_DATA_LEN	25
#define TERM_MODE_RESP_MAX_DATA_LEN	25

/* generic terminal mode header */
typedef struct ipmi_terminal_mode_hdr {
#ifdef BF_MS_FIRST
	uchar	netfn:6,
		lun:2;
#else
	uchar	lun:2,
		netfn:6;
#endif
#ifdef BF_MS_FIRST
	uchar	seq:6, 
		bridge:2;
#else
	uchar	bridge:2,
		seq:6;
#endif
	uchar	command;
} IPMI_TERMINAL_MODE_HDR;

typedef struct ipmi_terminal_mode_request {
#ifdef BF_MS_FIRST
	uchar	netfn:6, 		/* This contains the network function of 
                                           the message. For a request command, the 
                                           NetFn is always even. The IPMI and IPMB 
                                           specifications define the legal NetFns. 
                                           Of particular interest is NetFn 2Ch (Request) 
                                           and 2Dh (Response) for the commands stated 
                                           in the PICMG v3 specification. */
		responder_lun:2;	/* The Responder LUN (Logical Unit Number) 
					   defines which unit is meant to respond to 
					   the message (see the IPMI specification for 
					   further definition). */
#else
	uchar	responder_lun:2,
		netfn:6;
#endif
#ifdef BF_MS_FIRST
	uchar	req_seq:6, 		/* The Request Sequence identifier is
					   used by the device(s) to determine 
					   if duplicate requests/responses are
					   received. */
		bridge:2;		/* Used to identify whether the message
					   should be routed to the BMC's bridged 
					   message tracking functionalty or not.
					   See Table 14-12, Terminal Mode Message
					   Bridge Field */
#else
	uchar	bridge:2,
		req_seq:6;
#endif
	uchar	command;		/* Command. This defines the command within 
					   the NetFn to execute. */
	uchar	data[TERM_MODE_REQ_MAX_DATA_LEN];
					/* 7:N Data Bytes. The Command may be followed 
					   by zero or more data bytes that are command 
					   specific. The maximum N value by IPMI 
					   definition is 31; thus, 25 bytes of 
					   request data are allowed in each request. */
} IPMI_TERMINAL_MODE_REQUEST;

typedef struct ipmi_terminal_mode_response {
#ifdef BF_MS_FIRST
	uchar	netfn:6,
		responder_lun:2;
#else
	uchar	responder_lun:2,
		netfn:6;
#endif
#ifdef BF_MS_FIRST
	uchar	req_seq:6, 
		bridge:2;
#else
	uchar	bridge:2,
		req_seq:6;
#endif
	uchar	command;
	uchar	completion_code;
	uchar	data[TERM_MODE_RESP_MAX_DATA_LEN];
} IPMI_TERMINAL_MODE_RESPONSE;

// Platform dependent - fill in as required
#define MAX_FRU_DEV_ID	0
	/* TODO: For AMC carriers, this field indicates the maximum FRU ID
	   supported by Carrier IPMC. It does not imply that all FRUs with
	   FRU IDs between 0 and Max FRU ID are installed in the Carrier,
	   since the AdvancedMC Slots implemented by the Carrier are not
	   necessarily occupied.
	   For AMC MMCs MAX_FRU_DEV_ID is required to be zero in this context
	   (because MMCs do not have subsidiary FRUs by definition), but are
	   retained in the response data for compatibility with other use
	   contexts for this command. */

#define DEACTIVATE_FRU(a) 
#define ACTIVATE_FRU(a) 
#define NUM_LUN		1

/*======================================================================*/
/*
 *   IPM Device “Global” Commands
 *
Command				NetFn 	CMD 	BMC 	SHM 	IPMC
Get Device ID 			App 	01h	M	M	M
Cold Reset 			App 	02h 	O 	O 	O
Warm Reset 			App 	03h 	O 	O 	O
Get Self Test Results 		App 	04h 	M 	M 	M
Manufacturing Test On 		App 	05h 	O 	O 	O
Set ACPI Power State 		App 	06h 	O 	O 	O
Get ACPI Power State  		App 	07h 	O 	O 	O
Get Device GUID 		App 	08h 	O 	O 	O
Broadcast “Get Device ID” 	App 	01h 	O/M 	M 	M
*/
/*======================================================================*/

/*----------------------------------------------------------------------*/
/*			Get Device ID Command				*/
/*----------------------------------------------------------------------*/

#define	IPMI_CMD_GET_DEVICE_ID	0x01
#define APP_CMD_GET_DEVICE_ID	0x0601
/*
 * Flags for "Additional Device Support" field in  Get Device ID Command - response data
 * (formerly called IPM Device Support). Lists the IPMI ‘logical device’ commands and
 * functions that the controller supports that are in addition to the mandatory
 * IPM and Application commands.
 */
#define DEV_SUP_CHASSIS		0x80	/* [7] Chassis Device (device functions 
					   as chassis device per ICMB spec.) */
#define DEV_SUP_BRIDGE		0x40	/* [6] Bridge (device responds to Bridge 
					   NetFn commands) */
#define DEV_SUP_IPMB_EVENT_GEN	0x20	/* [5] IPMB Event Generator (device generates 
					   event messages [platform event request 
					   messages] onto the IPMB) */
#define DEV_SUP_IPMB_EVENT_RCV	0x10	/* [4] IPMB Event Receiver (device accepts 
					   event messages [platform event request 
					   messages] from the IPMB) */
#define DEV_SUP_FRU_INVENTORY	0x08	/* [3] FRU Inventory Device */
#define DEV_SUP_SEL		0x04	/* [2] SEL Device */
#define DEV_SUP_SDR_REPOSITORY	0x02	/* [1] SDR Repository Device */
#define DEV_SUP_SENSOR		0x01	/* [0] Sensor Device */

#define DEV_STATUS_READY		0
#define DEV_STATUS_NOT_READY		1


typedef struct generic_cmd_req {
	uchar	command;
} GENERIC_CMD_REQ;


/* Get Device ID Command - response data */
typedef struct get_device_id_cmd_resp {
	uchar	completion_code;	/* Completion Code */
	uchar	device_id;		/* Device ID. 00h = unspecified. */
#ifdef BF_MS_FIRST
	uchar	device_sdr_provided:1,	/* [7] 1 = device provides Device SDRs
					   0 = device does not provide Device SDRs */
		:3,			/* [6:4] reserved. Return as 0. */
		device_revision:4;	/* [3:0] Device Revision, binary encoded. */
					/* Firmware Revision 1 */
#else
	uchar	device_revision:4,
		:3,
		device_sdr_provided:1;
#endif
#ifdef BF_MS_FIRST
	uchar	device_available:1,	/* [7] Device available: 
					   0=normal operation, 
					   1= device firmware, SDR Repository 
					   update or self-initialization in progress. 
					   [Firmware / SDR Repository updates can 
					   be differentiated by issuing a Get SDR
					   command and checking the completion code.] */
		major_fw_rev:7;		/* [6:0] Major Firmware Revision, binary encoded. */
#else
	uchar	major_fw_rev:7,
		device_available:1;
#endif
	uchar	minor_fw_rev;		/* Firmware Revision 2: 
					   Minor Firmware Revision. BCD encoded. */
	uchar	ipmi_version;		/* IPMI Version. Holds IPMI Command 
					   Specification Version. BCD encoded.
					   00h = reserved. Bits 7:4 hold the Least 
					   Significant digit of the revision, while
					   bits 3:0 hold the Most Significant bits. 
					   E.g. a value of 51h indicates revision 1.5. */
	uchar	add_dev_support;	/* Additional Device Support (formerly 
					   called IPM Device Support). Lists the
					   IPMI ‘logical device’ commands and 
					   functions that the controller supports 
					   that are in addition to the mandatory
					   IPM and Application commands.
					   [7] Chassis Device (device functions 
					   as chassis device per ICMB spec.)
					   [6] Bridge (device responds to Bridge 
					   NetFn commands)
					   [5] IPMB Event Generator (device generates 
					   event messages [platform event request 
					   messages] onto the IPMB)
					   [4] IPMB Event Receiver (device accepts 
					   event messages [platform event request 
					   messages] from the IPMB)
					   [3] FRU Inventory Device
					   [2] SEL Device
					   [1] SDR Repository Device
					   [0] Sensor Device */
	uchar	manuf_id[3];		/* 8:10 Manufacturer ID, LS Byte first. 
					   The manufacturer ID is a 20-bit value 
					   that is derived from the IANA ‘Private 
					   Enterprise’ ID. Most significant four 
					   bits = reserved (0000b). 
					   000000h = unspecified. 
					   0FFFFFh = reserved. 
					   This value is binary encoded.
					   E.g. the ID for the IPMI forum is 7154 
					   decimal, which is 1BF2h, which would
					   be stored in this record as F2h, 1Bh, 
					   00h for bytes 8 through 10, respectively. */ 
	uchar	product_id[2];		/* 11:12 Product ID, LS Byte first. 
					   This field can be used to provide a 
					   number that identifies a particular 
					   system, module, add-in card, or board 
					   set. The number is specified according 
					   to the manufacturer given by Manufacturer 
					   ID. 0000h = unspecified. FFFFh = reserved. */
	uchar	aux_fw_rev[4];		/* (13:16) Auxiliary Firmware Revision
					   Information. This field is optional. 
					   If present, it holds additional information 
					   about the firmware revision, such as boot 
					   block or internal data structure version 
					   numbers. The meanings of the numbers are
					   specific to the vendor identified 
					   by Manufacturer ID. When the
					   vendor-specific definition is not 
					   known, generic utilities should 
					   display each byte as 2-digit 
					   hexadecimal numbers, with byte 13 
					   displayed first as the most significant
					   byte. */

} GET_DEVICE_ID_CMD_RESP;
	
/*----------------------------------------------------------------------*/
/*			Cold Reset Command				*/
/*----------------------------------------------------------------------*/
#define	IPMI_CMD_COLD_RESET	0x02

/*----------------------------------------------------------------------*/
/*			Warm Reset Command				*/
/*----------------------------------------------------------------------*/
#define	IPMI_CMD_WARM_RESET	0x03

/*----------------------------------------------------------------------*/
/*			Get Self Test Results command			*/
/*----------------------------------------------------------------------*/

#define SELFTEST_CANT_ACCESS_SEL_DEVICE		0x80	/* [7] 1b = Cannot access SEL device */
#define SELFTEST_CANT_ACCESS_SDR_REPOSITORY	0x40	/* [6] 1b = Cannot access SDR Repository */
#define SELFTEST_CANT_ACCESS_BMC_FRU_DEVICE	0x20	/* [5] 1b = Cannot access BMC FRU device */
#define SELFTEST_IPMB_SIGNAL_LINES_DEAD		0x10	/* [4] 1b = IPMB signal lines do not respond */
#define SELFTEST_SDR_REPOSITORY_EMPTY		0x08	/* [3] 1b = SDR Repository empty */
#define SELFTEST_INTERNAL_BMC_FRU_CORRUPTED	0x04	/* [2] 1b = Internal Use Area of BMC FRU corrupted */
#define SELFTEST_BOOT_BLOCK_FW_CORRUPTED	0x20	/* [1] 1b = controller update ‘boot block’ 
							   firmware corrupted */
#define SELFTEST_CNTR_OP_FW_CORRUPTED		0x01	/* [0] 1b = controller operational firmware 
							   corrupted */

#define SELFTEST_RESULT_NO_ERROR		0x55	/* No error. All Self Tests Passed */
#define SELFTEST_RESULT_NOT_IMPLEMENTED		0x56	/* Self Test function not implemented
							   in this controller. */
#define SELFTEST_RESULT_CORRUPT_OR_INACCESIBLE	0x57	/* Corrupted or inaccessible 
							   data or devices */
#define SELFTEST_RESULT_FATAL_HARDWARE_ERROR	0x58	/* Fatal Hardware Error */

#define	IPMI_CMD_GET_SELF_TEST_RESULTS	0x04		/* Get Self Test Results - command opcode */

/* Get Self Test Results - response data */
typedef struct get_self_test_results_cmd_resp {
	uchar	completion_code;	/* Completion Code */
	uchar	result1;		/* 55h = No error. All Self Tests Passed.
					   56h = Self Test function not 
					   implemented in this controller.
					   57h = Corrupted or inaccessible 
					   data or devices
					   58h = Fatal hardware error 
					   (system should consider BMC
					   inoperative). This will indicate 
					   that the controller hardware 
					   (including associated devices 
					   such as sensor hardware or RAM) 
					   may need to be repaired or replaced.
					   FFh = reserved.
					   all other: Device-specific 
					   ‘internal’ failure. Refer to the 
					   particular device’s specification 
					   for definition. */
	uchar	result2;		/* For byte 2 = 55h, 56h, FFh: 00h
					   For byte 2 = 58h, all other: Device-specific
					   For byte 2 = 57h: self-test 
					   error bitfield. Note: returning 
					   57h does not imply that all 
					   tests were run, just that 
					   a given test has failed. I.e. 1b
					   means ‘failed’, 0b means ‘unknown’. */
	uchar	result3;		/* [7] 1b = Cannot access SEL device
					   [6] 1b = Cannot access SDR Repository
					   [5] 1b = Cannot access BMC FRU device
					   [4] 1b = IPMB signal lines do not respond
					   [3] 1b = SDR Repository empty
					   [2] 1b = Internal Use Area of BMC FRU corrupted 
					   [1] 1b = controller update ‘boot block’ firmware corrupted
					   [0] 1b = controller operational firmware corrupted */
} GET_SELF_TEST_RESULTS_CMD_RESP;

/*======================================================================*/
/*
 *  BMC Watchdog Timer Commands
	
 	Command				NetFn 	CMD 	BMC 	SHM 	IPMC
	--------------------------------------------------------------------
	Reset Watchdog Timer	 	App 	22h	M	M	M
	Set Watchdog Timer 		App	24h	M	M	M
	Get Watchdog Timer 		App	25h	M	M	M
 */
/*======================================================================*/

/*----------------------------------------------------------------------*/
/*			Reset Watchdog Timer command			*/
/*----------------------------------------------------------------------*/

#define	IPMI_CMD_RESET_WATCHDOG_TIMER	0x22	/* Reset Watchdog Timer */
#define	IPMI_CMD_SET_WATCHDOG_TIMER	0x24	/* Set Watchdog Timer */
#define	IPMI_CMD_GET_WATCHDOG_TIMER	0x25	/* Get Watchdog Timer */

/* Response data */
typedef struct reset_watchdog_timer_cmd_resp {
	uchar	completion_code;	/* Completion Code. */
	/* Generic plus the following command-specific completion codes: 80h 
	 * Attempt to start un-initialized watchdog (CC_UNINITIALIZED_WATCHDOG).
	 * It is recommended that a BMC implementation return this error 
	 * completion code to indicate to software that a Set Watchdog Timer
	 * command has not been issued to initialize the timer since the last
	 * system power on, reset, or BMC reset. Note that since many systems
	 * may initialize the watchdog timer during BIOS operation, this 
	 * condition may only be seen by software if a BMC gets re-initialized 
	 * during system operation (as might be the case if a firmware update 
	 * occurred, for example).
	 */
	
} RESET_WATCHDOG_TIMER_CMD_RESP;

/*----------------------------------------------------------------------*/
/*			Set Watchdog Timer command			*/
/*----------------------------------------------------------------------*/
typedef struct set_watchdog_timer_cmd_req {
	uchar	command;
	/* timer use */
#ifdef BF_MS_FIRST
	uchar	dont_log:1,		/* [7] - 1b = don’t log 
					   By default, the BMC will automatically log
					   the corresponding sensor-specific watchdog
					   sensor event when a timer expiration occurs.
					   A “don’t log” bit is provided to temporarily
					   disable the automatic logging. The “don’t
					   log” bit is automatically cleared (logging
					   re-enabled) whenever a timer expiration occurs.
					   This is also cleared after every system
					   hard reset. */
		dont_stop_timer:1,	/* [6] - 1b = don’t stop timer 
					   on Set Watchdog Timer command 
					   (new for IPMI v1.5) new parameters 
					   take effect immediately. If timer 
					   is already running, countdown value
					   will get set to given value and
					   countdown will continue from that 
					   point. If timer is already stopped,
					   it will remain stopped. If the 
					   pre-timeout interrupt bit is set,
					   it will get cleared.
					       - 0b = timer stops automatically 
					   when Set Watchdog Timer command is
					   received. */
		/* Potential race conditions exist with implementations of this option. 
		 * If the Set Watchdog Timer command is sent just before a pre-timeout 
		 * interrupt or timeout is set to occur, the timeout could occur before 
		 * the command is executed. To avoid this condition, it is recommended that
		 * software set this value no closer than 3 counts before the pre-timeout 
		 * or timeout value is reached.*/
		:3,			/* [5:3] - reserved */
		timer_use:3;		/* [2:0] - timer use (logged on expiration 
					   when “don’t log” bit = 0b)
					   Indicates the current use assigned 
					   to the watchdog timer.
					   000b = reserved
					   001b = BIOS FRB2
					   010b = BIOS/POST
					   011b = OS Load
					   100b = SMS/OS
					   101b = OEM
					   110b -111b = reserved */
#else
	uchar	timer_use:3,
		:3,
		dont_stop_timer:1,
		dont_log:1;
#endif
	/* Timer Actions */
#define WD_PRE_TIMEOUT_INTR_NONE	0x0	/* None */
#define WD_PRE_TIMEOUT_INTR_SMI		0x1	/* SMI */
#define WD_PRE_TIMEOUT_INTR_NMI		0x2	/* NMI / Diagnostic Interrupt */
#define WD_PRE_TIMEOUT_INTR_MSG		0x3	/* Messaging Interrupt */

#define WD_TIMEOUT_ACTION_NONE		0x0	/* No action */
#define WD_TIMEOUT_ACTION_HARD_RESET	0x1	/* Hard Reset */
#define WD_TIMEOUT_ACTION_POWER_DOWN	0x2	/* Power Down */
#define WD_TIMEOUT_ACTION_POWER_CYCLE	0x3	/* Power Cycle */
		
#ifdef BF_MS_FIRST
	uchar	:1,			/* [7] - reserved */
		pre_timeout_intr:3,	/* [6:4] - pre-timeout interrupt (logged 
					   on expiration when “don’t log” bit = 0b)
					   000b = none
					   001b = SMI
					   010b = NMI / Diagnostic Interrupt
					   011b = Messaging Interrupt (this is 
					   the same interrupt as allocated to 
					   the messaging interface)
					   100b,111b = reserved */
		:1,			/* [3] - reserved */
		timeout_action:3;	/* [2:0] - timeout action
					   000b = no action
					   001b = Hard Reset
					   010b = Power Down
					   011b = Power Cycle
					   100b,111b = reserved */
#else
	uchar	timeout_action:3,
		:1,
		pre_timeout_intr:3,
		:1;
#endif
	uchar	pre_timeout_interval;	/* Pre-timeout interval in seconds. ‘1’ based. */
	uchar	timer_use_exp_fl_clr;	/* Timer Use Expiration flags clear.
					   The timeout use expiration flags retain
					   their state across system resets and power
					   cycles, as long as the BMC remains powered.
					   (0b = leave alone, 1b = clear timer use expiration bit)
					   [7] - reserved
					   [6] - reserved
					   [5] - OEM
					   [4] - SMS/OS
					   [3] - OS Load
					   [2] - BIOS/POST
					   [1] - BIOS FRB2
					   [0] - reserved */
	uchar	init_countdown_lsb;	/* Initial countdown value, lsbyte (100 ms/count) */
	uchar	init_countdown_msb;	/* Initial countdown value, msbyte */
} SET_WATCHDOG_TIMER_CMD_REQ;

typedef struct set_watchdog_timer_cmd_resp {
	uchar	completion_code;	/* Completion Code */
} SET_WATCHDOG_TIMER_CMD_RESP;

/*----------------------------------------------------------------------*/
/*			Get Watchdog Timer command			*/
/*----------------------------------------------------------------------*/

typedef struct get_watchdog_timer_cmd_resp {
	uchar	completion_code;	/* Completion Code */
	/* Timer Use */
#ifdef BF_MS_FIRST
	uchar	dont_log:1,		/* [7] - 1b = don’t log */
		dont_stop_timer:1,	/* [6] - 1b = timer is started (running)
					         0b = timer is stopped */
		:3,			/* [5:3] - reserved */
		timer_use:3;		/* [2:0] - timer use (logged on expiration 
					   when “don’t log” bit = 0b)
					   000b = reserved
					   001b = BIOS FRB2
					   010b = BIOS/POST
					   011b = OS Load
					   100b = SMS/OS
					   101b = OEM
					   110b -111b = reserved */

#else
	uchar	timer_use:3,
		:3,
		dont_stop_timer:1,
		dont_log:1;
#endif
	/* Timer Actions */
#ifdef BF_MS_FIRST
	uchar	:1,			/* [7] - reserved */
		pre_timeout_intr:3,	/* [6:4] - pre-timeout interrupt (logged 
					   on expiration when “don’t log” bit = 0b)
					   000b = none
					   001b = SMI
					   010b = NMI / Diagnostic Interrupt
					   011b = Messaging Interrupt (this would be 
					   the same interrupt as allocated to 
					   the messaging interface)
					   100b,111b = reserved */
		:1,			/* [3] - reserved */
		timeout_action:3;	/* [2:0] - timeout action
					   000b = no action
					   001b = Hard Reset
					   010b = Power Down
					   011b = Power Cycle
					   100b,111b = reserved */
#else
	uchar	timeout_action:3,
		:1,
		pre_timeout_intr:3,
		:1;
#endif
	uchar	pre_timeout_interval;	/* Pre-timeout interval in seconds. ‘1’ based. */
	uchar	timer_use_exp_fl;	/* Timer Use Expiration flags.
					   The timeout use expiration flags retain their
					   state across system resets and power cycles,
					   as long as the BMC remains powered.
					   (1b = timer expired while associated 
					   ‘use’ was selected.)
					   [7] - reserved
					   [6] - reserved
					   [5] - OEM
					   [4] - SMS/OS
					   [3] - OS Load
					   [2] - BIOS/POST
					   [1] - BIOS FRB2
					   [0] - reserved */
	uchar	init_countdown_lsb;	/* Initial countdown value, lsbyte (100 ms/count) */
	uchar	init_countdown_msb;	/* Initial countdown value, msbyte */
	uchar	present_countdown_lsb;	/* Present countdown value, lsbyte (100 ms/count) */
	uchar	present_countdown_msb;	/* Present countdown value, msbyte */

		/* The initial countdown value and present countdown values 
		 * should match immediately after the countdown is initialized
		 * via a Set Watchdog Timer command and after a Reset Watchdog
		 * Timer has been executed.
		 * Note that internal delays in the BMC may require software to 
		 * delay up to 100ms before seeing the countdown value change 
		 * and be reflected in the Get Watchdog Timer command.
		 */
} GET_WATCHDOG_TIMER_CMD_RESP;

/*======================================================================*/
/* 
 * BMC Device and Messaging Commands
 *
 * For ATCA, if any of the IPMI-defined System Interfaces are implemented,
 * the O/M commands are mandatory.
 * 
	Command				NetFn 	CMD 	BMC 	SHM 	IPMC
	--------------------------------------------------------------------
	Set BMC Global Enables 		App 	2Eh 	M 	O/M 	O/M
	Get BMC Global Enables 		App 	2Fh 	M 	M 	O/M
	Clear Message Flags		App 	30h 	M 	O/M 	O/M
	Get Message Flags		App 	31h 	M 	O/M 	O/M
	Enable Message Channel Receive 	App 	32h 	O 	O 	O
	Get Message 			App 	33h 	M 	O/M 	O/M
	Send Message 			App 	34h 	M 	M 	O/M
	Read Event Message Buffer 	App 	35h 	O 	O 	O
	Get BT Interface Capabilities	App 	36h 	M 	O/M 	O/M
	Master Write-Read 		App 	52h 	M 	O/M 	O/M
	Get System GUID 		App 	37h 	O 	O 	O
	Get Channel Authentication Capabilities 
					App 	38h 	O 	M 	O
	Get Session Challenge 		App 	39h 	O 	M 	O
	Activate Session 		App 	3Ah 	O 	M 	O
	Set Session Privilege Level 	App 	3Bh 	O 	M 	O
	Close Session 			App 	3Ch 	O 	M 	O
	Get Session Info 		App 	3Dh 	O 	M 	O
	Get AuthCode 			App	3Fh 	O 	M 	O
	Set Channel Access 		App 	40h 	O 	M 	O
	Get Channel Access 		App 	41h 	O 	M 	O
	Get Channel Info 		App 	42h 	O 	M 	O
	Set User Access 		App 	43h 	O 	M 	O
	Get User Access 		App 	44h 	O 	M 	O
	Set User Name 			App 	45h 	O 	M 	O
	Get User Name 			App 	46h 	O 	M 	O
	Set User Password 		App 	47h 	O 	M 	O
*/
/*======================================================================*/

/*----------------------------------------------------------------------*/
/*			Send Message Command				*/
/*----------------------------------------------------------------------*/
#define	IPMI_CMD_SEND_MESSAGE	0x33
/*
The Send Message command is used for bridging IPMI messages between channels,
and between the system management software (SMS) and a given channel. 
Refer to 6.13, BMC Message Bridging, for information on how the Send Message
command is used.
For IPMI v2.0 the Send Message command has been updated to include the ability
to indicate whether a message must be sent authenticated or with encryption
(for target channels on which authentication and/or encryption are supported
and configured). */
/* See section 22.7 Send Message Command */
#define BRIDGE_NO_TRACKING	0
#define BRIDGE_TRACK_REQ	1
#define	BRIDGE_SEND_RAW		2
typedef struct send_message_cmd_req {
	uchar	command;
#ifdef BF_MS_FIRST
	uchar	tracking:2,	/* [7:6] 
		    - 00b = No tracking. The BMC reformats the message for the
		    selected channel but does not track the originating channel,
		    sequence number, or address information. This option is
		    typically used when software sends a message from the system
		    interface to another media.
		    Software will typically use ‘no tracking’ when it delivers
		    sends a message from the system interface to another channel,
		    such as IPMB. In this case, software will format the 
		    encapsulated message so that when it appears on the other
		    channel, it will appear to have been directly originated by
		    BMC LUN 10b. See 6.12.1, BMC LUN 10b Routing.
		    - 01b = Track Request. The BMC records the originating channel,
		    sequence number, and addressing information for the requester,
		    and then reformats the message for the protocol of the
		    destination channel. When a response is returned, the BMC looks
		    up the requester’s information and format the response message
		    with the framing and destination address information and reformats
		    the response for delivery back to the requester. This option
		    is used for delivering IPMI Request messages from non-SMS
		    (non-system interface) channels.
		    See 6.12.3, Send Message Command with Response Tracking.
		    - 10b = Send Raw. (optional) This option is primarily provided
		    for test purposes. It may also be used for proprietary
		    messaging purposes. The BMC simply delivers the encapsulated
		    data to the selected channel in place of the IPMI Message data.
		    Note that if the channel uses sessions, the first byte of the
		    Message Data field must be a Session Handle. The BMC should
		    return a non-zero completion code if an attempt is made to 
		    use this option for a given channel and the option is not 
		    supported. It is recommended that completion code CCh be
		    returned for this condition.
		    - 11b = reserved */
		encryption:1,	/* [5]
		    - 1b = Send message with encryption. BMC will return an error
		    completion code if this encryption is unavailable.
		    - 0b = Encryption not required. The message will be sent 
		    unencrypted if that option is available under the given session.
		    Otherwise, the message will be sent encrypted. */
		authentication:1, /* [4]
		    - 1b = Send message with authentication. BMC will return an 
		    error completion code if this authentication is unavailable.
		    - 0b = Authentication not required. Note behavior is dependent
		    on whether authentication is used is depending on whether
		    the target channel is running an IPMI v1.5 or IPMI v2.0/RMCP+
		    session, as follows:
		    IPMI v1.5 sessions will default to sending the message with
		    authentication if that option is available for the session.
		    IPMI v2.0/RMCP+ sessions will send the message unauthenticated
		    if that option is available under the session. Otherwise,
		    the message will be sent with authentication. */
		channel_number:4;	/* [3:0] channel number to send message to. 
		    Each interface has a channel number that is used when 
		    configuring the channel and for routing messages between
		    channels. Only the channel number assignments for the 
		    primary IPMB and the System Interface are fixed, the 
		    assignment of other channel numbers can vary on a per-platform
		    basis. Software uses a Get Channel Info command to determine
		    what types of channels are available and what channel number
		    assignments are used on a given platform. See IPMI_CH_NUM_xx
		    definitions for fixed channel assignments. */
#else
	uchar	channel_number:4,
		authentication:1,
		encryption:1,
		tracking:2;
#endif
	uchar	message_data;	/* 2:N Message Data. Format dependent on target channel
			   type. See Table 22-10, Message Data for Send Message 
			   Command */
} SEND_MESSAGE_CMD_REQ;

typedef struct send_message_cmd_resp {
	uchar completion_code;	/* generic, plus additional command-specific
	    completion codes:
	    - 80h = Invalid Session Handle. The session handle does not match 
	    up with any currently active sessions for this channel.
	    
	    If channel medium = IPMB, SMBus, or PCI Management Bus:
	    (This status is important for applications that need to access 
	    low-level I2C or SMBus devices and should be implemented.)
	    - 81h = Lost Arbitration
	    - 82h = Bus Error
	    - 83h = NAK on Write */
	uchar response_data;	/* (2:N) Response Data
	    This data will only be present when using the Send Message command
	    to originate requests from IPMB or PCI Management Bus to other
	    channels such as LAN or serial/modem. It is not present in the
	    response to a Send Message command delivered via the System Interface.
	    NOTE: The BMC does not parse messages that are encapsulated in a 
	    Send Message command. Therefore, it does not know what privilege
	    level should associated with an encapsulated message. Thus, messages
	    that are sent to a session using the Send Message command are always
	    output using the Authentication Type that was negotiated when the
	    session was activated. */
} SEND_MESSAGE_CMD_RESP;

/* Table 22-10 summarizes the contents of the Message Data field when the Send Message command is used
to deliver an IPMI Message to different channel types. Note that in most cases the format of message information
the Message Data field follows that used for the IPMB, with two typical exceptions: When the message is
delivered to channels without physical slave devices, a software ID (SWID) field takes the place of the slave
address field. When the message is delivered to a channel that supports sessions, the first byte of the message data
holds a Session Handle.
*/


/*======================================================================*/
/*
 * Chassis Device Commands
 * 
 	Command				NetFn 	CMD 	BMC 	SHM 	IPMC
	--------------------------------------------------------------------
	Get Chassis Capabilities 	Chassis 00h 	M 	M	O
	Get Chassis Status 		Chassis 01h 	O/M 	M 	O
	Chassis Control 		Chassis 02h 	O/M 	M 	O
	Chassis Reset 			Chassis	03h 	O	O 	O
	Chassis Identify 		Chassis 04h 	O 	O 	O
	Set Chassis Capabilities 	Chassis 05h 	O 	O 	O
	Set Power Restore Policy 	Chassis 06h 	O 	O 	O
	Get System Restart Cause 	Chassis 07h 	O 	O 	O
	Set System Boot Options 	Chassis 08h 	O 	O 	O
	Get System Boot Options 	Chassis 09h 	O 	O 	O
	Get POH Counter 		Chassis 0Fh 	O 	O 	O
	
*/
/*======================================================================*/


/*======================================================================*/
/* 
 *  Event Commands
 *  
 	Command				NetFn 	CMD 	BMC 	SHM 	IPMC
	--------------------------------------------------------------------
	Set Event Receiver  		S/E 	00h 	M 	M 	M
	Get Event Receiver 		S/E 	01h 	M 	M 	M
	Platform Event  		S/E 	02h 	M 	M 	M
	(a.k.a. “Event Message”)
 * 
 *  Using NETFN_EVENT_REQ/NETFN_EVENT_RESP
 */
/*======================================================================*/
#define IPMI_EVENT_MESSAGE_REVISION	0x4

/*----------------------------------------------------------------------*/
/*			Set Event Receiver command			*/
/*----------------------------------------------------------------------*/
/* This global command tells a controller where to send Event Messages. */

#define IPMI_SE_CMD_SET_EVENT_RECEIVER	0x00	/* Set Event Receiver */
#define IPMI_SE_CMD_GET_EVENT_RECEIVER	0x01	/* Get Event Receiver */
#define IPMI_SE_PLATFORM_EVENT		0x02	/* Platform Event */

typedef struct set_event_receiver_cmd_req {
	uchar	command;
       /*Event Receiver Slave Address. 0FFh disables Event Message Generation */
	uchar	evt_receiver_slave_addr;	/* [7:1] - IPMB (I2C) Slave Address 
						   [0] - always 0b when [7:1] hold 
						         I2C slave address */
#ifdef BF_MS_FIRST
	uchar	:6,				/* [7:2] - reserved */
		evt_receiver_lun:2;		/* [1:0] - Event Receiver LUN */
#else
	uchar	evt_receiver_lun:2;
#endif
} SET_EVENT_RECEIVER_CMD_REQ;

typedef struct set_event_receiver_cmd_resp {
	uchar	completion_code;		/* Completion Code */
} SET_EVENT_RECEIVER_CMD_RESP;

typedef struct event_config {
	uchar	receiver_slave_addr;
	uchar	receiver_lun;
	uchar	evt_enabled;
} EVENT_CONFIG;


/*----------------------------------------------------------------------*/
/*			Get Event Receiver command			*/
/*----------------------------------------------------------------------*/
/* This global command is used to retrieve the present setting for the 
 * Event Receiver Slave Address and LUN. */

typedef struct get_event_receiver_cmd_resp {
	uchar completion_code;			/* Completion Code. */
	uchar	evt_receiver_slave_addr;	/* Event Receiver Slave Address.
						   0FFh indicates Event Message 
						   Generation has been disabled. 
						   Otherwise
						   [7:1] IPMB (I2C) Slave Address
						   [0] always 0b when [7:1] hold
						   I2C slave address */
#ifdef BF_MS_FIRST						   
	uchar	:6,				/* [7:2] - reserved */
		evt_receiver_lun:2;		/* [1:0] - Event Receiver LUN */
#else
	uchar	evt_receiver_lun:2;
#endif
} GET_EVENT_RECEIVER_CMD_RESP;

/*----------------------------------------------------------------------*/
/*			Platform Event					*/
/*----------------------------------------------------------------------*/

typedef struct platform_event_message_cmd_req {
	uchar	command;
	uchar EvMRev;		/* Event Message Revision. This field is
				   used to identify different revisions of 
				   the Event Message format. The revision
				   number shall be 04h for Event Messages 
				   that comply with (Intelligent Platform Management
				   Interface Specification, Second Generation,
				   v2.0, Document Revision 1.0, February 12, 2004)
				   specification.  IPMI v1.0 messages use 03h.
				   It is recommended that software be able to
				   interpret both versions. */

	uchar sensor_type;	/* Sensor Type: Indicates the event class or 
				   type of sensor that generated the Event Message. 
				   The Sensor Type Codes are specified in Table 42-3, 
				   Sensor Type Codes. */

	uchar sensor_number;	/* Sensor #. A unique number (within a given
				   sensor device) representing the ‘sensor’ 
				   within the management controller that 
				   generated the Event Message. Sensor numbers
				   are used for both identification and access
				   of sensor information, such as getting and 
				   setting sensor thresholds. */
#ifdef BF_MS_FIRST
	uchar	event_dir:1,	/* Event Dir. Indicates the event transition
				   direction. (0 = Assertion Event, 
				   1 = Deassertion Event) */
		event_type:7;	/* This field indicates the type of threshold
				   crossing or state transition (trigger) that
				   produced the event. This is encoded using
				   the Event/Reading Type Code. See Section 
				   42, Sensor and Event Code Tables. */
	/* The remainder of the Event Message data according to the class of 
	 * the Event Type for the sensor (threshold, discrete, or OEM). The 
	 * contents and format of this field is specified in Table 29-6, Event 
	 * Request Message Event Data Field Contents, */
#else
	uchar	event_type:7,
		evet_dir:1;
#endif
	uchar event_data1;	/* Event Data 1 */
	uchar event_data2;	/* Event Data 2 */
	uchar event_data3;	/* Event Data 3 */
} PLATFORM_EVENT_MESSAGE_CMD_REQ;


typedef struct event_log_entry {
	uchar	evt_msg_rev;
	uchar	sensor_type;
	uchar	sensor_number;
	uchar	evt_direction;
	uchar	evt_data1;
	uchar	evt_data2;
	uchar	evt_data3;
	unsigned timestamp;
	unsigned record_id;
} EVENT_LOG_ENTRY;

typedef struct generic_event_msg {
	uchar	evt_msg_rev;
	uchar	sensor_type;
	uchar	sensor_number;
	uchar	evt_direction;
	uchar	evt_data1;
	uchar	evt_data2;
	uchar	evt_data3;
} GENERIC_EVENT_MSG;
/*======================================================================*/
/*
 *  PEF and Alerting Mandatory Commands
 *  
 	Command				NetFn 	CMD 	BMC 	SHM 	IPMC
	--------------------------------------------------------------------
	Get PEF Capabilities 		S/E 	10h 	M 	M 	M
	Arm PEF Postpone Timer 		S/E 	11h 	M 	M 	M
	Set PEF Configuration Parameters S/E 	12h 	M 	M 	M
	Get PEF Configuration Parameters S/E 	13h 	M 	M 	M
	Set Last Processed Event ID 	S/E 	14h 	M 	M 	M
	Get Last Processed Event ID 	S/E 	15h 	M 	M 	M
	Alert Immediate 		S/E 	16h 	O 	O 	O
	PET Acknowledge 		S/E 	17h 	O 	O 	O
 * 
 *  Using NETFN_EVENT_REQ/NETFN_EVENT_RESP
 */
/*======================================================================*/
#define IPMI_SE_CMD_GET_PEF_CAPABILITIES	0x10	/* Get PEF Capabilities */
#define IPMI_SE_CMD_ARM_PEF_POSTPONE_TIMER	0x11	/* Arm PEF Postpone Timer */
#define IPMI_SE_CMD_SET_PEF_CONFIG_PARAMS	0x12	/* Set PEF Configuration Parameters */
#define IPMI_SE_CMD_GET_PEF_CONFIG_PARAMS	0x13	/* Get PEF Configuration Parameters */
#define IPMI_SE_CMD_SET_LAST_PROCESSED_EVENT	0x14	/* Set Last Processed Event */
#define IPMI_SE_CMD_GET_LAST_PROCESSED_EVENT	0x15	/* Get Last Processed Event */
#define IPMI_SE_CMD_ALERT_IMMEDIATE		0x16	/* Alert Immediate */
#define IPMI_SE_CMD_PET_ACKNOWLEDGE		0x17	/* PET Acknowledge*/


/*----------------------------------------------------------------------*/
/*			Get PEF Capabilities Command			*/
/*----------------------------------------------------------------------*/
typedef struct get_pef_capabilities_cmd_resp {
	uchar	completion_code;	/* Completion Code */
	uchar	pef_version;		/* PEF Version (BCD encoded, LSN first, 
					   51h for this specification. 51h ==
					   version 1.5) */
	uchar	action_support;		/* Action Support - These are the 
					   supported actions that the BMC 
					   can take on event messages that
					   it receives or has internally 
					   generated.
					   
					   [7:6] - reserved
					   [5] - 1b = diagnostic interrupt
					   [4] - 1b = OEM action
					   [3] - 1b = power cycle
					   [2] - 1b = reset
					   [1] - 1b = power down
					   [0] - 1b = Alert */
	uchar	num_evt_filter_tbl_entries;	/* Number of event filter table entries (1 based) */
} GET_PEF_CAPABILITIES_CMD_RESP;

/* PEF Action types */
#define PEF_ACTION_GROUP_CONTROL	0x40 /* group control operation (see [ICMB]) */
#define PEF_ACTION_DIAG_INTERRUPT	0x20
#define PEF_ACTION_OEM_ACTION		0x10
#define PEF_ACTION_POWER_CYCLE		0x08
#define PEF_ACTION_RESET		0x04
#define PEF_ACTION_POWER_DOWN		0x02
#define PEF_ACTION_ALERT		0x01

/* PEF Action Priorities */

#define PEF_ACTION_PRIORITY_POWER_DOWN	1	/* (optional) */
#define PEF_ACTION_PRIORITY_POWER_CYCLE	2	/* (optional) Will not be executed
						   if a power down action was also 
						   selected. */
#define PEF_ACTION_PRIORITY_RESET	3	/* (mandatory) Will not be executed
						   if a power down or power cycle
						   action was also selected. */
#define PEF_ACTION_PRIORITY_DIAG_INTERRUPT	4	/* (optional) The diagnostic
						   interrupt will not occur if a
						   higher priority action is also
						   selected to occur. */
#define PEF_ACTION_PRIORITY_ALERT	5	/* (mandatory if alerting is s
						   upported) Send alerts in order
						   based on the selected Alert Policy.
						   Alert actions will be deferred 
						   until after the power down has
						   completed. There is an additional
						   prioritization within alerts being
						   sent: based on the Alert Policy 
						   Table entries for the alert. This
						   is described further in Section
						   17.11, Alert Policy Table. */
#define PEF_ACTION_PRIORITY_OEM 	6	/* (optional) Priority determined 
						   by OEM.*/

typedef struct pef_capabilities {
	uchar	pef_version;
	uchar	action_support;
	uchar	num_evt_filter_tbl_entries;
	uchar	pef_postpone_timeout_state;	// enabled, disabled, temporary disabled
	uchar	pef_postpone_timeout_value;
	uchar	config_param_data;
} PEF_CAPABILITIES;

#define PEF_POSTPONE_TIMEOUT_DISABLED		0
#define PEF_POSTPONE_TIMEOUT_ENABLED		1
#define PEF_POSTPONE_TIMEOUT_TEMP_DISABLED	3

/*----------------------------------------------------------------------*/
/*			Arm PEF Postpone Timer Command			*/
/*----------------------------------------------------------------------*/
typedef struct arm_pef_postpone_timer_cmd_req {
	uchar	command;
	uchar	pef_postpone_timeout;	/* [7:0] - PEF Postpone Timeout, 
					   in seconds. 01h == 1 second.
					   00h = disable Postpone Timer 
					   (PEF will immediately handle events, 
					   if enabled). The BMC automatically 
					   disables the timer whenever the
					   system enters a sleep state, is powered 
					   down, or reset.
					   01h - FDh = arm timer. Timer will 
					   automatically start counting down 
					   from given value when the last-processed 
					   event Record ID is not equal to the last
					   received event’s Record ID.
					   FEh = Temporary PEF disable. The PEF 
					   Postpone timer does not countdown from 
					   the value. The BMC automatically re-enables 
					   PEF (if enabled in the PEF configuration 
					   parameters) and sets the PEF Postpone timeout
					   to 00h whenever the system enters a sleep 
					   state, is powered down, or reset. Software 
					   can cancel this disable by setting this 
					   parameter to 00h or 01h-FDh.
					   FFh = get present countdown value */
} ARM_PEF_POSTPONE_TIMER_CMD_REQ;

typedef struct arm_pef_postpone_timer_cmd_resp {
	uchar	completion_code;	/* Completion Code */
	uchar	present_timer_countdown_value;	/* Present timer countdown value */
} ARM_PEF_POSTPONE_TIMER_CMD_RESP;

/*----------------------------------------------------------------------*/
/*			Set PEF Configuration Parameters command	*/
/*----------------------------------------------------------------------*/
typedef struct set_pef_config_params_cmd_req {
	uchar	command;
	uchar	param_selector;		/* Parameter selector
					   [7] - reserved
					   [6:0] - Parameter selector */
	uchar	config_param_data;	/* 2:N Configuration parameter data, 
					   per Table 24-6, PEF Configuration Parameters. */
} SET_PEF_CONFIG_PARAMS_CMD_REQ;

typedef struct set_pef_config_params_cmd_resp {
	uchar	completion_code;	/* Completion Code. Generic plus the 
					   following command-specific completion
					   codes:
					   80h = parameter not supported.
					   81h = attempt to set the ‘set in progress’ 
					   value (in parameter #0) when not in
					   the ‘set complete’ state. (This completion 
					   code provides a way to recognize that 
					   another party has already ‘claimed’ the 
					   parameters)
					   82h = attempt to write read-only parameter */
} SET_PEF_CONFIG_PARAMS_CMD_RESP;

/*----------------------------------------------------------------------*/
/*			Get PEF Configuration Parameters command	*/
/*----------------------------------------------------------------------*/
typedef struct get_pef_config_params_cmd_req {
	uchar	command;
#ifdef BF_MS_FIRST
	uchar	rev_selector:1,		/* [7] - 1b = get parameter revision only.
					         0b = get parameter */
	param_selector:7;		/* [6:0] - Parameter selector */
#else
	uchar	param_selector:7,
	rev_selector:1;
#endif
	uchar	set_selector;		/* Set Selector (00h if parameter does not 
					   require a Set Selector) */
	uchar	block_selector;		/* Block Selector (00h if parameter does not
					   require a block number) */
} GET_PEF_CONFIG_PARAMS_CMD_REQ;

typedef struct get_pef_config_params_cmd_resp {
	uchar	completion_code;	/* Completion Code. Generic plus the
					   following command-specific completion
					   codes:
					   80h = parameter not supported */
	uchar	param_rev;		/* [7:0] - Parameter revision. */
	/* Format: MSN = present revision. LSN = oldest revision parameter is
	 * backward compatible with. 11h for parameters in this specification.
	 * The following data bytes are not returned when the ‘get parameter 
	 * revision only’ bit is 1b. */
	uchar	config_param_data;	/* 3:N Configuration parameter data, 
					   per Table 30-6, PEF Configuration 
					   Parameters */
	/*  If the rollback feature is implemented, the BMC makes a copy of the
	 *  existing parameters when the ‘set in progress’ state becomes asserted 
	 *  (See the Set In Progress parameter #0). While the ‘set in progress’ 
	 *  state is active, the BMC will return data from this copy of the 
	 *  parameters, plus any uncommitted changes that were made to the data. 
	 *  Otherwise, the BMC returns parameter data from non-volatile storage. */
} GET_PEF_CONFIG_PARAMS_CMD_RESP;

/*----------------------------------------------------------------------*/
/*			PEF Configuration Parameters			*/
/*----------------------------------------------------------------------*/

/* Parameter types */

#define PEF_CONFIG_PARAM_SET_IN_PROGRESS		0
#define PEF_CONFIG_PARAM_PEF_CONTROL			1
#define PEF_CONFIG_PARAM_PEF_ACTION_GLOBAL_CONTROL	2
#define PEF_CONFIG_PARAM_PEF_STARTUP_DELAY		3
#define PEF_CONFIG_PARAM_PEF_ALERT_STARTUP_DELAY	4
#define PEF_CONFIG_PARAM_NUMBER_OF_EVENT_FILTERS	5
#define PEF_CONFIG_PARAM_EVENT_FILTER_TABLE		6
#define PEF_CONFIG_PARAM_EVENT_FILTER_TABLE_DATA_1	7
#define PEF_CONFIG_PARAM_NUMBER_OF_ALERT_POLICY_ENTRIES	8
#define PEF_CONFIG_PARAM_ALERT_POLICY_TABLE		9
#define PEF_CONFIG_PARAM_SYSTEM_GUID			10
#define PEF_CONFIG_PARAM_NUMBER_OF_ALERT_STRINGS	11
#define PEF_CONFIG_PARAM_ALERT_STRING_KEYS		12
#define PEF_CONFIG_PARAM_ALERT_STRINGS			13
#define PEF_CONFIG_PARAM_NUM_GROUP_CONTROL_TABLE_ENTRIES	14
#define PEF_CONFIG_PARAM_GROUP_CONTROL_TABLE		15

/* Parameter data structs */

typedef struct pef_set_in_progress {
	/* data 1 - This parameter is used to indicate when any of the following
	parameters are being updated, and when the updates are completed.
	The bit is primarily provided to alert software than some other software or
	utility is in the process of making changes to the data.
	An implementation can also elect to provide a ‘rollback’ feature that uses
	this information to decide whether to ‘roll back’ to the previous
	configuration information, or to accept the configuration change.
	If used, the roll back shall restore all parameters to their previous state.
	Otherwise, the change shall take effect when the write occurs. */
#ifdef BF_MS_FIRST
	uchar 	:6,		/* [7:2] - reserved */
		set_complete:2;	/* [1:0] 
	- 00b = set complete. If a system reset or transition to powered down state
	occurs while ‘set in progress’ is active, the BMC will go to the ‘set complete’
	state. If rollback is implemented, going directly to ‘set complete’ without 
	first doing a ‘commit write’ will cause any pending write data to be discarded.
	- 01b = set in progress. This flag indicates that some utility or other
	software is presently doing writes to parameter data. It is a notification flag
	only, it is not a resource lock. The BMC does not provide any interlock 
	mechanism that would prevent other software from writing parameter data while.
	- 10b = commit write (optional). This is only used if a rollback is implemented.
	The BMC will save the data that has been written since the last time the ‘set 
	in progress’ and then go to the ‘set in progress’ state. An error completion 
	code willbe returned if this option is not supported.
	- 11b = reserved */
#else
	uchar	set_complete:2;
#endif
} PEF_SET_IN_PROGRESS;


typedef struct pef_control {
#ifdef BF_MS_FIRST
	uchar	:4,	/* [7:4] - reserved */
		pef_alert_startup_delay_enable:1,	
		/* [3] - PEF Alert Startup Delay disable. (optional)
		   1b = enable PEF Alert Startup delay
		   0b = disable PEF startup delay. */
		pef_startup_delay_enable:1,
		/* [2] - PEF Startup Delay disable. (optional)
		   An implementation that supports this bit should also provide
		   a mechanism that allows the user to Disable PEF in case the 
		   filter entries are programmed to cause an ‘infinite loop’
		   of PEF actions (such as system resets or power cycles) when 
		   the PEF startup delay is disabled. If this bit is not
		   implemented the PEF startup delay must always be enabled.
		   1b = enable PEF startup delay on manual (pushbutton) system
		   power-ups (from S4/S5) and system resets (including
		   system resets initiated by PEF).
		   0b = disable PEF startup delay. */
		pef_actions_event_messages_enable:1,
		/* - 1b = enable event messages for PEF actions. If this bit is
		   set, each action triggered by a filter will generate an event
		   message for the action. These allow the occurrence of PEF
		   triggered actions to be logged (if event logging is enabled).
		   The events are logged as System Event Sensor 12h, offset
		   04h. See Table 42-3, Sensor Type Codes.) These event
		   messages are also subject to PEF.
		   - 0b = disable event messages for PEF actions. */
		pef_enable:1;	/* [0] - 1b = enable PEF.
				       - 0b = disable PEF. */
#else
	uchar	pef_enable:1,
		pef_actions_event_messages_enable:1,
		pef_startup_delay_enable:1,
		pef_alert_startup_delay_enable:1;
#endif
} PEF_CONTROL;

typedef struct pef_action_global_control {
#ifdef BF_MS_FIRST
	uchar	:2,				/* [7:6] - reserved */
		enable_diagnostic_interrupt:1,	/* [5] - 1b = enable diagnostic interrupt */
		enable_oem_action:1,		/* [4] - 1b = enable OEM action */
		enable_power_cycle_action:1,	/* [3] - 1b = enable power cycle 
						   action (No effect if power is 
						   already off) */
		enable_reset_action:1,		/* [2] - 1b = enable reset action */
		enable_power_down_action:1,	/* [1] - 1b = enable power down action */
		enable_alert_action:1;		/* [0] - 1b = enable Alert action */
#else
	uchar	enable_alert_action:1,
		enable_power_down_action:1,
		enable_reset_action:1,
	   	enable_power_cycle_action:1,
      		enable_oem_action:1,
		enable_diagnostic_interrupt:1;		  
#endif
} PEF_ACTION_GLOBAL_CONTROL;

typedef struct pef_startup_delay {
	/* data 1 - time to delay PEF after a system power-ups (from S4/S5) and
	resets. Default = 60 seconds. If this parameter is not provided, the
	default PEF Startup Delay must be implemented. Enable/disable of the
	delay is configured using the PEF Control parameter, above. If this
	parameter is supported, a 00h value can also be used to disable the
	delay if necessary. See Section 17.4, PEF Startup Delay, for more
	information.
	Note: An implementation that supports this parameter should also
	provide a mechanism that allows the user to Disable PEF in case the
	filter entries are programmed to cause an ‘infinite loop’ of PEF actions
	under the situation where this parameter is set to too short an interval to
	allow a user to locally disable PEF. An implementation is allowed to force
	this parameter to a minimum, non-zero value. */
	uchar delay;		/* [7:0] - PEF Startup Delay in seconds,
				   +/- 10%. 1-based. 00h = no delay. */
} PEF_STARTUP_DELAY;


typedef struct pef_alert_startup_delay {
	/* data 1 - time to delay Alerts after system power-ups (from S4/S5) and
	resets. Default = platform-specific. 60-seconds typical, though may be
	longer on systems that require more startup time before user can take
	action to disable PEF. If this parameter is not provided, a default PEF
	Startup Delay, appropriate for the platform, must be implemented.
	Enable/disable of the delay can also be optionally configured using the
	PEF Control parameter, above. An implementation can separately
	implement this parameter and/or the enable/disable bit. */
	uchar delay;		 /* [7:0] - PEF Alert Startup Delay in 
				   seconds, +/- 10%. 1-based.
				   00h = no delay. */
} PEF_ALERT_STARTUP_DELAY;

typedef struct pef_num_event_filters {
	/* Number of event filters supported. 1-based. This parameter does not
	need to be supported if Alerting is not supported. */
	uchar	:1,		/* [7] - reserved */
		num_event_filters;	/* [6:0] - number of event filter entries.
					   0 = alerting not supported. */
} PEF_NUM_EVENT_FILTERS;

typedef struct pef_event_filter_table_data1 {
	/* This parameter provides an aliased access to the first byte of the event
	filter data. This is provided to simplify the act of enabling and disabling
	individual filters by avoiding the need to do a read-modify-write of the
	entire filter data. */
	/* data 1 - Set Selector = filter number */
#ifdef BF_MS_FIRST
	uchar	:1,		/* [7] - reserved */
		filter_number:7;	/* [6:0] - Filter number. 1-based. 00h = reserved. */
#else
	uchar	filter_number:7;
#endif
	uchar	data;		/* data 2 - data byte 1 of event filter data */
} PEF_EVENT_FILTER_TABLE_DATA1;

#define PEF_FILTER_CONFIG_MANUF		2	/*  manufacturer pre-configured filter */
#define PEF_FILTER_CONFIG_SW		0	/* software configurable filter */

#define PEF_EVT_SEVERITY_UNSPECIFIED		0x00 
#define PEF_EVT_SEVERITY_MONITOR		0x01  
#define PEF_EVT_SEVERITY_INFORMATION		0x02  
#define PEF_EVT_SEVERITY_OK			0x04  
#define PEF_EVT_SEVERITY_NON_CRITICAL_COND	0x08  
#define PEF_EVT_SEVERITY_CRITICAL_COND		0x10 
#define PEF_EVT_SEVERITY_NON_RECOVERABLE	0x20 

typedef struct pef_event_filter_table_entry {
#ifdef BF_MS_FIRST
	uchar	enable_filter:1,	/* 1b = enable filter / 0b = disable filter */
		filter_config:2;	/* 11b = reserved
					   10b = manufacturer pre-configured filter. 
					   (PEF_FILTER_CONFIG_MANUF)
					   The filter entry has been configured by
					   the system integrator and should not be
					   altered by software. Software is allowed
					   to enable or disable the filter, however.
					   01b = reserved
					   00b = software configurable filter. 
					   (PEF_FILTER_CONFIG_SW)
					   The filter entry is available for 
					   configuration by system management 
					   software. */
#else
	uchar	:5,			/* reserved */
		filter_config:2,
		enable_filter:1;
#endif
	uchar	event_filter_action;	/* PEF_ACTION_nn
					   All actions are optional for an 
					   implementation, with the exception of
					   Alert which is mandatory if alerting
					   is supported for one or more channels.
					   The BMC will return 0b for unsupported
					   actions. Software can test for which
					   actions are supported by writing 1’s
					   to the specified fields and reading
					   back the result. (Note that reserved
					   bits must be written with 0’s) */
#ifdef BF_MS_FIRST
	uchar	:1,			/* reserved */
		grp_control_selector:3,	/* group control selector (1-based).
					   Selects entry from group control
					   table. (see [ICMB]) */
		policy_number:4;		/* policy number. Value is ‘don’t care’
					   if Alert is not selected in the
					   Event Filter Action. */
#else
	uchar	policy_number:4,
		grp_control_selector:3;
#endif
	uchar	event_severity;		/* This field can be used to fill in 
					   the Event Severity field in a PET alert.
					   The severity values are based on the 
					   ‘DMI’ severity values used for the
					   generic sensor event/reading type code.
					   In the case that more than one
					   event filter match occurs for a given
					   Alert Policy Number, the numerically
					   highest severity value will be used.
					   (PEF_EVT_SEVERITY_nn) */

	uchar 	generator_id_1;		/* Slave Address or Software ID from
					   Event Message. FFh = match any */
	uchar	generator_id_2;		/* Channel Number / LUN to match. 
					   FFh = match any see section 32, 
					   SEL Record Formats. */
	uchar	sensor_type;		/* FFh = match any */
	uchar	sensor_number;		/* FFh = match any */
	uchar	event_trigger;		/* FFh = match any */
	
	/* The following bit field is used to match different values of the 
	   least significant nibble of the Event Data 1 field. This enables
	   a filter to provide a match on multiple event offset values.
	   Bit positions 15 through 0 correspond to the offset values Fh - 0h,
	   respectively. A 1 in a given bit position will cause a match if the
	   value in bits 3:0 of the Event Data 1 hold the corresponding value 
	   for the bit position. Multiple mask bits can be set to 1, enabling
	   a match to multiple values. A match must be made with this field in
	   order to have a match for the filter. 
	   
	   The least significant nibble of event data 1 typically holds an event
	   offset value. This offset selects among different possible events for
	   a sensor. For example, a ‘button’ sensor supports a set of sensor-
	   specific event offsets: 0 for Power Button pressed, 1 for Sleep Button
	   pressed, and 2 for Reset Button pressed. 
	   
	   When an event is generated, it could have a 0, 1, or 2 in the event
	   offset field depending on what button press occurred.
	   
	   The Event Offset Mask makes it simple to have a filter match a subset
	   of the possible event offset values. Each bit in the mask corresponds
	   to a different offset values starting with bit 0 in the mask 
	   corresponding to offset 0. For example, if it is desired to have a
	   filter match offsets 0 and 2, but not 1, the mask would be configured
	   to 000_0000_0000_0101b.
	   */
	uchar	event_data1_event_offset_mask1; /* mask bit positions 7 to 0, respectively. */
	uchar	event_data1_event_offset_mask2; /*  mask bit positions 15 to 8, respectively. */

	/* The following value is applied to the entire Event Data 1 byte. The
	   field is Used to indicate ‘wildcarded’ or ‘compared’ bits. This field
	   must be used in conjunction with Compare 2. To match any Event Data 
	   field value, just set the corresponding AND Mask, Compare 1, and 
	   Compare 2 fields to 00h. (See Section 17.8, Event Data 1 Event 
	   Offset Mask for more information).
	   Note that the Event Data 1 AND mask, Compare 1 mask, and Compare 2
	   masks will typically be set to wild-card the least significant of 
	   Event Data 1 in order to allow the Event Data 1 Event Mask field to
	   determine matches to the event offset.

	   Bits 7:0 all have the following definition:
	   0 = Wildcard bit. (drops this bit position in the Event Data byte out of
	   the comparison process) Corresponding bit position must be a 1 in
	   Compare 1, and a 0 in Compare 2. 
	   (Note - setting a 0 in this bit, a 1 and Compare 1 and a 1 in
	   Compare 2 guarantees that you’ll never have a match.)
	   1 = use bit for further ‘exact’ or ‘non-exact’ comparisons based on
	   Compare 1 and Compare 2 values. */
	uchar	event_data1_and_mask;

	/* The following field is used to indicate whether each bit position’s 
	   comparison is an exact comparison or not. (See Section 17.8, Event 
	   Data 1 Event Offset Mask for more information). Here, ‘test value’ 
	   refers to the Event Data value after the AND Mask has been applied.

	   Bits 7:0 all have the following definition:
	   
	   1 = match bit in test value exactly to correspond bit position in
	   Compare 2
	   
	   0 = contributes to match if corresponding bit in test value matches
	   corresponding bit in Compare 2. */
	uchar	event_data1_compare1;
	
	/* For the following field, (See Section 17.8, Event Data 1 Event Offset
	   Mask for more information), here, ‘test value’ refers to the Event
	   Data value after the AND Mask has been applied.

	   Bits 7:0 all have the following definition:
	   1 = match a ‘1’ in corresponding bit position in test value.
	   0 = match a ‘0’ in corresponding bit position in test value. */
	uchar	event_data1_compare2;

	uchar	event_data2_and_mask;
	uchar	event_data2_compare1;
	uchar	event_data2_compare2;

	uchar	event_data3_and_mask;
	uchar	event_data3_compare1;
	uchar	event_data3_compare2;
	 
} PEF_EVENT_FILTER_TABLE_ENTRY;

typedef struct pef_mask {
	uchar	AND_mask;
	uchar	compare1;
	uchar	compare2;
} PEF_MASK;

typedef struct pef_event_filter_table {
	/* data 1 - Set Selector = filter number. */
#ifdef BF_MS_FIRST
	uchar	:1,			/* [7] - reserved. */
		filter_number:7;	/* [6:0] - Filter number. 1-based. 00h = reserved. */
#else
	uchar	filter_number:7;
#endif
	PEF_EVENT_FILTER_TABLE_ENTRY	filter_data;	/* data 2:21 - filter data (20 bytes) 
				   This field to be filled in from the filter 
				   data for this filter number. See 
				   PEF_EVENT_FILTER_TABLE_ENTRY struct */
} PEF_EVENT_FILTER_TABLE;

typedef struct pef_num_alert_policy_entries {
	/* Number of alert policy entries supported. 1-based. This parameter does
	not need to be supported if Alerting is not supported. */
#ifdef BF_MS_FIRST
	uchar	:1,				/* [7] - reserved */
		num_alert_policy_entries:7; 	/* [6:0] - number of alert policy 
						   entries. 0 = alerting not supported. */
#else
	uchar	num_alert_policy_entries:7;
#endif
} PEF_NUM_ALERT_POLICY_ENTRIES;

typedef struct pef_alert_policy_table {
	/* data 1 - Set Selector = entry number */
#ifdef BF_MS_FIRST
	uchar	:1,				/* [7] - reserved */
		alert_policy_entry_number:7;	/* [6:0] - alert policy entry number. 1-based. */
#else
	uchar	alert_policy_entry_number:7;
#endif
	uchar	data;		/* data 2:4 - 3 byte entry data 
   				   This field to be filled in from alert policy 
				   tables for this alert policy entry number */
} PEF_ALERT_POLICY_TABLE;

typedef struct pef_alert_policy_table_entry {
	/* Policy Number / Policy: This value identifies the entries belonging 
	   to a particular policy set. When an Alert Action is taken, the BMC
	   will scan the Alert Policy Table and will attempt to generate alerts
	   based on the entries that form the policy set. */
#ifdef BF_MS_FIRST
	uchar	policy_number:4,	/* [7:4] - policy number. 1 based. 0000b = reserved. */
		entry_enabled:1,	/* [3] - 0b = this entry is disabled. 
					   Skip to next entry in policy, if any.
					   1b = this entry is enabled. */
		policy:3;	/* [2:0] - policy
				   0h = always send alert to this destination.
				   1h = if alert to previous destination was successful, 
				        do not send alert to this destination.
				        Proceed to next entry in this policy set.
				   2h = if alert to previous destination was successful, 
				        do not send alert to this destination. Do not 
					process any more entries in this policy set.
				   3h = if alert to previous destination was successful,
				        do not send alert to this destination. Proceed
				       	to next entry in this policy set that is to
				        a different channel.
				   4h = if alert to previous destination was successful,
				        do not send alert to this destination. Proceed to next 
				        entry in this policy set that is to a
				        different destination type. */
#else
	uchar	policy:3,
		entry_enabled:1,
		policy_number:4;
#endif
	/* Channel / Destination Channel that the alert is to be sent over. 
	   Channel determines which set of destination addresses or phone numbers
	   is used. Destination addresses and/or phone numbers are set via
	   the LAN and/or serial/modem configuration parameter commands. The
	   Alert Type (e.g. PET, TAP, Dial Page, etc.) is specified in the 
	   configuration parameters associated with the specified destination. */
#ifdef BF_MS_FIRST
	uchar	channel_number:4,	/* [7:4] = Channel Number. */
		destination:4;		/* [3:0] = Destination selector. */
	/* Alert String Key: This field holds information that is used to look
	   up the Alert String to send for this Alert Policy entry.00h = no alert string. */
#else
	uchar	destination:4,
		channel_number:4;
#endif
#ifdef BF_MS_FIRST
	uchar	alert_string_event_specific:1,	/* [7] - Event-specific Alert String
			    1b = Alert String look-up is event specific. 
			        The following Alert String Set / Selector subfield
				is interpreted as an Alert String Set Number that 
				is used in conjunction with the Event Filter Number
				to lookup the Alert String from the PEF Configuration
				Parameters.
			    0b = Alert String is not event specific. The following
			        Alert String Set / Selector sub-field is interpreted
				as an Alert String Selector that provides a direct
				pointer to the desired Alert String from the PEF 
				Configuration Parameters. */
		alert_string_selector:7;  /* [6:0] - Alert String Set / Selector. 
				This value identifies one or more Alert Strings in the 
				Alert String table. When used as an Alert String Set 
				Number, it is used in conjunction with the Event Filter
			       	Number to uniquely identify an Alert String. When used
			       	as an Alert String Selector it directly selects an Alert
			       	String from the PEF Configuration Parameters.
				The Alert String Key and lookup mechanism allows the Alert
			       	String to be ‘Event Specific’ - meaning the string selection
			       	is determined by both the Event Policy Entry and Event
				Filter, or, the string can be selected by the Event Policy
			       	alone. An Alert String can be pointed to by multiple policy 
				entries. The Alert Policy Entry identifies a particular 
				channel and destination for an alert. This in turn,
				identifies the alert type. Thus, the binding of an Alert
			       	Policy Entry and an Alert String effectively provides
			       	a mechanism for allowing different Alert Strings to be
			       	selected based on the alert destination, or the type of
			       	alert destination. For example, a single Alert String 
				could be shared among all Alert Policy Entries for 
				‘Dial Page’ destinations, while event-specific Alert
			       	Strings could be used for alerts to LAN destinations. */	
#else
	uchar	alert_string_selector:7,
		alert_string_event_specific:1;
#endif
} PEF_ALERT_POLICY_TABLE_ENTRY;

typedef struct pef_system_guid {
       /* data 1 Used to fill in the GUID field in a PET Trap. Stored per Table
	* 20-10, GUID Format. */
#ifdef BF_MS_FIRST
	uchar	:7,			/* [7:1] - reserved */
		system_guid_follows:1;	/* [0] 1b = BMC uses following value in PET Trap.
				               0b = BMC ignores following value and uses 
				               value returned from Get System GUID command
				               instead. */
#else
	uchar	system_guid_follows:1;
#endif
	uchar	system_guid[15];	/* 2:17 - System GUID */
} PEF_SYSTEM_GUID;

typedef struct pef_num_alert_strings {
	/* Number of alert strings supported in addition to Alert String 0. 1-based.
	 * This parameter does not need to be supported if Alerting is not supported. */
#ifdef BF_MS_FIRST
	uchar	:1,			/* [7] - reserved */
		num_alert_strings:7;	/* [6:0] - number of alert strings. */
#else
	uchar	num_alert_strings:7;
#endif
} PEF_NUM_ALERT_STRINGS;

typedef struct pef_alert_string_keys {
       /* Sets the keys used to look up Alert String data in PEF. This parameter
	* does not need to be supported if Alerting is not supported.
	* data 1 - Set Selector = Alert string selector. */
#ifdef BF_MS_FIRST
	uchar	:1,			/* [7] - reserved. */
		string_selector:7;	/* [6:0] - string selector.
					   0 = selects volatile string parameters
					   01h-7Fh = non-volatile string selectors */
#else
	uchar	string_selector:7;
#endif
	/* 		
	PEF uses the following Event Filter Number and the Alert String Key
	fields to look up the string associated with a particular event. String 0 is a
	special, volatile string reserved for use by the Alert Immediate command.
	The following two fields are used by PEF to look up a particular Alert
	String based on information obtained from the alert policy entry. The
	fields should typically be set to 0’s (unspecified) for string selector 0. PEF
	will scan the values for string 0 when doing a look up, so the string 0
	values can be set to non-zero values for PEF testing/debug purposes in
	order to avoid writes to non-volatile storage.
	data 2 - Event Filter Number
	*/
#ifdef BF_MS_FIRST
	uchar	:1,		/* [7] - reserved. */
		filter_number:7;	/* [6:0] - Filter number. 1-based. 00h = unspecified. */
#else
	uchar	filter_number:7;
#endif
	/*
	data 3 - Alert String Set
	*/
#ifdef BF_MS_FIRST
	uchar	:1,		/* [7] - reserved */
		set_number:7;	/* [6:0] - Set number for string. 1-based. 00h = unspecified. */
#else
	uchar	set_number:7;
#endif
} PEF_ALERT_STRING_KEYS;

typedef struct pef_alert_strings {
	/*
	Sets the Alert String data. The string data that should be used is
	dependent on the Channel and Alert Type. This parameter does not need
	to be supported if Alerting is not supported.

	For Dial paging, the BMC automatically follows the string with a <CR>
	(carriage return) character when sending it to the modem.

	For TAP paging the string corresponds to ‘Field 2’, the Pager Message.
	Note that while the string accepts 8-bit ASCII data, the TAP
	implementation only supports 7-bit ASCII.

	The BMC shall automatically zero the 8th bit when transmitting the string
	during TAP paging.

	String 0 is a special, volatile string reserved for use by the Alert
	Immediate command.

	data 1 - Set Selector = string selector. 
	*/
#ifdef BF_MS_FIRST
	uchar	:1,			/* [7] - reserved. */
		string_selector:7;	/* [6:0] - string selector. 
					   0 = selects volatile string
					   01h-7Fh = non-volatile string selectors */
#else
	uchar	string_selector:7;
#endif
	/* 
	data 2 - Block Selector = string block number to set, 1 based. Blocks
	are 16 bytes.
	*/
	uchar	block_selector;
	uchar	data[16];	/* data 3:N - String data. Null terminated 
				   8-bit ASCII string. 16-bytes max. per block. */
} PEF_ALERT_STRINGS;

typedef struct pef_num_grp_ctrl_tbl_entries {
	/*
	(optional. Present if BMC supports automatic ICMB Group Power Control.
	See ICMB specification for details.)
	data 1 - Number of group control table entries. 1-based (4 min, 8 max) */
	uchar num_entries;
} PEF_NUM_GRP_CTRL_TBL_ENTRIES;

typedef struct pef_group_control_table {
	/*
	(optional, non-volatile.  Present if BMC supports automatic ICMB Group
	Power Control. See ICMB specification for details.)
	data 1 - Set Selector = group control table entry selector.
	*/
#ifdef BF_MS_FIRST
	uchar	:1,			/* [7] - reserved. */
		table_entry_selector:7;	/* [6:0] - group control table entry selector. */
#else
	uchar	table_entry_selector:7;
#endif
	/* data 2 - */
#ifdef BF_MS_FIRST
	uchar	:2,			/* [7:6] - reserved */
		request_force:1,	/* [5] - Request/Force
			0b = request control operation. A requested operation will only
			complete once the same operation has been requested for
			all control groups and all enabled control members for the
			given chassis.
			1b = force control operation. A forced operation will occur
			regardless of whether the same operation has been
			requested for all control groups and all enabled control
			membership for the given chassis. */
		immed_delayed:1,	/* [4] - Immediate/Delayed. 
			Selects whether the BMC requests an immediate or delayed 
			control operation. Note: whether this operation is 
			initiated at the time the command is received is
			dependent on the request/force bit, see above.
			0b = immediate control. BMC sends command that requests an
			immediate control operation.
			1b = delayed control. BMC sends control command to request a
			delayed control operation. This is conditioned by the
			request/force bit. */
		channel_number:4;	/* [3:0] - Channel Number (channel number for 
			ICMB that group control operation is to be delivered over) */
#else
	uchar	channel_number:4,
		immed_delayed:1,
		request_force:1,
		:2;
#endif

	/* data 3: */
	uchar	group_id_0;	/* Group ID 0 (1-based)
				   00h = unspecified
				   FFh = all groups */
	/* data 4: */
#ifdef BF_MS_FIRST
	uchar	:3,		/* Member ID 0 (0-based) [7:5] - reserved */
		enable_member_id_0_check:1,	/* [4] - 0b = enable member ID check.
						   1b = disable member ID check. */
		member_id_0:4;	/* [3:0] - member ID. ID of this chassis within
				   specified group. (value is ignored if Group 
				   ID 0 = FFh) */
#else
	uchar	member_id_0:4,
		enable_member_id_0_check:1;
#endif
	/* data 5: */
	uchar	group_id_1;	/* Group ID 1 (1-based)
				   00h = unspecified
				   FFh = all groups */
	/* data 6: */
#ifdef BF_MS_FIRST
	uchar	:3,		/* Member ID 1 (0-based)
				   [7:5] - reserved */
		enable_member_id_1_check:1,	/* [4] - 0b = enable member ID check.
						   1b = disable member ID check. */
		member_id_1:4;	/* [3:0] - member ID. ID of this chassis within
				   specified group. (value is ignored if Group
				   ID 1 = FFh) */
#else
	uchar	member_id_1:4,
		enable_member_id_1_check:1,
		:3;
#endif
	/* data 7: */
	uchar	group_id_2;	/* Group ID 2 (1-based) 
				   00h = unspecified
				   FFh = all groups */
	/* data 8: */
#ifdef BF_MS_FIRST
	uchar	:3,		/* Member ID 2 (0-based) 
				   [7:5] - reserved */
		enable_member_id_2_check:1,	/* [4] - 0b = enable member ID check.
						   1b = disable member ID check. */
		member_id_2:4;	/* [3:0] - member ID. ID of this chassis within
				   specified group. (value is ignored if Group
				   ID 2 = FFh) */
#else
	uchar	member_id_2:4,
		enable_member_id_2_check:1,
		:3;
#endif
	/* data 9: */
	uchar	group_id_3;	/* Group ID 3 (1-based)
				   00h = unspecified
				   FFh = all groups */
	/* data 10: */
#ifdef BF_MS_FIRST
	uchar	:3,		/* Member ID 3 (0-based)
				   [7:5] - reserved */
		enable_member_id_3_check:1,	/* [4] - 0b = enable member ID check.
						   1b = disable member ID check. */
		member_id_3:4;	/* [3:0] - member ID. ID of this chassis within
				   specified group. (value is ignored if Group
				   ID 3 = FFh) */
#else
	uchar	member_id_3:4,
		enable_member_id_3_check:1,
		:3;
#endif
	/* data 11: - Retries and Operation */
#ifdef BF_MS_FIRST
	uchar	:1,		/* [7] - reserved */
		num_retries:3,	/* [6:4] - number of times to retry sending the
				   command to perform the group operation [For
				   ICMB, the BMC broadcasts a Group Chassis 
				   Control command] (1-based) */
		operation:4;	/* [3:0] - operation

	0h = power down. Force system into soft off (S4/S45) state.
	This is for ‘emergency’ management power down actions.
	The command does not initiate a clean shut-down of the
	operating system prior to powering down the system.

	1h = power up.

	2h = power cycle (optional). This command provides a power off
	interval of at least 1 second.

	3h = hard reset. Some systems may accept this option even if
	the system is in a state (e.g. powered down) where resets
	are unavailable.

	4h = pulse Diagnostic Interrupt. (optional) Pulse a version of a
	diagnostic interrupt that goes directly to the processor(s).
	This is typically used to cause the operating system to do
	a diagnostic dump (OS dependent). The interrupt is
	commonly an NMI on IA-32 systems and an INIT on Intel®
	Itanium™ processor based systems.

	5h = Initiate a soft-shutdown of OS via ACPI by emulating a
	fatal overtemperature. (optional) */
#else
	uchar	operation:4,
		num_retries:3,
		:1;
#endif
} PEF_GROUP_CONTROL_TABLE;

/*
OEM Parameters
(optional. Non-volatile or volatile as specified by OEM)
96:127 This range is available for special OEM configuration parameters. The
OEM is identified according to the Manufacturer ID field returned by the
Get Device ID command.
*/

/*
NOTE:
1. The enable/disable member ID check bit controls whether a control request
for the group is checked against the enabled members or not. If Member ID Check
is disabled, then a control request to the group will automatically be ‘logged’
for that group. Note, however, that the requested control state must match for
all enabled groups in order for it to take effect.
*/

/*----------------------------------------------------------------------*/
/*			Set Last Processed Event ID command		*/
/*----------------------------------------------------------------------*/
typedef struct	set_last_processed_event_id_cmd_req {
	uchar	command;
#ifdef BF_MS_FIRST
	uchar	:7,		/* [7:1] - reserved. */
		record_id:1;	/* [0] - 0b = set Record ID for last record 
				   processed by software.
				   1b = set Record ID for last record processed by BMC. */
#else
	uchar	record_id:1,
		:7;
#endif
	uchar	rec_id_lsb;
	uchar	rec_id_msb;	/* 2:3 Record ID. LS-byte first. */
} SET_LAST_PROCESSED_EVENT_ID_CMD_REQ;

typedef struct	set_last_processed_event_id_cmd_resp {
	uchar	completion_code;	/* Completion Code
					   81h = cannot execute command, 
					   SEL erase in progress */
} SET_LAST_PROCESSED_EVENT_ID_CMD_RESP;

/*----------------------------------------------------------------------*/
/*			Get Last Processed Event ID command		*/
/*----------------------------------------------------------------------*/
typedef struct get_last_processed_event_id_cmd_resp {
	uchar	completion_code;	/* Completion Code
					   81h = cannot execute command, 
					   SEL erase in progress */
	uchar	most_recent_timestamp[4]; /* 2:5 Most recent addition timestamp. LS byte first. */
	uchar	record_id[2];		/* 6:7 Record ID for last record in SEL. 
					   Returns FFFFh if SEL is empty. */
	uchar	last_sw_proc_evt_rec_id[2];	/* 8:9 Last SW Processed Event Record ID. */
	uchar	last_bmc_proc_evt_rec_id[2];	/* 10:11 Last BMC Processed Event Record ID. 
						   Returns 0000h when event has been
						   processed but could not be logged 
						   because the SEL is full or logging 
						   has been disabled. */
} GET_LAST_PROCESSED_EVENT_ID_CMD_RESP;
 
typedef struct events_processed {
	short last_sw_proc_evt_rec_id;
	short last_bmc_proc_evt_rec_id;
	short last_evt_rec_id;
	unsigned last_evt_rec_timestamp;
} EVENTS_PROCESSED;

/*======================================================================*/
/*
 *   Sensor Device Commands
 *
 	Command				NetFn 	CMD 	BMC 	SHM 	IPMC
	--------------------------------------------------------------------
	Get Device SDR Info 		S/E 	20h 	O 	M 	M
	Get Device SDR 			S/E 	21h 	O 	M 	M
	Reserve Device SDR Repository 	S/E 	22h 	O 	M 	M
	Get Sensor Reading Factors 	S/E 	23h 	O 	O 	O
	Set Sensor Hysteresis 		S/E 	24h 	O 	O 	O
	Get Sensor Hysteresis 		S/E 	25h 	O 	O 	O
	Set Sensor Threshold 		S/E 	26h 	O 	O 	O
	Get Sensor Threshold 		S/E 	27h 	O 	O 	O
	Set Sensor Event Enable 	S/E 	28h 	O 	O 	O
	Get Sensor Event Enable 	S/E 	29h 	O 	O 	O
	Re-arm Sensor Events 		S/E 	2Ah 	O 	O 	O
	Get Sensor Event Status 	S/E 	2Bh 	O 	O 	O
	Get Sensor Reading 		S/E 	2Dh 	M 	M 	M
	Set Sensor Type 		S/E 	2Eh 	O 	O 	O
	Get Sensor Type 		S/E 	2Fh 	O 	O 	O
 * 
 *  Using NETFN_EVENT_REQ/NETFN_EVENT_RESP
 */
/*======================================================================*/

#define IPMI_SE_CMD_GET_DEVICE_SDR_INFO		0x20	/* Get Device SDR Info */
#define IPMI_SE_CMD_GET_DEVICE_SDR		0x21	/* Get Device SDR */
#define IPMI_SE_CMD_RSV_DEVICE_SDR_REPOSITORY	0x22	/* Reserve Device SDR Repository */
#define IPMI_SE_CMD_GET_SENSOR_READING_FACTORS	0x23	/* Get Sensor Reading Factors */
#define IPMI_SE_CMD_SET_SENSOR_HYSTERESIS	0x24	/* Set Sensor Hysteresis */
#define IPMI_SE_CMD_GET_SENSOR_HYSTERESIS	0x25	/* Get Sensor Hysteresis */
#define IPMI_SE_CMD_SET_SENSOR_THRESHOLD	0x26	/* Set Sensor Threshold */
#define IPMI_SE_CMD_GET_SENSOR_THRESHOLD	0x27	/* Get Sensor Threshold */
#define IPMI_SE_CMD_SET_SENSOR_EVENT_ENABLE	0x28	/* Set Sensor Event Enable */
#define IPMI_SE_CMD_GET_SENSOR_EVENT_ENABLE	0x29	/* Get Sensor Event Enable */
#define IPMI_SE_CMD_REARM_SENSOR_EVENTS		0x2A	/* Re-arm Sensor Events */
#define IPMI_SE_CMD_GET_SENSOR_EVENT_STATUS	0x2B	/* Get Sensor Event Status */
#define IPMI_SE_CMD_GET_SENSOR_READING		0x2D	/* Get Sensor Reading */
#define IPMI_SE_CMD_SET_SENSOR_TYPE		0x2E	/* Set Sensor Type */
#define IPMI_SE_CMD_GET_SENSOR_TYPE		0x2F	/* Get Sensor Type */

#define EVENT_CMD_GET_DEVICE_SDR_INFO		0x0420
#define EVENT_CMD_GET_DEVICE_SDR		0x0421
/*----------------------------------------------------------------------*/
/*			Get Device SDR Info command			*/
/*----------------------------------------------------------------------*/
typedef struct get_device_sdr_info_cmd {
	uchar	command;
	uchar operation;	/* Operation (optional)
				   [7:1] - reserved
				   [0] - 1b = Get SDR count. This returns 
				   the total number of SDRs in the device.
				   0b = Get Sensor count. This returns the 
				   number of sensors implemented on LUN this
				   command was addressed to */
} GET_DEVICE_SDR_INFO_CMD;

typedef struct get_device_sdr_info_resp {
	uchar	completion_code;/* Completion Code */
	uchar	num;		/* For Operation = “Get Sensor Count” (or if 
				   byte 1 not present in request):
				   - Number of sensors in device for LUN this 
				   command was addressed to.
				   For Operation = “Get SDR Count”:
				   - Total Number of SDRs in the device. */
#ifdef BF_MS_FIRST
	uchar	flags:1,	/* Flags:
				   Dynamic population
				   [7] - 0b = static sensor population. 
				   The number of sensors handled by this
				   device is fixed, and a query shall return 
				   records for all sensors.
				   1b = dynamic sensor population. This device
				   may have its sensor population vary during 
				   ‘run time’ (defined as any time other that
				   when an install operation is in progress). */
		:3,		/* [6:4] - reserved */
		device_luns:4;	/* Device LUNs
				   [3] - 1b = LUN 3 has sensors
				   [2] - 1b = LUN 2 has sensors
				   [1] - 1b = LUN 1 has sensors
				   [0] - 1b = LUN 0 has sensors */
#else
	uchar	device_luns:4,
		:3,
		flags:1;
#endif
//	uchar sensor_population_change_indicator[4];	
				/* 4:7 Sensor Population Change Indicator. 
				   LS byte first.
				   Four byte timestamp, or counter. Updated or
				   incremented each time the sensor population 
				   changes. This field is not provided if the 
				   flags indicate a static sensor population.*/
} GET_DEVICE_SDR_INFO_RESP;

/*----------------------------------------------------------------------*/
/*			Get Device SDR command				*/
/*----------------------------------------------------------------------*/
typedef struct get_device_sdr_cmd {
	uchar command;
	uchar reservation_id_lsb; /* Reservation ID. LS Byte. Only required
				   for partial reads with a non-zero
				   ‘Offset into record’ field. Use 0000h
				   for reservation ID otherwise. */
	uchar reservation_id_msb;	/* Reservation ID. MS Byte. */
	uchar record_id_lsb;	/* Record ID of record to Get, LS Byte. 
				   0000h returns the first record. */
	uchar record_id_msb;	/* Record ID of record to Get, MS Byte */
	uchar offset;		/* Offset into record */
	uchar bytes_to_read;	/* Bytes to read. FFh means read entire record. */
} GET_DEVICE_SDR_CMD;

#define MAX_SDR_BYTES	20
typedef struct get_device_sdr_resp {
	uchar completion_code;	/* Completion Code. Generic, plus following
				   command specific:
				   80h = record changed. This status is 
				   returned if any of the record contents
				   have been altered since the last time the
				   Requester issued the request with 00h for
				   the ‘Offset into SDR’ field. */
	uchar rec_id_next_lsb;	/* Record ID for next record, LS Byte */
	uchar rec_id_next_msb;	/* Record ID for next record, MS Byte */
	uchar req_bytes[MAX_SDR_BYTES];	/* 4:3+N Requested bytes from record */
} GET_DEVICE_SDR_RESP;

/*----------------------------------------------------------------------*/
/*			Reserve Device SDR Repository command		*/
/*----------------------------------------------------------------------*/
typedef struct reserve_device_sdr_repository_resp {
	uchar completion_code;	/* Completion Code */
	uchar reservation_id_lsb;	/* Reservation ID, LS Byte 0000h reserved. */
	uchar reservation_id_msb;	/* Reservation ID, MS Byte */
} RESERVE_DEVICE_SDR_REPOSITORY_RESP;

/*----------------------------------------------------------------------*/
/*			Get Sensor Reading command			*/
/*----------------------------------------------------------------------*/

/* System Software use of Sensor Scanning bits & Entity Info
System software must ignore any sensor that has the sensor scanning bit 
disabled - if system software didn’t disable the sensor. This provides an 
alternate mechanism to allow the management controller to automatically
adjust the sensor population without requiring a corresponding change of the
sensor data records. For example, suppose the management controller has a way
of automatically knowing that a particular temperature sensor will be absent in
a given system configuration if a given processor is also absent. The management
controller could elect to automatically disable scanning for that temperature 
sensor. System management software would ignore that sensor even if it was 
reported in the SDRs.

Note that this is an alternate mechanism that may be useful in some circumstances.
The primary mechanism is to use the Entity ID information in the SDRs, and combine
that information with presence detection for the entity.

If there is a presence detection sensor for a given entity, then system management
software should ignore all other sensors associated with that entity. Some sensors
have intrinsic support for this. For example, a sensor-specific Processor sensor 
has a ‘Processor Presence’ bit. If that bit is implemented, and the processor is
absent, any other sensors and non-presence related bits associated with that 
processor can be ignored. If the sensor type doesn’t have an intrinsic presence
capability, you can implement an ‘Entity Presence’ sensor. This sensor solely 
reports whether a given Entity is present or not.

*/

typedef struct get_sensor_reading_cmd_req {
	uchar	command;
	uchar	sensor_number;
} GET_SENSOR_READING_CMD_REQ;

typedef struct get_sensor_reading_resp {
	uchar	completion_code;
	uchar	sensor_reading;	 /* byte of reading. Ignore on read if sensor
				    does not return an numeric (analog) reading */
#ifdef BF_MS_FIRST
	uchar	event_messages_enabled:1,	/* 0b = All Event Messages disabled from this sensor */
		sensor_scanning_enabled:1,	/* 0b = sensor scanning disabled */
		unavailable:1,			/* 1b = reading/state unavailable
						   (formerly “initial update in progress”).
This bit is set to indicate that a ‘re-arm’ or ‘Set Event Receiver’
command has been used to request an update of the sensor
status, and that update has not occurred yet. Software should
use this bit to avoid getting an incorrect status while the first
sensor update is in progress. This bit is only required if it is
possible for the controller to receive and process a ‘Get Sensor
Reading’ or ‘Get Sensor Event Status’ command for the sensor
before the update has completed. This is most likely to be the
case for sensors, such as fan RPM sensors, that may require
seconds to accumulate the first reading after a re-arm. The bit
is also used to indicate when a reading/state is unavailable
because the management controller cannot obtain a valid
reading or state for the monitored entity, typically because the
entity is not present. See Section 16.4, Event Status, Event
Conditions, and Present State and Section 16.6, Re-arming for
more information. */
		:5;	/* reserved */
#else
	uchar	:5,
		unavailable:1,
		sensor_scanning_enabled:1,
		event_messages_enabled:1;
#endif		
/* These bytes are optional:
(Byte 4) For threshold-based sensors
Present threshold comparison status
[7:6] - reserved. Returned as 1b. Ignore on read.
[5] - 1b = at or above (=) upper non-recoverable threshold
[4] - 1b = at or above (=) upper critical threshold
[3] - 1b = at or above (=) upper non-critical threshold
[2] - 1b = at or below (=) lower non-recoverable threshold
[1] - 1b = at or below (=) lower critical threshold
[0] - 1b = at or below (=) lower non-critical threshold
For discrete reading sensors
[7] - 1b = state 7 asserted
[6] - 1b = state 6 asserted
[5] - 1b = state 5 asserted
[4] - 1b = state 4 asserted
[3] - 1b = state 3 asserted
[2] - 1b = state 2 asserted
[1] - 1b = state 1 asserted
[0] - 1b = state 0 asserted
(Byte 5) For discrete reading sensors only. (Optional)
(00h Otherwise)
[7] - reserved. Returned as 1b. Ignore on read.
[6] - 1b = state 14 asserted
[5] - 1b = state 13 asserted
[4] - 1b = state 12 asserted
[3] - 1b = state 11 asserted
[2] - 1b = state 10 asserted
[1] - 1b = state 9 asserted
[0] - 1b = state 8 asserted
*/	    
} GET_SENSOR_READING_RESP;

/*----------------------------------------------------------------------*/
/*			Get Sensor Reading Factors command		*/
/*----------------------------------------------------------------------*/

typedef struct get_sensor_reading_factors_cmd {
	uchar	command;
	uchar sensor_number;	/* sensor number (FFh = reserved) */
	uchar reading_byte;	/* reading byte */
} GET_SENSOR_READING_FACTORS_CMD;

typedef struct get_sensor_reading_factors_resp {
	uchar completion_code;	/* Completion Code */
	uchar next_reading;	/* Next reading. This field indicates the 
				   next reading for which a different set of
sensor reading factors is defined. If the reading byte passed in the request
does not match exactly to a table entry, the nearest entry will be returned, and
this field will hold the reading byte value for which an exact table match would
have been obtained. Once the ‘exact’ table byte has been obtained, this field
will be returned with a value such that, if the returned value is used as the
reading byte for the next request, the process can be repeated to cycle
through all the Sensor Reading Factors in the device’s internal table. This
process shall ‘wrap around’ such a complete list of the table values can be
obtained starting with any reading byte value. */
	uchar	M_lsb;		/* M LS 8 bits */
#ifdef BF_MS_FIRST
	uchar	M_msb:2,	/* [7:6] - M: MS 2 bits */
		tolerance:6;	/* [5:0] - Tolerance in +/- 1/2 raw counts */
#else
	uchar	tolerance:6,
		M_msb:2;
#endif
	uchar	B_lsb;		/* [7:0] - B: LS 8 bits */
#ifdef BF_MS_FIRST
	uchar	B_msb:2,	/* [7:6] - B: MS 2 bits 
				   Unsigned, 10-bit Basic Sensor Accuracy
				   in 1/100 percent scaled up by
				   unsigned Accuracy exponent. */
		accuracy_lsb:6;	/* [5:0] - Accuracy: LS 6 bits */
#else
	uchar	accuracy_lsb:6,
		B_msb:2;
#endif
#ifdef BF_MS_FIRST
	uchar	accuracy_msb:4,	/* [7:4] - Accuracy: MS 4 bits */
	     	accuracy_exp:2,	/* [3:2] - Accuracy exp: 2 bits, unsigned */
		:2;		/* [1:0] - reserved: 2 bits, returned as 00b */
#else
	uchar	:2,
		accuracy_exp:2,
		accuracy_msb:4;
#endif
#ifdef BF_MS_FIRST
	uchar	R_exponent:4,	/* [7:4] - R (result) exponent 4 bits, signed */
		B_exponent:4;	/* [3:0] - B exponent 4 bits, signed */
#else
	uchar	B_exponent:4,
		R_exponent:4;
#endif
} GET_SENSOR_READING_FACTORS_RESP;


/*======================================================================*/
/*
 *    FRU Device Commands
 *
 	Command				NetFn 	CMD 	BMC 	SHM 	IPMC
	--------------------------------------------------------------------
	Get FRU Inventory Area Info 	Storage 10h 	M 	M 	M
	Read FRU Data 			Storage 11h 	M 	M 	M
	Write FRU Data 			Storage 12h 	M 	M 	M
 *    	
 *    Using NETFN_NVSTORE_REQ/NETFN_NVSTORE_RESP
 */
/*======================================================================*/
/* 
This section based on - IPMI - Platform Management FRU Information
Storage Definition document v1.0
Document Revision 1.1
September 27, 1999

The FRU Information is used to primarily to provide ‘inventory’ information
about the boards that the FRU Information Device is located on. It is a goal 
that all major field replaceable units (FRUs) have an EEPROM or FRU Information 
Device from which, at a minimum, the part number or version number can be read
through software. It is also a goal that the FRUs serial number be software 
readable.

The format divides the FRU Information Device (or EEPROM) into six different 
areas:

1) Common Header
The Common Header is mandatory for all FRU Information Device implementations. 
It holds version information for the overall information format specification 
and offsets to the other information areas. The other areas may or may not be 
present based on the application of the device. An area is specified as ‘Null’
or ‘not present’ when the Common Header has a value of 00h for the starting 
offset for that area.

2) Internal Use Area
This area provides private, implementation-specific information storage for
other devices that exist on the same FRU as the FRU Information Device. The
Internal Use Area is usually used to provide private non-volatile storage for a
management controller.

3) Chassis Info Area 
This area is used to hold Serial Number, Part Number, and other information
about the system chassis. A system can have multiple FRU Information Devices
within a chassis, but only one device should provide the Chassis Info Area.
Thus, this area will typically be absent from most FRU Information Devices.
Ideally this information is in a FRU device that is part of a board that is
associated with the chassis, such as a front panel or power distribution board.
But many systems, particularly low-end systems, do not incorporate such
locations. Therefore, it is common to find the Chassis Info Area included in the
FRU information for the baseboard.

4) Board Info Area 
This area provides Serial Number, Part Number, and other information about
the board that the FRU Information Device is located on.

5) Product Info Area
This area is present if the FRU itself is a separate product. This is typically
seen when the FRU is an add-in card. When this area is provided in the FRU
Information Device that contains the Chassis Info Area, the product info is for
the overall system, as initially manufactured.

5) MultiRecord Info Area
The MultiRecord Info Area provides a region that holds one or more records
where the type and format of the information is specified in the individual
headers for the records. This differs from the other information areas, where
the type and format of the information are implied by which offset is used in
the Common Header. The MultiRecord Info Area provides a mechanism for
extending the FRU Information Specification to cover new information types
without impacting the existing area definitions.

The management controller provides Read/Write FRU Inventory Data commands for
accessing the nonvolatile storage contents via the controller’s messaging 
interface.
*/
/*
The Common Header area is the starting point for accessing FRU information data. 
The Common Header area always starts at offset 00h. The Common Header is present
for all FRU Information Devices in the system. The data in this header provides
the offsets to the other information areas in the device. A checksum is included
to allow the integrity of the header data to be verified.

The Chassis Info, Board Info, and Product Info areas each contain a number of 
variable length fields. Each of these fields is preceded by a type/length byte 
that indicates the length of the field and the type of encoding that is used for
the field. The leading fields in each area serve predefined functions. These 
fields can be followed by ‘custom’ fields that are defined by manufacturing or
by the OEM. The same variable length field format is used within records in the
MultiRecord Area.

An application that accesses the information starts by verifying that the format
version information for the FRU Information Device is a version that the 
application supports. It does this be extracting the format version information 
from Common Header area. Next, the application extracts the starting offset for 
the desired area from the Common Header. The application then accesses the 
‘header’ information specified at the beginning of the area. If the area format 
version is correct, the application can proceed an access the data. (Note for 
the MultiRecord area, the Common Header offset points both the start of the area
and to the first Record in the MultiRecord area.)

Since the fields within an area (or Record) can be variable or fixed length, 
the application must ‘walk’ the fields sequentially. The application traverses 
all fields by walking individual fields until the ‘end-of-fields’ type/length 
byte (value C1h) is encountered. 
(Note: Only pre-defined fields are allowed to be fixed length without type/length
bytes. All custom fields are specified as variable length fields with type/length 
bytes.)

Each area/record starts with a fixed number of pre-defined fields. This is 
followed by a variable number of optional custom fields. If a field is not used,
a ‘NULL’ or ‘Empty’ version of the field is used as a placeholder. Thus, an 
application can rely on the Nth field as always having the same meaning. (Note:
this is guaranteed for pre-defined fields only.)

An application could present predefined fields with labels according to their 
function and formatted according to the ‘type’ specified in the type/length field.
The application could present custom fields with a generic label (e.g. ‘Custom 
Field 1’) followed by the field data formatted according to the ‘type’ specified
in the type/length field (e.g. Hex, ASCII, Binary).

Prior to using any FRU Information Data, an applications should validate the
checksum for the area or record.
*/

typedef struct fru_cache {
	uchar	fru_dev_id;
	int	fru_inventory_area_size;
	uchar	*fru_data;
} FRU_CACHE;


typedef struct fru_common_header {
#ifdef BF_MS_FIRST
	uchar	:4,			/* Common Header Format Version
					   7:4 - reserved, write as 0000b */
		format_version:4;	/* 3:0 - format version number = 1h 
					   for this specification. */
#else
	uchar	format_version:4,
		:4;
#endif
	uchar	int_use_offset;		/* Internal Use Area Starting Offset
					   (in multiples of 8 bytes). 00h
					   indicates that this area is not 
					   present. */
	uchar	chassis_info_offset;	/* Chassis Info Area Starting
					   Offset (in multiples of 8 bytes). 00h
					   indicates that this area is not 
					   present. */
	uchar	board_offset;		/* Board Area Starting Offset (in 
					   multiples of 8 bytes). 00h indicates
					   that this area is not present. */
	uchar	product_info_offset;	/* Product Info Area Starting
					   Offset (in multiples of 8 bytes).
					   00h indicates that this area is not 
					   present. */
	uchar	multirecord_offset;	/* MultiRecord Area Starting Offset
					   (in multiples of 8 bytes). 00h
					   indicates that this area is not
					   present. */
	uchar	pad;			/* PAD, write as 00h */
	uchar	checksum;		/* Common Header Checksum (zero checksum) */
} FRU_COMMON_HEADER;


typedef struct multirecord_area_header {
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
	uchar	picmg_rec_id;	/* PICMG Record ID. */
	uchar	rec_fmt_ver;	/* Record Format Version. For this specification,
				   the value 0h shall be used. */
} MULTIRECORD_AREA_HEADER;

	
typedef struct fru_internal_use_area {
#ifdef BF_MS_FIRST
	uchar	:4,			/* Internal Use Format Version
					   7:4 - reserved, write as 0000b */
		format_version:4;	/* 3:0 - format version number = 1h 
					   for this specification. */
#else
	uchar	format_version:4,
		:4;
#endif
	uchar	data;			/* Internal use data - One or more bytes
					   defined and formatted as needed for
					   management controller or other device
					   that owns/uses this area. */
} FRU_INTERNAL_USE_AREA;

typedef struct fru_chassis_info_area_hdr {
#ifdef BF_MS_FIRST
	uchar	:4,			/* Chassis Info Area Format Version
					   7:4 - reserved, write as 0000b */
		format_version:4;	/* 3:0 - format version number = 1h 
					   for this specification. */
#else
	uchar	format_version:4,
		:4;
#endif

	uchar	len;			/* Chassis Info Area Length (in multiples
					   of 8 bytes) */
	uchar	type;			/* Chassis Type (enumeration) */
} FRU_CHASSIS_INFO_AREA_HDR;

/* The rest is variable length data
	uchar	part_num_type:4,	- Chassis Part Number type/length 
		part_num_len;
	uchar	part_num[N];		- Chassis Part Number bytes 
	uchar	serial_num_type:4,	- Chassis Serial Number type/length
		serial_num_len;
	uchar	serial_num[M];		- Chassis Serial Number bytes 

	uchar	custom_data[xx]		- Custom Chassis Info fields, if any. 
					  Each field must be preceeded
					  with type/length byte.
		C1h 			- (type/length byte encoded to indicate no more info fields).
		00h 			- any remaining unused space
	uchar	checksum		- Chassis Info Checksum (zero checksum)
*/

typedef struct board_area_format_hdr {
#ifdef BF_MS_FIRST
	uchar	:4,			/* Board Area Format Version
					   7:4 - reserved, write as 0000b */
		format_version:4;		/* 3:0 - format version number = 1h 
					   for this specification. */
#else
	uchar	format_version:4,
		:4;
#endif
	uchar	len;			/* Board Area Length (in multiples
					   of 8 bytes) */
	uchar	lang_code;		/* Language Code */
	uchar	mfg_time[3];		/*  Mfg. Date / Time
					    Number of minutes from 0:00 hrs 1/1/96.
					    LSbyte first (little endian) */
} BOARD_AREA_FORMAT_HDR;

/* The rest is variable length data
	uchar	manuf_type:4,		- Board Manufacturer type/length byte
		manuf_len;
	uchar	manuf[P];		- Board Manufacturer bytes
	uchar	prod_name_type:4,	- Board Product Name type/length byte
		prod_name_len;
	uchar	prod_name[Q];		- Board Product Name bytes
	uchar	ser_num_type:4		- Serial Number type/length byte
		ser_num_len;
	uchar	ser_num[N]		- Board Serial Number bytes
	uchar	part_num_type:4		- Board Part Number type/length byte
		part_num_len;
	uchar	part_num[M];		- Board Part Number bytes
	uchar	fru_file_id_type:4,	- FRU File ID type/length byte
		fru_file_id_len;
	uchar	fru_file_id[R];		- FRU File ID bytes 

The FRU File version field is a pre-defined field
provided as a manufacturing aid for verifying the file that was used
during manufacture or field update to load the FRU information. The
content is manufacturer-specific. This field is also provided in the
Product Info area. Either or both fields may be ‘null’.

	uchar	custom_data[xx];	- Additional custom Mfg. Info fields. 
					  Defined by manufacturing. Each
					  field must be preceded by a type/length byte
		C1h; 			- (type/length byte encoded to indicate no more info fields).
	uchar	unused[Y];		- 00h - any remaining unused space
	uchar	checksum;		- Board Area Checksum (zero checksum)
*/

typedef struct product_area_format_hdr {
#ifdef BF_MS_FIRST
	uchar	:4,			/* Product Area Format Version
					   7:4 - reserved, write as 0000b */
		format_version:4;		/* 3:0 - format version number = 1h 
					   for this specification. */
#else
	uchar	format_version:4,
		:4;
#endif
	uchar	len;			/* Product Area Length (in multiples
					   of 8 bytes) */
	uchar	lang_code;		/* Language Code */
} PRODUCT_AREA_FORMAT_HDR;

/* The rest is variable length data
	uchar	manuf_name_type:4,	- Manufacturer Name type/length byte
		manuf_name-len;	
	uchar	manuf_name[N];		- Manufacturer Name bytes
	uchar	prod_name_type:4,	- Product Name type/length byte
	uchar	prod_name[M];		- Product Name bytes
	uchar	prod_part_model_num_type:4,	- Product Part/Model Number type/length byte
		prod_part_model_num_len;
	uchar	prod_part_model[O];	- Product Part/Model Number bytes
	uchar	prod_version_type:4,	- Product Version type/length byte
		prod_version_len;
	uchar	prod_version[R];	- Product Version bytes
	uchar	prod_serial_num_type:4,	- Product Serial Number type/length byte
		prod_serial_num_len;
	uchar	prod_serial_num[P];	- Product Serial Number bytes
	uchar	asset_tag_type:4,	- Asset Tag type/length byte
		asset_tag_len;
	uchar	asset_tag[Q];		- Asset Tag
	uchar	fru_file_id_type:4,	- FRU File ID type/length byte
		fru_file_id_len;
	uchar	fru_file_id[R];		- FRU File ID bytes. 

The FRU File version field is a pre-defined field
provided as a manufacturing aid for verifying the file that was used
during manufacture or field update to load the FRU information. The
content is manufacturer-specific. This field is also provided in the
Board Info area. Either or both fields may be ‘null’.

	uchar	custom_data[xx];	- Custom product info area fields, 
					  if any (must be preceded with
					  type/length byte)
		C1h;			- (type/length byte encoded to indicate
	       				  no more info fields).
	uchar	unused[Y];		- 00h - any remaining unused space
	uchar	checksum;		- Product Info Area Checksum (zero checksum)
*/	
#define IPMI_STO_CMD_GET_FRU_INVENTORY_AREA_INFO 0x10	/* Get FRU Inventory Area Info */
#define IPMI_STO_CMD_READ_FRU_DATA		0x11	/* Read FRU Data */
#define IPMI_STO_CMD_WRITE_FRU_DATA		0x12	/* Write FRU Data */

/* Combined "Netfn | Cmd" defines */
#define NVSTORE_CMD_GET_FRU_INVENTORY_AREA_INFO	0x0A10
#define NVSTORE_CMD_IPMI_STO_CMD_READ_FRU_DATA	0x0A11

/*----------------------------------------------------------------------*/
/*			Get FRU Inventory Area Info command		*/
/*----------------------------------------------------------------------*/

typedef struct get_fru_inventory_area_info_cmd_req {
	uchar	command;
	uchar fru_dev_id;	/* FRU Device ID. FFh = reserved. */
} GET_FRU_INVENTORY_AREA_INFO_CMD_REQ;

typedef struct get_fru_inventory_area_info_cmd_resp {
	uchar completion_code;	/* Completion Code */
	uchar fru_inventory_area_size_lsb;	/* FRU Inventory area size in bytes, LS Byte */
	uchar fru_inventory_area_size_msb;	/* FRU Inventory area size in bytes, MS Byte */
#ifdef BF_MS_FIRST
	uchar	:7,			/* [7:1] - reserved */
		access_method:1;		/* [0] 0b = Device is accessed by bytes, 
					   1b = Device is accessed by words */
#else
	uchar	access_method:1,
		:7;
#endif
} GET_FRU_INVENTORY_AREA_CMD_RESP;

/*----------------------------------------------------------------------*/
/*			Read FRU Data command				*/
/*----------------------------------------------------------------------*/

typedef struct read_fru_data_cmd_req {
	uchar	command;
	uchar fru_dev_id;	/* FRU Device ID. FFh = reserved. */
	uchar fru_inventory_offset_lsb;	/* FRU Inventory Offset to read, LS Byte */
	uchar fru_inventory_offset_msb;	/* FRU Inventory Offset to read, MS Byte 
					   Offset is in bytes or words per 
					   device access type returned in the
					   Get FRU Inventory Area Info command. */
	uchar count_to_read;	/*  Count to read --- count is ‘1’ based */
} READ_FRU_DATA_CMD_REQ;

typedef struct read_fru_data_cmd_resp {
	uchar completion_code;	/* Completion code. Generic, plus following
				   command specific:
				   81h = FRU device busy. The requested cannot
				   be completed because the implementation of 
				   the logical FRU device is in a state where
				   the FRU information is temporarily unavailable.
				   This could be due to a condition such as a
				   loss of arbitration if the FRU is implemented
				   as a device on a shared bus.
				   Software can elect to retry the operation
				   after at least 30 milliseconds if this code
				   is returned. Note that it is highly 
				   recommended that management controllers 
				   incorporate builtin retry mechanisms. 
				   Generic IPMI software cannot be relied
				   upon to take advantage of this completion code. */
	uchar count_returned;	/* Count returned --- count is ‘1’ based */
	uchar data[20];		/* 3:2+N Requested data - TODO check size */
} READ_FRU_DATA_CMD_RESP;

/*----------------------------------------------------------------------*/
/*			Write FRU Data command				*/
/*----------------------------------------------------------------------*/

typedef struct write_fru_data_cmd_req {
	uchar	command;
	uchar fru_dev_id;	/* FRU Device ID. FFh = reserved. */
	uchar fru_inventory_offset_lsb;	/* FRU Inventory Offset to write, LS Byte */
	uchar fru_inventory_offset_msb;	/* FRU Inventory Offset to write, MS Byte */
	uchar data[20];		/* 4:3+N Data to write */
} WRITE_FRU_DATA_CMD_REQ;

typedef struct write_fru_data_cmd_resp {
	uchar completion_code;	/* Completion code. Generic, plus following
				   command specific:
				   80h = write-protected offset. Cannot 
				   complete write because one or more bytes of
				   FRU data are to a write-protected offset in
				   the FRU device. Note that an implementation
				   may have allowed a ‘partial write’ of the
				   data to occur.
				   81h = FRU device busy. Refer to the 
				   preceding table for the Read FRU Command 
				   for the description of this completion code. */
	uchar count_written;	/* Count written --- count is ‘1’ based */
} WRITE_FRU_DATA_CMD_RESP;


/*======================================================================*/
/*
 *    SDR Device Commands
 *
 	Command				NetFn 	CMD 	BMC 	SHM 	IPMC
	--------------------------------------------------------------------
	Get SDR Repository Info		Storage 20h 	M 	M 	M
	Get SDR Repository Allocation Info Storage 21h 	O 	O 	O
	Reserve SDR Repository		Storage 22h 	M 	M 	M
	Get SDR				Storage 23h 	M 	M 	M
	Add SDR				Storage 24h 	M 	M 	O/M
	Partial Add SDR 		Storage 25h 	M 	M 	O/M
	Delete SDR			Storage 26h 	O 	O 	O
	Clear SDR Repository 		Storage 27h 	M 	M 	O/M
	Get SDR Repository Time 	Storage 28h 	O/M 	O/M 	O/M
	Set SDR Repository Time		Storage 29h 	O/M 	O/M 	O/M
	Enter SDR Repository Update Mode Storage 2Ah 	O 	O 	O
	Exit SDR Repository Update Mode Storage 2Bh 	M 	M 	M
	Run Initialization Agent 	Storage 2Ch 	O 	O 	O
 *    	
 *    Using NETFN_NVSTORE_REQ/NETFN_NVSTORE_RESP
 */
/*======================================================================*/

#define IPMI_STO_CMD_GET_SDR_REPOSITORY_INFO	0x20	/* Get SDR Repository Info */
#define IPMI_STO_CMD_GET_SDR_REPOSITORY_ALLOCATION_INFO	0x21	/* Get SDR Repository Allocation Info */
#define IPMI_STO_CMD_RESERVE_SDR_REPOSITORY	0x22	/* Reserve SDR Repository */
#define IPMI_STO_CMD_GET_SDR			0x23	/* Get SDR */
#define IPMI_STO_CMD_ADD_SDR			0x24	/* Add SDR */
#define IPMI_STO_CMD_PARTIAL_ADD_SDR		0x25	/* Partial Add SDR */
#define IPMI_STO_CMD_DELETE_SDR			0x26	/* Delete SDR */
#define IPMI_STO_CMD_CLEAR_SDR_REPOSITORY	0x27	/* Clear SDR Repository */
#define IPMI_STO_CMD_GET_SDR_REPOSITORY_TIME	0x28	/* Get SDR Repository Time */
#define IPMI_STO_CMD_SET_SDR_REPOSITORY_TIME	0x29	/* Set SDR Repository Time */
#define IPMI_STO_CMD_ENTER_SDR_REPOSITORY_UPDATE_MODE	0x2A	/* Enter SDR Repository Update Mode */
#define IPMI_STO_CMD_EXIT_SDR_REPOSITORY_UPDATE_MODE	0x2B	/* Exit SDR Repository Update Mode */
#define IPMI_STO_CMD_RUN_INITIALIZATION_AGENT	0x2C	/* Run Initialization Agent */

/*----------------------------------------------------------------------*/
/*			Get SDR Repository Info command			*/
/*----------------------------------------------------------------------*/

typedef struct get_sdr_repository_info_cmd_resp {
	uchar completion_code;	/* Completion Code */
	uchar sdr_version;	/* SDR Version - version number of the SDR
				   command set for the SDR Device.
				   51h for this specification. (BCD encoded 
				   with bits 7:4 holding the Least Significant
				   digit of the revision and bits 3:0 holding
				   the Most Significant bits.) */
	uchar record_count_lsb;	/* Record count LS Byte - number of records 
				   in the SDR Repository */
	uchar record_count_msb;	/* Record count MS Byte - number of records
				   in the SDR Repository */
	uchar free_space_lsb;	/* 5:6 Free Space in bytes, LS Byte first. 
				   0000h indicates ‘full’, FFFEh indicates
				   64KB-2 or more available. FFFFh indicates
				   ‘unspecified’. */
	uchar free_space_msb;
	uchar most_recent_addition_timestamp[4];	
	/* 7:10 Most recent addition timestamp. LS byte first. */
	uchar	most_recent_erase[4];	/* 11:14 Most recent erase (delete or 
					   clear) timestamp. LS byte first. */
	uchar operation_support;	/* Operation Support
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
} GET_SDR_REPOSITORY_INFO_CMD_RESP;
	
/*----------------------------------------------------------------------*/
/*			Reserve SDR Repository command			*/
/*----------------------------------------------------------------------*/

typedef struct reserve_sdr_repository_cmd_resp {
	uchar completion_code;	/* Completion Code */
	uchar reservation_id_lsb;	/* Reservation ID, LS Byte */
	uchar reservation_id_msb;	/* Reservation ID, MS Byte */
} RESERVE_SDR_REPOSITORY_CMD_RESP;

/*----------------------------------------------------------------------*/
/*			Get SDR command					*/
/*----------------------------------------------------------------------*/

typedef struct get_sdr_cmd_req {
	uchar	command;
	uchar reservation_id_lsb; /* Reservation ID. LS Byte. Only required for
				   partial reads with a nonzero ‘Offset into
				   record’ field. Use 0000h for reservation 
				   ID otherwise. */
	uchar reservation_id_msb; /* Reservation ID. MS Byte. */
	uchar record_id_lsb;	/* Record ID of record to Get, LS Byte */
	uchar record_id_msb;	/* Record ID of record to Get, MS Byte */
	uchar offset;		/* Offset into record */
	uchar bytes_to_read;	/* Bytes to read. FFh means read entire record. */
} GET_SDR_CMD_REQ;

typedef struct get_sdr_cmd_resp {
	uchar completion_code;	/* Completion Code */
	uchar record_id_next_lsb;	/* Record ID for next record, LS Byte */
	uchar record_id_next_msb;	/* Record ID for next record, MS Byte */
	uchar record_data[20];	/* 4:3+N Record Data */
} GET_SDR_CMD_RESP;

/*----------------------------------------------------------------------*/
/*			Exit SDR Repository Update Mode command		*/
/*----------------------------------------------------------------------*/
typedef struct exit_sdr_repository_update_mode_cmd_resp {
	uchar completion_code;	/* Completion Code */
} EXIT_SDR_REPOSITORY_UPDATE_MODE_CMD_RESP;


/*======================================================================*/
/*
 * 	Sensor Type Codes and Data
 * 	(Table 42-3) 
 * 
 */

#define ST_TEMPERATURE				0x01	// Temperature
#define ST_VOLTAGE				0x02	// Voltage
#define ST_CURRENT				0x03	// Current
#define ST_FAN					0x04	// Fan

/* Sensor Type: Physical Security */
#define ST_PHYSICAL_SECURITY			0x05	// Physical Security (Chassis Intrusion)
#define STO_PS_GENERAL_CHASSIS_INTRUSION	0x00	// General Chassis Intrusion
#define STO_PS_DRIVE_BAY_INTRUSION		0x01	// Drive Bay Instrusion
#define STO_PS_IO_CARD_AREA_INTRUSION		0x02	// I/O Card area intrusion
#define STO_PS_PROCESSOR_AREA_INTRUSION		0x03	// Processor area intrusion
#define STO_PS_LAN_LEASH_LOST			0x04	/* LAN Leash Lost (system 
							   is unplugged from LAN)
		The Event Data 2 field can be used to identify which network
		controller the leash was lost on where 00h corresponds to the
		first (or only) network controller. */
#define STO_PS_UNAUTHORIZED_DOCKING		0x05	// Unauthorized dock/undock
#define STO_PS_FAN_AREA_INTRUSION		0x06	/* FAN area intrusion 
		(supports detection of hot plug fan tampering) */

/* Sensor Type: Platform Security Violation Attempt */
#define ST_PLATFORM_SECURITY_VIOLATION_ATTEMPT	0x06
#define STO_PSVA_SECURE_MODE_VIAOLATION_ATTEMPT 0x00 	// Secure Mode (Front Panel Lockout) Violation attempt
#define STO_PSVA_PREBOOT_PW_VIOLATION_USER	0x01 	// Pre-boot Password Violation - user password
#define STO_PSVA_PREBOOT_PW_VIOLATION_SETUP	0x02 	// Pre-boot Password Violation attempt - setup password
#define STO_PSVA_PREBOOT_PW_VIOLATION_NW_BOOT	0x03 	// Pre-boot Password Violation - network boot password
#define STO_PSVA_PREBOOT_PW_VIOLATION_OTHER	0x04 	// Other pre-boot Password Violation
#define STO_PSVA_OOB_ACCESS_PW_VIOLATION	0x05 	// Out-of-band Access Password Violation

/* Sensor Type: Processor */
#define ST_PROCESSOR				0x07
#define STO_PROC_IERR				0x00	// IERR
#define STO_PROC_THERMAL_TRIP			0x01	// Thermal Trip
#define STO_PROC_FRB1_BIST_FAILURE		0x02	// FRB1/BIST failure
#define STO_PROC_FRB2_POST_FAILURE		0x03	/* FRB2/Hang in POST failure 
		(used hang is believed to be due or related to a processor 
		failure. Use System Firmware Progress sensor for other BIOS 
		hangs.) */
#define STO_PROC_FRB3_STARTUP_INIT_FAILURE	0x04	// FRB3/Processor Startup/Initialization failure (CPU didn’t start)
#define STO_PROC_CONFIG_ERROR			0x05	// Configuration Error
#define STO_PROC_SM_BIOS_UNCORRECTABLE_CPU_ERROR	0x06	// SM BIOS ‘Uncorrectable CPU-complex Error’
#define STO_PROC_PROCESSOR_PRESENCE_DETECTED	0x07	// Processor Presence detected
#define STO_PROC_PROCESSOR_DISABLED		0x08	// Processor disabled
#define STO_PROC_TERMINATOR_PRESENCE_DETECTED	0x09	// Terminator Presence Detected
#define STO_PROC_PROCESSOR_AUTOMATICALLY_THROTTLED	0x0A	
		/* Processor Automatically Throttled (processor throttling triggered
		 * by a hardware-based mechanism operating independent from
		 * system software, such as automatic thermal throttling or throttling
		 * to limit power consumption.) */

/* Sensor Type: Power Supply */
#define ST_POWER_SUPPLY				0x08	
		/* Power Supply (also used for power converters [e.g.
		 * DC-to-DC converters] and VRMs [voltage regulator modules]). */
#define STO_PSU_PRESENCE_DETECTED		0x00	// Presence detected
#define STO_PSU_FAILURE_DETECTED		0x01	// Power Supply Failure detected
#define STO_PSU_PREDICTIVE_FAILURE		0x02	// Predictive Failure
#define STO_PSU_INPUT_LOST			0x03	// Power Supply input lost (AC/DC)
#define STO_PSU_INPUT_LOST_OR_OUT_OF_RANGE	0x04	// Power Supply input lost or out-of-range
#define STO_PSU_INPUT_OUT_OF_RANGE		0x05	// Power Supply input out-of-range, but present
#define STO_PSU_CONFIG_ERROR			0x06	// Configuration error. 
	/* The Event Data 3 field provides a more detailed definition of the error:
	   7:4 = Reserved for future definition, set to 0000b
	   3:0 = Error Type, one of 
	   	0h = Vendor mismatch, for power supplies that include this
		status. (Typically, the system OEM defines the vendor
		compatibility criteria that drives this status).
		1h = Revision mismatch, for power supplies that include this
		status. (Typically, the system OEM defines the vendor
		revision compatibility that drives this status).
		2h = Processor missing. For processor power supplies
		(typically DC-to-DC converters or VRMs), there's usually
		a one-to-one relationship between the supply and the
		CPU. This offset can indicate the situation where the
		power supply is present but the processor is not. This
		offset can be used for reporting that as an unexpected
		or unsupported condition.
		Others = Reserved for future definition */

/* Sensor Type: Power Unit */
#define ST_POWER_UNIT				0x09	// Power Unit 

#define ST0_PU_POWER_OFF			0x00	// Power Off / Power Down
#define ST0_PU_POWER_CYCLE			0x01	// Power Cycle
#define ST0_PU_240VA_POWER_DOWN			0x02	// 240VA Power Down
#define ST0_PU_INTERLOCK_POWER_DOWN		0x03	// Interlock Power Down
#define ST0_PU_AC_LOST				0x04	// AC lost
#define ST0_PU_SOFT_POWER_CONTROL_FAILURE	0x05	// Soft Power Control Failure (unit did 
							// not respond to request to turn on)
#define ST0_PU_FAILURE_DETECTED			0x06	// Power Unit Failure detected
#define ST0_PU_PREDICTIVE_FAILURE		0x07	// Predictive Failure

/* Sensor Type: Cooling Device */
#define ST_COOLING_DEVICE			0x0A	// Cooling Device 

/* Sensor Type: Other units-based Sensor */
#define ST_OTHER_UNITS_BASED_SENSOR		0x0B	// Other Units-based Sensor (per units given in SDR)

/* Sensor Type: Memory */
#define ST_MEMORY				0x0C	// Memory 
#define STO_MEM_CORRECTABLE_ECC			0x00	// Correctable ECC / other correctable memory error
#define STO_MEM_UNCORRECTABLE_ECC		0x01	// Uncorrectable ECC / other uncorrectable memory error
#define STO_MEM_PARITY_ERROR			0x02	// Parity
#define STO_MEM_SCRUB_FAILED			0x03	// Memory Scrub Failed (stuck bit)
#define STO_MEM_DEVICE_DISABLED			0x04	// Memory Device Disabled
#define STO_MEM_CORRECTABLE_ERR_LIMIT		0x05	// Correctable ECC / other correctable memory error logging limit reached
#define STO_MEM_PRESENCE_DETECTED		0x06	
	/* Presence detected. Indicates presence of entity associated with
	 * the sensor. Typically the entity will be a ‘memory module’ or other
	 * entity representing a physically replaceable unit of memory. */
#define STO_MEM_CONFIG_ERROR			0x07	
	/* Configuration error. Indicates a memory configuration error for the
	 * entity associated with the sensor. This can include when a given
	 * implementation of the entity is not supported by the system (e.g.,
	 * when the particular size of the memory module is unsupported) or
	 * that the entity is part of an unsupported memory configuration
	 * (e.g. the configuration is not supported because the memory
	 * module doesn’t match other memory modules). */
#define STO_MEM_SPARE				0x08	
	/* Spare. Indicates entity associated with the sensor represents a
	 * ‘spare’ unit of memory.
	 * The Event Data 3 field can be used to provide an event extension
	 * code, with the following definition:
	 * Event Data 3
	 * [7:0] - Memory module/device (e.g. DIMM/SIMM/RIMM)
	 * identification, relative to the entity that the sensor is
	 * associated with (if SDR provided for this sensor). */

/* Sensor Type: Drive Slot (Bay) */
#define ST_DRIVE_SLOT				0x0D	// Drive Slot (Bay)

/* Sensor Type: POST MEMORY RESIZE */
#define ST_POST_MEMORY_RESIZE			0x0E	// POST Memory Resize

/* Sensor Type: System Firmware Progress */
#define ST_SYSTEM_FIRMWARE_PROGRESS		0x0F	// System Firmware Progress
	/* (formerly POST Error) */
#define STO_SFP_SYSTEM_FIRMWARE_ERROR		0x00 	// System Firmware Error (POST Error)
	/* The Event Data 2 field can be used to provide an event extension
	   code, with the following definition:
	   Event Data 2 */
#define SYS_FW_ERROR_UNSPECIFIED	   	0x00	// Unspecified.
#define SYS_FW_ERROR_NO_MEM	   		0x01	// No system memory is physically 
							// installed in the system.
#define SYS_FW_ERROR_NO_USABLE_MEM	   	0x02	// No usable system memory, all installed memory has
							// experienced an unrecoverable failure.
#define SYS_FW_ERROR_HD_FAILURE	   		0x03	// Unrecoverable hard-disk/ATAPI/IDE device failure.
#define SYS_FW_ERROR_SYS_BOARD_FAILURE	   	0x04	// Unrecoverable system-board failure.
#define SYS_FW_ERROR_FLOPPY_FAILURE	   	0x05	// Unrecoverable diskette subsystem failure.
#define SYS_FW_ERROR_HD_CTRL_FAILURE	   	0x06	// Unrecoverable hard-disk controller failure.
#define SYS_FW_ERROR_KBD_FAILURE	   	0x07	// Unrecoverable PS/2 or USB keyboard failure.
#define SYS_FW_ERROR_REM_BOOT_MEDIA_NOT_FOUND	0x08	// Removable boot media not found
#define SYS_FW_ERROR_VIDEO_CTRL_FAILURE		0x09	// Unrecoverable video controller failure
#define SYS_FW_ERROR_NO_VIDEO			0x0A	// No video device detected
#define SYS_FW_ERROR_BIOS_CORRUPTION		0x0B	// Firmware (BIOS) ROM corruption detected
#define SYS_FW_ERROR_CPU_VOLTAGE_MISMATCH	0x0C	// CPU voltage mismatch 
							// (processors that share same supply
							// have mismatched voltage requirements)
#define SYS_FW_ERROR_CPU_SPEED_MATCHING_FAILURE	0x0D	// CPU speed matching failure
							// 0Eh to FFh reserved
							
						
#define STO_SFP_SYSTEM_FIRMWARE_HANG		0x01	// System Firmware Hang 
	/* (uses same Event Data 2 definition as 
	   following System Firmware Progress offset) */
#define STO_SFP_SYSTEM_FIRMWARE_PROGRESS	0x02	// System Firmware Progress
	/* The Event Data 2 field can be used to provide an event extension
	   code, with the following definition:
	   Event Data 2: */
#define SYS_FW_PROGRESS_UNSPECIFIED		0x00	// Unspecified.
#define SYS_FW_PROGRESS_MEM_INIT		0x01	// Memory initialization.
#define SYS_FW_PROGRESS_HD_INIT	   		0x02	// Hard-disk initialization
#define SYS_FW_PROGRESS_SEC_PROC_INIT	   	0x03	// Secondary processor(s) initialization
#define SYS_FW_PROGRESS_USER_AUTH	   	0x04	// User authentication
#define SYS_FW_PROGRESS_USER_SYS_SETUP	   	0x05	// User-initiated system setup
#define SYS_FW_PROGRESS_RESOURCE_CONFIG	   	0x06	// USB resource configuration
#define SYS_FW_PROGRESS_PCI_RESOURCE_CONFIG	0x07	// PCI resource configuration
#define SYS_FW_PROGRESS_OPTION_ROM_INIT	   	0x08	// Option ROM initialization
#define SYS_FW_PROGRESS_VIDEO_INIT	   	0x09	// Video initialization
#define SYS_FW_PROGRESS_CACHE_INIT	   	0x0A	// Cache initialization
#define SYS_FW_PROGRESS_SM_BUS_INIT	   	0x0B	// SM Bus initialization
#define SYS_FW_PROGRESS_KBD_CNTRL_INIT	   	0x0C	// Keyboard controller initialization
#define SYS_FW_PROGRESS_BMC_INIT	   	0x0D	// Embedded controller/management 
							// controller initialization
#define SYS_FW_PROGRESS_DOCKING_INIT	   	0x0E	// Docking station attachment
#define SYS_FW_PROGRESS_ENABLING_DOCKING 	0x0F	// Enabling docking station
#define SYS_FW_PROGRESS_EJECTING_DOCKING	0x10	// Docking station ejection
#define SYS_FW_PROGRESS_DISABLING_DOCKING	0x11	// Disabling docking station
#define SYS_FW_PROGRESS_OS_WAKEUP_VECTOR_CALL	0x12	// Calling operating system wake-up vector
#define SYS_FW_PROGRESS_OS_BOOT	   		0x13	// Starting operating system boot process, 
							// e.g. calling Int 19h
#define SYS_FW_PROGRESS_MB_INIT	   		0x14	// Baseboard or motherboard initialization
#define SYS_FW_PROGRESS_RESERVED	   	0x15	// reserved
#define SYS_FW_PROGRESS_FLOPPY_INIT	   	0x16	// Floppy initialization
#define SYS_FW_PROGRESS_KBD_TEST	   	0x17	// Keyboard test
#define SYS_FW_PROGRESS_MOUSE_TEST	   	0x18	// Pointing device test
#define SYS_FW_PROGRESS_MAIN_PROC_INIT	   	0x19	// Primary processor initialization
							// 1Ah to FFh reserved


/* Sensor Type: Event Logging Disabled */
#define ST_EVENT_LOGGING_DISABLED		0x10	// Event Logging Disabled 
#define STO_ELD_CORR_MEM_ERR_LOG_DISABLED	0x00	// Correctable Memory Error Logging Disabled
	/* Event Data 2
	   [7:0] - Memory module/device (e.g. DIMM/SIMM/RIMM) identification, 
	   relative to the entity that the sensor is associated with (if SDR
	   provided for this sensor). */
#define STO_ELD_EVENT_TYPE_LOG_DISABLED		0x01	// Event ‘Type’ Logging Disabled. 
	/* Event Logging is disabled for following event/reading type and
	   offset has been disabled.
	   Event Data 2
	   Event/Reading Type Code
	   Event Data 3
	   [7:6] - reserved. Write as 00b.
	   [5] - 1b = logging has been disabled for all events of given type
	   [4] - 1b = assertion event, 0b = deassertion event
	   [3:0] - Event Offset */
#define STO_ELD_LOG_AREA_RESET_OR_CLEARED	0x02	// Log Area Reset/Cleared
#define STO_ELD_ALL_EVENT_LOG_DISABLED		0x03	// All Event Logging Disabled
#define STO_ELD_SEL_FULL			0x04	// SEL Full. 
	/* If this is used to generate an event, it is recommended
	   that this be generated so that this will be logged as the last entry
	   in the SEL. If the SEL is very small, an implementation can elect
	   to generate this event after the last entry has been placed in the
	   SEL to save space. In this case, this event itself would not get
	   logged, but could still trigger actions such as an alert via PEF.
	   Note that an application can always use the Get SEL Info
	   command to determine whether the SEL is full or not. Since Get
	   SEL Info is a mandatory command, this provides a cross-platform
	   way to get that status. */
#define STO_ELD_SEL_ALMOST_FULL			0x05	// SEL Almost Full. 
	/* If Event Data 3 is not provided, then by default
	   this event represents the SEL has reached a point of being 75% 
	   or more full. For example, if the SEL supports 215 entries, the
	   75% value would be 161.25 entries. Therefore, the event would be
	   generated on the 162nd entry. Note that if this event itself is
	   logged, it would be logged as the 163rd entry.
	   Event Data 3
	   Contains hex value from 0 to 100 decimal (00h to 64h)
	   representing the % of which the SEL is filled at the time the
	   event was generated: 00h is 0% full (SEL is empty), 64h is
	   100% full, etc. */

/* Sensor Type: Watchdog 1 */
#define ST_WATCHDOG_1				0x11	// Watchdog 1
	/* This sensor is provided to support IPMI v0.9 to v1.0 transition.
	   This is deprecated in IPMI v1.5. See sensor 23h for recommended
	   definition of Watchdog sensor for new v1.0 and for IPMI v1.5
	   implementations. */
#define STO_WD_BIOS_WD_RESET			0x00	// BIOS Watchdog Reset
#define STO_WD_RESET				0x01	// OS Watchdog Reset
#define STO_WD_SHUTDOWN				0x02	// OS Watchdog Shut Down
#define STO_WD_POWER_DOWN			0x03	// OS Watchdog Power Down
#define STO_WD_POWER_CYCLE			0x04	// OS Watchdog Power Cycle
#define STO_WD_NMI_DIAG_INTERRUPT		0x05	// OS Watchdog NMI / Diagnostic Interrupt
#define STO_WD_EXPIRED				0x06	// OS Watchdog Expired, status only
#define STO_WD_PRE_TIMEOUT_INTR			0x07	// OS Watchdog pre-timeout Interrupt, non-NMI

/* Sensor Type: System Event */
#define ST_SYSTEM_EVENT				0x12	// System Event 

#define STO_SE_SYS_RECONFIG			0x00	// System Reconfigured
#define STO_SE_OEM_SYS_BOOT_EVENT		0x01	// OEM System Boot Event
#define STO_SE_UNDETERMINED_HW_FAILURE		0x02	// Undetermined system hardware failure
	/* (this event would typically require system-specific diagnostics to
	   determine FRU / failure type) */
#define STO_SE_ENTRY_ADDED_TO_AUX_LOG		0x03	// Entry added to Auxiliary Log
	/* (see 31.12, Get Auxiliary Log Status Command and 31.13, Set
	   Auxiliary Log Status Command, for more information)
	   Event Data 2
	   [7:4] - Log Entry Action */
#define LOG_ENTRY_ADDED				0x0	// entry added
#define LOG_ENTRY_NOT_IPMI_MAP			0x1	// entry added because event 
							// did not map to standard IPMI event
#define LOG_ENTRY_WITH_SEL_ENTRIES		0x2	// entry added along with one or more 
							// corresponding SEL entries
#define LOG_ENTRY_CLEARED			0x3	// log cleared
#define LOG_ENTRY_LOG_DISABLED			0x4	// log disabled
#define LOG_ENTRY_LOG_ENABLED			0x5	// log enabled
							// all other reserved
	/* [3:0] - Log Type */
#define LOG_TYPE_MCA				0x0	// MCA Log
#define LOG_TYPE_OEM1				0x1	// OEM 1
#define LOG_TYPE_OEM2				0x2	// OEM 2
							// all other reserved


#define STO_SE_PEF_ACTION			0x04	// PEF Action
	/* Event Data 2
	   The following PEF_ACTION_xx bits reflect the PEF Actions that are about to be
	   taken after the event filters have been matched. The event is
	   captured before the actions are taken.
	   [7:6] - reserved */

/* Sensor Type: Critical Interrupt */
#define ST_CRITICAL_INTERRUPT			0x13	// Critical Interrupt 

#define STO_CI_FRONT_PANEL		0x0 	// Front Panel NMI / Diagnostic Interrupt
#define STO_CI_BUS_TIMEOUT		0x1 	// Bus Timeout
#define STO_CI_IO_CHANNEL_CHECK_NMI	0x2 	// I/O channel check NMI
#define STO_CI_SOFTWARE_NMI		0x3 	// Software NMI
#define STO_CI_PCI_PERR			0x4	// PCI PERR
#define STO_CI_PCI_SERR			0x5	// PCI SERR
#define STO_CI_EISA_FAIL_SAFE_TIMEOUT	0x6	// EISA Fail Safe Timeout
#define STO_CI_BUS_CORRECTABLE_ERROR	0x7	// Bus Correctable Error
#define STO_CI_BUS_UNCORRECTABLE_ERROR	0x8	// Bus Uncorrectable Error
#define STO_CI_FATAL_NMI		0x9	// Fatal NMI (port 61h, bit 7)
#define STO_CI_BUS_FATAL_ERROR		0xA	// Bus Fatal Error

/* Sensor Type: Button / Switch */
#define ST_BUTTON_SWITCH		0x14	// Button / Switch 

#define STO_BS_POWER_BUTTON_PRESSED	0x0	// Power Button pressed
#define STO_BS_SLEEP_BUTTON_PRESSED	0x1	// Sleep Button pressed
#define STO_BS_RESET_BUTTON_PRESSED	0x2	// Reset Button pressed
#define STO_BS_FRU_LATCH_OPEN		0x3	// FRU latch open 
	/* (Switch indicating FRU latch is in ‘unlatched’ position and FRU
	   is mechanically removable) */
#define STO_BS_FRU_SERVICE_REQ_BUTTON	0x4	// FRU service request button 
						// (1 = pressed, service, e.g.
						// removal/replacement, requested)

/* Sensor Type: Module / Board */
#define ST_MODULE_BOARD			0x15	// Module / Board 

/* Sensor Type: Microcontroller / Coprocessor */
#define ST_UCONTROLLER_COPROCESSOR	0x16	// Microcontroller / Coprocessor

/* Sensor Type: Add-in Card */
#define ST_ADD_IN_CARD			0x17	// Add-in Card

/* Sensor Type: Chassis */
#define ST_CHASSIS			0x18	// Chassis

/* Sensor Type: Chip Set */
#define ST_CHIP_SET			0x18	// Chip Set
#define STO_CS_SOFT_POWER_CNTRL_FAILURE	0x00	// Soft Power Control Failure
	/* (chip set did not respond to BMC request to change system power state).
	   This offset is similar to offset 05h for a power unit, except that the 
	   power unit event is only related to a failure to power up, while this
	   event corresponds to any system power state change directly requested
	   via the BMC. */
	/* Event Data 2
	   The Event Data 2 field for this command can be used to provide
	   additional information on the type of failure with the following
	   definition:
	   Requested power state
	   00h = S0 / G0 “working”
	   01h = S1 “sleeping with system h/w & processor context maintained”
	   02h = S2 “sleeping, processor context lost”
	   03h = S3 “sleeping, processor & h/w context lost, memory retained.”
	   04h = S4 “non-volatile sleep / suspend-to disk”
	   05h = S5 / G2 “soft-off”
	   06h = S4 / S5 soft-off, particular S4 / S5 state cannot be determined
	   07h = G3 / Mechanical Off
	   08h = Sleeping in an S1, S2, or S3 states (used when particular 
	   S1, S2, S3 state cannot be determined)
	   09h = G1 sleeping (S1-S4 state cannot be determined)
	   0Ah = S5 entered by override
	   0Bh = Legacy ON state
	   0Ch = Legacy OFF state
	   0Dh = reserved
	   Event Data 3
	   The Event Data 3 field for this command can be used to provide
	   additional information on the type of failure with the following
	   definition:
	   Power state at time of request
	   00h = S0 / G0 “working”
	   01h = S1 “sleeping with system h/w & processor context maintained”
	   02h = S2 “sleeping, processor context lost”
	   03h = S3 “sleeping, processor & h/w context lost, memory retained.”
	   04h = S4 “non-volatile sleep / suspend-to disk”
	   05h = S5 / G2 “soft-off”
	   06h = S4 / S5 soft-off, particular S4 / S5 state cannot be determined
	   07h = G3 / Mechanical Off
	   08h = Sleeping in an S1, S2, or S3 states (used when particular 
	   S1, S2, S3 state cannot be determined)
	   09h = G1 sleeping (S1-S4 state cannot be determined)
	   0Ah = S5 entered by override
	   0Bh = Legacy ON state
	   0Ch = Legacy OFF state
	   0Dh = unknown
	   */

/* Sensor Type: Other FRU */
#define ST_OTHER_FRU				0x1A	// Other FRU 


/* Sensor Type: Cable / Interconnect */
#define ST_CABLE_OR_INTERCONNECT		0x1B	// Cable / Interconnect


/* Sensor Type: Terminator */
#define ST_TERMINATOR				0x1C	// Terminator


/* Sensor Type: System Boot Initiated */
#define ST_SYSTEM_BOOT_INITIATED		0x1D	// System Boot Initiated
#define	STO_SB_INITIATED_BY_POWER_UP		0x0	// Initiated by power up
#define	STO_SB_INITIATED_BY_HARD_RESET		0x1	// Initiated by hard reset
#define	STO_SB_INITIATED_BY_WARM_RESET		0x2	// Initiated by warm reset
#define	STO_SB_USER_REQUESTED_PXE_BOOT		0x3	// User requested PXE boot
#define	STO_SB_AUTO_BOOT_TO_DIAGNOSTIC		0x4	// Automatic boot to diagnostic


/* Sensor Type: Boot Error */
#define ST_BOOT_ERROR				0x1E	// Boot Error
#define STO_BE_NO_BOOTABLE_MEDIA		0x0	// No bootable media
#define STO_BE_NON_BOOTABLE_FLOPPY		0x1	// Non-bootable diskette left in drive
#define STO_BE_PXE_SERVER_NOT_FOUND		0x2	// PXE Server not found
#define STO_BE_INVALID_BOOT_SECTOR		0x3	// Invalid boot sector
#define STO_BE_BOOT_SELECTION_TIMEOUT		0x4	// Timeout waiting for user selection of boot source


/* Sensor Type: OS Boot */
#define ST_OS_BOOT				0x1F	// OS Boot
#define STO_OB_A_BOOT_COMPLETED			0x0	// A: boot completed
#define STO_OB_C_BOOT_COMPLETED			0x1	// C: boot completed
#define STO_OB_PXE_BOOT_COMPLETED		0x2	// PXE boot completed
#define STO_OB_DIAG_BOOT_COMPLETED		0x3	// Diagnostic boot completed
#define STO_OB_CD_BOOT_COMPLETED		0x4	// CD-ROM boot completed
#define STO_OB_ROM_BOOT_COMPLETED		0x5	// ROM boot completed
#define STO_OB_BOOT_COMPLETED			0x6	// boot completed - boot device not specified


/* Sensor Type: OS Critical Stop */
#define ST_OS_CRITICAL_STOP			0x20	// OS Critical Stop 
#define STO_OCS_STOP_DURING_LOAD_INIT		0x0	// Stop during OS load / initialization
#define STO_OCS_RUN_TIME_STOP			0x1	// Run-time Stop


/* Sensor Type: Slot / Connector */
#define ST_SLOT_CONNECTOR			0x21	// Slot / Connector 
#define STO_SC_FAULT_STATUS_ASSERTED		0x0	// Fault Status asserted
#define STO_SC_IDENTIFY_STATUS_ASSERTED		0x1	// Identify Status asserted
#define STO_SC_SLOT_DEVICE_INSTALLED		0x2	// Slot / Connector Device installed/attached
							// [This can include dock events]
#define STO_SC_SLOT_READY_FOR_INSTALL		0x3	
	/* Slot / Connector Ready for Device Installation - Typically, this
	   means that the slot power is off. The Ready for Installation,
	   Ready for Removal, and Slot Power states can transition
	   together, depending on the slot implementation. */
#define STO_SC_SLOT_READY_FOR_DEVICE_REMOVAL	0x4	// Slot/Connector Ready for Device Removal
#define STO_SC_SLOT_POWER_OFF			0x5	// Slot Power is Off
#define STO_SC_SLOT_DEVICE_REMOVAL_REQUEST	0x6	//Slot / Connector Device Removal Request
	/* This is typically connected to a switch that becomes asserted to request
	   removal of the device) */
#define STO_SC_INTERLOCK_ASSERTED		0x7	// Interlock asserted 
	/* This is typically connected to a switch that mechanically 
	   enables/disables power to the slot, or locks the slot in the 
	   ‘Ready for Installation / Ready for Removal states’ - depending
	   on the slot implementation. The asserted state indicates that the
	   lock-out is active.  */
#define STO_SC_SLOT_DISABLED			0x8	// Slot is Disabled
#define STO_SC_SLOT_HOLDS_SPARE_DEVICE		0x9	// Slot holds spare device
	/* The Event Data 2 & 3 fields can be used to provide an event
	   extension code, with the following definition:
	   Event Data 2
	   7 reserved
	   6:0 Slot/Connector Type
	   0 PCI
	   1 Drive Array
	   2 External Peripheral Connector
	   3 Docking
	   4 other standard internal expansion slot
	   5 slot associated with entity specified by Entity ID for sensor
	   6 AdvancedTCA
	   7 DIMM/memory device
	   8 FAN
	   9 PCI Express™
	   10 SCSI (parallel)
	   11 SATA / SAS
	   all other = reserved
	   
	   Event Data 3
	   7:0 Slot/Connector Number */

/* Sensor Type: System ACPI Power State */
#define ST_SYSTEM_ACPI_POWER_STATE		0x22	// System ACPI Power State
#define STO_SAPS_S0_G0_WORKING			0x0	// S0 / G0 “working”
#define STO_SAPS_S1_SLEEPING_WITH_CONTEXT	0x1	// S1 “sleeping with system h/w 
							// & processor context maintained”
#define STO_SAPS_S2_SLEEPING_CONTEXT_LOST	0x2	// S2 “sleeping, processor context lost”
#define STO_SAPS_S3_SLEEPING_MEM_RETAINED	0x3	// S3 “sleeping, processor & h/w context
							// lost, memory retained.”
#define STO_SAPS_S4_SUSPEND_TO_DISK		0x4	// S4 “non-volatile sleep / suspend-to disk”
#define STO_SAPS_S5_G2_SOFT_OFF			0x5	// S5 / G2 “soft-off”
#define STO_SAPS_S4_S5_SOFT_OFF			0x6	// S4 / S5 soft-off, particular 
							// S4 / S5 state cannot be determined
#define STO_SAPS_G3_MECHANICAL_OFF		0x7	// G3 / Mechanical Off
#define STO_SAPS_SLEEPING_UNKNOWN_S_STATE	0x8	// Sleeping in an S1, S2, or S3 states 
	/* (used when particular S1, S2, S3 state cannot be determined) */
#define STO_SAPS_G1_SLEEPING			0x9	// G1 sleeping (S1-S4 state cannot be determined)
#define STO_SAPS_S5_ENTERED_BY_OVERRIDE		0xA	// S5 entered by override
#define STO_SAPS_LEGACY_ON_STATE		0xB	// Legacy ON state
#define STO_SAPS_LEGACY_OFF_STATE		0xC	// Legacy OFF state
#define STO_SAPS_UNKNOWN_STATE			0xE	// Unknown


/* Sensor Type: Watchdog 2 */
#define ST_WATCHDOG_2				0x23	// Watchdog 2 
	/* This sensor is recommended for new IPMI v1.0 and later implementations. */
#define STO_WD2_TIMER_EXPIRED			0x0	// Timer expired, status only (no action, no interrupt)
#define STO_WD2_HARD_RESET			0x1	// Hard Reset
#define STO_WD2_POWER_DOWN			0x2	// Power Down
#define STO_WD2_POWER_CYCLE			0x3	// Power Cycle
						// 0x4-0x7 reserved
#define STO_WD2_TIMER_INTERRUPT			0x8	// Timer interrupt

	/* The Event Data 2 field for this command can be used to provide
	   an event extension code, with the following definition:
	   7:4 interrupt type
	   0h = none
	   1h = SMI
	   2h = NMI
	   3h = Messaging Interrupt
	   Fh = unspecified
	   all other = reserved
	   3:0 timer use at expiration:
	   0h = reserved
	   1h = BIOS FRB2
	   2h = BIOS/POST
	   3h = OS Load
	   4h = SMS/OS
	   5h = OEM
	   Fh = unspecified
	   all other = reserved */

/* Sensor Type: Platform Alert */
#define ST_PLATFORM_ALERT			0x24	// Platform Alert 
	/* This sensor can be used for returning the state and generating
	   events associated with alerts that have been generated by the
	   platform mgmt. subsystem */
#define STO_PA_PLATFORM_GENERATED_PAGE		0x0	// platform generated page
#define STO_PA_PLATFORM_LAN_ALERT		0x1	// platform generated LAN alert
#define STO_PA_PLATFORM_EVENT_TRAP		0x2	// Platform Event Trap generated,
							// formatted per IPMI PET specification
#define STO_PA_PLATFORM_SNMP_TRAP		0x3	// platform generated SNMP trap, OEM format

/* Sensor Type: Entity Presence */
#define ST_ENTITY_PRESENCE			0x25	// Entity Presence
	/* This sensor type provides a mechanism that allows a management 
	   controller to direct system management software to ignore a set
	   of sensors based on detecting that presence of an entity. This
	   sensor type is not typically used for event generation - but to 
	   just provide a present reading. */
#define STO_EP_ENTITY_PRESENT			0x0	// Entity Present. 
	/* This indicates that the Entity identified by the Entity ID for the 
	   sensor is present. */
#define STO_EP_ENTITY_ABSENT			0x1	// Entity Absent. 
	/* This indicates that the Entity identified by the Entity ID for the
	   sensor is absent. If the entity is absent, system management 
	   software should consider all sensors associated with that Entity
	   to be absent as well - and ignore those sensors. */
#define STO_EP_ENTITY_DISABLED			0x2	// Entity Disabled. 
	/* The Entity is present, but has been disabled. A deassertion of
	   this event indicates that the Entity has been enabled. */


/* Sensor Type: Monitor ASIC / IC */
#define ST_MONITOR_IC				0x26	// Monitor ASIC / IC


/* Sensor Type: LAN */
#define ST_LAN					0x27	// LAN 
#define STO_LAN_HEARTBEAT_LOST			0x0	// LAN Heartbeat Lost
#define STO_LAN_HEARTBEAT			0x1	// LAN Heartbeat


/* Sensor Type: Management Subsystem Health */
#define ST_MANAGEMENT_SUBSYSTEM_HEALTH		0x28	// Management Subsystem Health
#define STO_MSO_SENSOR_ACCESS_DEGRADED		0x0	// sensor access degraded or unavailable
#define STO_MSO_CONTROLLER_ACCESS_DEGRADED	0x1	// controller access degraded or unavailable
#define STO_MSO_MGMT_CONTROLLER_OFF_LINE	0x2	// management controller off-line
#define STO_MSO_MGMT_CONTROLLER_UNAVAILABLE	0x3	// management controller unavailable


/* Sensor Type: Battery */
#define ST_BATTERY				0x29	// Battery 
#define STO_BAT_BATTERY_LOW			0x0	// battery low (predictive failure)
#define STO_BAT_BATTERY_FAILED			0x1	// battery failed
#define STO_BAT_BATTERY_PRESENT			0x2	// battery presence detected


/* Sensor Type: Session Audit */
#define ST_SESSION_AUDIT			0x2A	// Session Audit
#define STO_SA_SESSION_ACTIVATED		0x0	// Session Activated
#define STO_SA_SESSION_DEACTIVATED		0x1	// Session Deactivated
	/* The Event Data 2 & 3 fields can be used to provide an event
	   extension code, with the following definition:
	   Event Data 2
	   7:6 reserved
	   5:0 User ID for user that activated session.
	   00_0000b = unspecified.
	   Event Data 3
	   7:6 reserved
	   5:4 Deactivation cause
	   00b = Session deactivatation cause unspecified. This
	   value is also used for Session Activated events.
	   01b = Session deactivated by Close Session command
	   10b = Session deactivated by timeout
	   11b = Session deactivated by configuration change
	   3:0 Channel number that session was activated/deactivated
	   over. Use channel number that session was activated over if a
	   session was closed for an unspecified reason, a timeout, or a
	   configuration change. */

/* Sensor Type: Version Change */
#define ST_VERSION_CHANGE			0x2B	// Version Change 
#define STO_VC_HARDWARE_CHANGE			0x0	
	/* Hardware change detected with associated Entity. Informational.
	   This offset does not imply whether the hardware change was
	   successful or not. Only that a change occurred. */
#define STO_VC_SW_CHANGE			0x1	
	/* Firmware or software change detected with associated Entity.
	   Informational. Success or failure not implied. */
#define STO_VC_HW_INCOMPATIBILITY		0x2	
	/* Hardware incompatibility detected with associated Entity. */
#define STO_VC_SW_INCOMPATIBILITY		0x3	
	/* Firmware or software incompatibility detected with associated Entity. */
#define STO_VC_UNSUPPORTED_HW			0x4	
	/* Entity is of an invalid or unsupported hardware version. */
#define STO_VC_UNSUPPORTED_SW_VERSION		0x5
	/* Entity contains an invalid or unsupported firmware or software version. */
#define STO_VC_HW_CHANGE_SUCCESSFUL		0x6
	/* Hardware Change detected with associated Entity was successful.
	  (deassertion event means ‘unsuccessful’). */
#define STO_VC_SW_CHANGE_SUCCESSFUL		0x7
       /* Software or F/W Change detected with associated Entity was
	  successful. (deassertion event means ‘unsuccessful’)
	  Event data 2 can be used for additional event information on the
	  type of version change, with the following definition:
	  Event Data 2
	  7:0 Version change type
	  00h unspecified
	  01h management controller device ID (change in one or
	  more fields from ‘Get Device ID’)
	  02h management controller firmware revision
	  03h management controller device revision
	  04h management controller manufacturer ID
	  05h management controller IPMI version
	  06h management controller auxiliary firmware ID
	  07h management controller firmware boot block
	  08h other management controller firmware
	  09h system firmware (EFI / BIOS) change
	  0Ah SMBIOS change
	  0Bh operating system change
	  0Ch operating system loader change
	  0Dh service or diagnostic partition change
	  0Eh management software agent change
	  0Fh management software application change
	  10h management software middleware change
	  11h programmable hardware change (e.g. FPGA)
	  12h board/FRU module change (change of a module
	  plugged into associated entity)
	  13h board/FRU component change (addition or removal of
	  a replaceable component on the board/FRU that is
	  not tracked as a FRU)
	  14h board/FRU replaced with equivalent version
	  15h board/FRU replaced with newer version
	  16h board/FRU replaced with older version
	  17h board/FRU hardware configuration change (e.g. strap,
	  jumper, cable change, etc.) */

/* Sensor Type: FRU State */
#define ST_FRU_STATE				0x2C 	// FRU State 
#define STO_FS_FRU_NOT_INSTALLED		0x0	// FRU Not Installed
#define STO_FS_FRU_INACTIVE			0x1	// FRU Inactive (in standby or ‘hot spare’ state)
#define STO_FS_FRU_ACTIVATION_REQUESTED		0x2	// FRU Activation Requested
#define STO_FS_FRU_ACTIVATION_IN_PROGRESS	0x3	// FRU Activation In Progress
#define STO_FS_FRU_ACTIVE			0x4	// FRU Active
#define STO_FS_FRU_DEACTIVATION_REQUESTED	0x5	// FRU Deactivation Requested
#define STO_FS_FRU_DEACTIVATION_IN_PROGRESS	0x6	// FRU Deactivation In Progress
#define STO_FS_FRU_COMMUNICATION_LOST		0x7	// FRU Communication Lost
	/* The Event Data 2 field for this command can be used to provide
	   the cause of the state change and the previous state:
	   7:4 Cause of state change
	   0h = Normal State Change.
	   1h = Change Commanded by software external to FRU.
	   2h = State Change due to operator changing a Handle latch.
	   3h = State Change due to operator pressing the hot swap push button.
	   4h = State Change due to FRU programmatic action.
	   5h = Communication Lost.
	   6h = Communication Lost due to local failure.
	   7h = State Change due to unexpected extraction.
	   8h = State Change due to operator intervention/update.
	   9h = Unable to compute IPMB address.
	   Ah = Unexpected Deactivation.
	   Fh = State Change, Cause Unknown.
	   All other = reserved
	   3:0 Previous state offset value (return offset for same state
	   as present state if previous state is unknown)
	   All other = reserved. */
	   
/* Sensor Type: OEM Reserved C0h-FFh */
/*
The IPMI specification does provide for the possibility of discrete sensors with OEM-defined
sensor-specific states, using Sensor Type Codes in the range C0h to FFh. To ensure that PICMG®
specifications can define sensor types that can be compatibly implemented and used on an
interoperable basis, PICMG® reserves 16 of the 64 OEM Sensor Type Codes (the range F0h
through FFh) for use with PICMG®-defined sensors. This specification assigns the first two codes
in this range (F0h and F1h) to the hot swap and IPMB-0 sensors, respectively.
*/
#define ST_HOT_SWAP				0xF0
#define ST_IPMB_0				0xF1

/* AMC MMC Module Hot Swap sensor. */
#define ST_MODULE_HOT_SWAP			0xF2

/* Hot swap events */
#define MODULE_HANDLE_CLOSED		0 // Module Handle Closed
#define MODULE_HANDLE_OPENED		1 // Module Handle Opened
#define MODULE_QUIESCED			2 // Quiesced
#define MODULE_BACKEND_POWER_FAILURE	3 // Backend Power Failure
#define MODULE_BACKEND_POWER_SHUTDOWN	4 // Backend Power Shut Down

/* Hot swap switch electrical states */
#define HANDLE_SWITCH_OPEN	1	// pulled high when switch disengaged
#define HANDLE_SWITCH_CLOSED	0	// connected to ground when the switch is engaged

/*======================================================================*/
/*
 *    SDR SENSOR DATA RECORD FORMATS
 *    (Ch. 43)
 *
 */
#define ENTITY_TYPE_PHYSICAL	0	/* treat entity as a physical entity per Entity ID table */
#define ENTITY_TYPE_LOGICAL	1	/* treat entity as a logical container entity. */

typedef struct full_sensor_record {
	/* SENSOR RECORD HEADER */
	uchar record_id[2];	/* 1:2 Record ID - The Record ID is used by 
				   the Sensor Data Repository device for record
				   organization and access. It is not related 
				   to the sensor ID. */
	uchar sdr_version;	/* 3 SDR Version - Version of the Sensor Model
				   specification that this record is compatible 
				   with. 51h for this specification. BCD 
				   encoded with bits 7:4 holding the Least 
				   Significant digit of the revision and bits
				   3:0 holding the Most Significant bits. */
	uchar record_type;	/* 4 Record Type - Record Type Number = 01h, 
				   Full Sensor Record */
	uchar record_len;	/* 5 Record Length - Number of remaining 
				   record bytes following. */
	/* RECORD KEY BYTES */
#ifdef BF_MS_FIRST
	uchar	owner_id:7,	/* 6 Sensor Owner ID - [7:1] - 7-bit I2C Slave
				   Address, or 7-bit system software ID */
		id_type:1;	/* [0] - 0b = ID is IPMB Slave Address, 
				   1b = system software ID */
#else
	uchar	id_type:1,
		owner_id:7;
		
#endif
#ifdef BF_MS_FIRST
	uchar	channel_num:4,	/* 7 Sensor Owner LUN - 
				   [7:4] - Channel Number. The Channel Number
				   can be, used to specify access to sensors that
				   are located on management controllers that 
				   are connected to the BMC via channels other
				   than the primary IPMB. (Note: In IPMI v1.5 
				   the ordering of bits 7:2 of this byte have
				   changed to support the 4-bit channel number.) */
		rsv1:2,		/* [3:2] - reserved */
		sensor_owner_lun:2; /* [1:0] - Sensor Owner LUN. LUN in the Sensor
				   Owner that is used to send/receive IPMB 
				   messages to access the sensor. 00b if system
				   software is Sensor Owner. */
#else
	uchar	sensor_owner_lun:2,
		rsv1:2,
		channel_num:4;
#endif
	uchar sensor_number;	/* 8 Sensor Number - Unique number identifying
				   the sensor behind a given slave address and 
				   LUN. Code FFh reserved. */
	/* RECORD BODY BYTES */
	uchar entity_id;	/* 9 Entity ID - Indicates the physical entity
				   that the sensor is monitoring or is otherwise
				   associated with the sensor. See Table 43-13,
				   Entity ID Codes. */
#ifdef BF_MS_FIRST
	uchar	entity_type:1,	/* 10 Entity Instance - 
				   [7] - 0b = treat entity as a physical entity
				   per Entity ID table (ENTITY_TYPE_PHYSICAL)
				   1b = treat entity as a logical container entity. 
				   For example, if this bit is set, and the Entity 
				   ID is ‘Processor’, the container entity would
				   be considered to represent a logical 
				   ‘Processor Group’ rather than a physical 
				   processor. This bit is typically used in 
				   conjunction with an Entity Association record.
				   (ENTITY_TYPE_LOGICAL) */
		entity_instance_num:7; /* [6:0] - Instance number for entity. 
				   (See section 39.1, System- and Device-relative
				   Entity Instance Values for more information)
				   00h-5Fh system-relative Entity Instance. 
				   The Entity Instance number must be unique 
				   for each different entity of the same type 
				   Entity ID in the system.
				   60h-7Fh device-relative Entity Instance.
				   The Entity Instance number must only be 
				   unique relative to the management controller
				   providing access to the Entity. */
#else
	uchar	entity_instance_num:7,
		entity_type:1;
#endif
#ifdef BF_MS_FIRST
	uchar	rsv2:1,		/* 11 Sensor Initialization - [7] - reserved. Write as 0b. */
		init_scanning:1,	/* [6] - Init Scanning 1b = enable scanning 
				   (this bit=1 implies that the sensor accepts
				   the ‘enable/disable scanning’ bit in the 
				   Set Sensor Event Enable command). */
		init_events:1,	/* [5] - Init Events 1b = enable events (per 
				   Sensor Event Message Control Support bits
				   in Sensor Capabilities field, and per the
				   Event Mask fields, below). */
		init_thresholds:1, /* [4] - Init Thresholds 1b = initialize sensor
				   thresholds (per settable threshold mask below). */
		init_hysteresis:1, /* [3] - Init Hysteresis 1b = initialize sensor
				   hysteresis (per Sensor Hysteresis Support
				   bits in the Sensor Capabilities field, below). */
		init_sensor_type:1, /* [2] - Init Sensor Type 1b = initialize 
				   Sensor Type and Event / Reading Type code. */

		/* Sensor Default (power up) State 
		   -------------------------------
		   Reports how this sensor comes up on device power
		   up and hardware/cold reset.
		   The Initialization Agent does not use this
		   bit. This bit solely reports to software
		   how the sensor comes prior to being 
		   initialized by the Initialization Agent. */
		powerup_evt_generation:1, /* [1] - 0b = event generation disabled,
				   1b = event generation enabled */
		powerup_sensor_scanning:1;	/* [0] - 0b = sensor scanning disabled,
				   1b = sensor scanning enabled */
#else
	uchar	powerup_sensor_scanning:1,
		powerup_evt_generation:1,
		init_sensor_type:1,
		init_hysteresis:1,
		init_thresholds:1,
		init_events:1,
		init_scanning:1,
		rsv2:1;
#endif
#ifdef BF_MS_FIRST
	uchar	ignore_sensor:1,	/* 12 Sensor Capabilities - [7] 
				   1b = Ignore sensor if Entity is not present 
				   or disabled.
				   0b = don’t ignore sensor */
		/* Sensor Auto Re-arm Support
		   --------------------------
		   Indicates whether the sensor requires manual rearming, or 
		   automatically rearms itself when the event clears. ‘manual’
		   implies that the get sensor event status and rearm sensor
		   events commands are supported */
		sensor_manual_support:1,	/* [6] - 0b = no (manual), 1b = yes (auto) */
				    
		/* Sensor Hysteresis Support
		   ------------------------- */
		sensor_hysteresis_support:2, /* [5:4] 
					00b = No hysteresis, or hysteresis built-in
				       	      but not specified.
					01b = hysteresis is readable.
					10b = hysteresis is readable and settable.
					11b = Fixed, unreadable, hysteresis. 
					      Hysteresis fields values implemented
					      in the sensor. */
					
		/* Sensor Threshold Access Support
		   ------------------------------- */
		sensor_threshold_access:2,	/* [3:2]
					00b = no thresholds.
					01b = thresholds are readable, per Reading
				              Mask, below.
					10b = thresholds are readable and settable
				              per Reading Mask and Settable Threshold 
					      Mask, respectively.
					11b = Fixed, unreadable, thresholds. Which
				              thresholds are supported is
					      reflected by the Reading Mask. The
					      threshold value fields report the
					      values that are ‘hard-coded’ in the
					      sensor. */
				     
		/* Sensor Event Message Control Support
		   ------------------------------------ 
		   Indicates whether this sensor generates Event Messages, and 
		   if so, what type of Event Message control is offered. */
		event_msg_control:2;	/* [1:0] - 
				   00b = per threshold/discrete-state event 
				   enable/disable control (implies that entire
				   sensor and global disable are also supported)
				   01b = entire sensor only (implies that global
				   disable is also supported)
				   10b = global disable only
				   11b = no events from sensor */
#else
	uchar	event_msg_control:2,
		sensor_threshold_access:2,
		sensor_hysteresis_support:2,
		sensor_manual_support:1,
		ignore_sensor:1;
#endif
	uchar sensor_type;	/* 13 Sensor Type - Code representing the sensor 
				   type. From Table 42-3, Sensor Type Codes.
				   E.g. Temperature, Voltage, Processor, etc. */
	uchar event_type_code;	/* 14 Event / Reading Type Code - Event/Reading 
				   Type Code. From Table 42-1, Event/Reading
				   Type Code Ranges. */
	unsigned short event_mask;	/* 15 & 16 Assertion Event Mask / Lower
				   Threshold Reading Mask - This field reports
				   the assertion event generation or threshold 
				   event generation capabilities for a discrete
				   or threshold-based sensor, respectively. 
				   This field is also used by the init agent to
				   enable assertion event generation when the ‘Init
				   Events’ bit in the Sensor Capabilities field 
				   is set and the Sensor Event Message Control
				   Support field indicates that the sensor has 
				   ‘per threshold/discrete state’ event enable
				   control. */
	/* Assertion Event Mask (for non- threshold-based sensors)
	   -------------------------------------------------------
	   The Event Mask bytes are a bit mask that specifies support for 15 successive
	   events starting with the event specified by Event/Reading Type Code. LS byte
	   first. 
	   [15] - reserved. Write as ‘0’.
	   [14:0] - Event offsets 14 through 0, respectively.
	   1b = assertion event can be generated by this sensor
	   Lower Threshold Reading Mask (for threshold-based sensors)
	   Indicates which lower threshold comparison status is returned via the Get Sensor
	   Reading command.
	   [15] - reserved. Write as 0b
	   [14] - 1b = Lower non-recoverable threshold comparison is returned
	   [13] - 1b = Lower critical threshold is comparison returned
	   [12] - 1b = Lower non-critical threshold is comparison returned
	   Threshold Assertion Event Mask (for threshold-based sensors)
	   [11] - 1b = assertion event for upper non-recoverable going high supported
	   [10] - 1b = assertion event for upper non-recoverable going low supported
	   [9] - 1b = assertion event for upper critical going high supported
	   [8] - 1b = assertion event for upper critical going low supported
	   [7] - 1b = assertion event for upper non-critical going high supported
	   [6] - 1b = assertion event for upper non-critical going low supported
	   [5] - 1b = assertion event for lower non-recoverable going high supported
	   [4] - 1b = assertion event for lower non-recoverable going low supported
	   [3] - 1b = assertion event for lower critical going high supported
	   [2] - 1b = assertion event for lower critical going low supported
	   [1] - 1b = assertion event for lower non-critical going high supported
	   [0] - 1b = assertion event for lower non-critical going low supported */
#define AE_LOWER_NON_RECOVERABLE_THRESHOLD_COMPARISON_RETURNED	0x4000	/* bit 14 */
#define AE_LOWER_CRITICAL_THRESHOLD_IS_COMPARISON_RETURNED	0x2000  /* bit 13 */
#define AE_LOWER_NON_CRITICAL_THRESHOLD_IS_COMPARISON_RETURNED	0x1000  /* bit 12 */
	
#define AE_UPPER_NON_RECOVERABLE_GOING_HIGH_SUPPORTED		0x0800  /* bit 11 */
#define AE_UPPER_NON_RECOVERABLE_GOING_LOW_SUPPORTED		0x0400  /* bit 10 */

#define AE_UPPER_CRITICAL_GOING_HIGH_SUPPORTED			0x0200	/* bit 9 */
#define AE_UPPER_CRITICAL_GOING_LOW_SUPPORTED			0x0100

#define AE_UPPER_NON_CRITICAL_GOING_HIGH_SUPPORTED		0x0080	/* bit 7 */
#define AE_UPPER_NON_CRITICAL_GOING_LOW_SUPPORTED		0x0040

#define AE_LOWER_NON_RECOVERABLE_GOING_HIGH_SUPPORTED		0x0020	/* bit 5 */
#define AE_LOWER_NON_RECOVERABLE_GOING_LOW_SUPPORTED		0x0010
	
#define AE_LOWER_CRITICAL_GOING_HIGH_SUPPORTED			0x0008	/* bit 3 */
#define AE_LOWER_CRITICAL_GOING_LOW_SUPPORTED			0x0004

#define AE_LOWER_NON_CRITICAL_GOING_HIGH_SUPPORTED		0x0002	/* bit 1 */
#define AE_LOWER_NON_CRITICAL_GOING_LOW_SUPPORTED		0x0001

	/* Deassertion Event Mask / Upper Threshold Reading Mask
	   ----------------------------------------------------- */
	unsigned short deassertion_event_mask;	/* 17 & 18 - Deassertion Event Mask 
					   (for non- threshold-based sensors)
	The Event Mask bytes are a bit mask that specifies support for 15 successive
	events starting with the event specified by Event/Reading Type Code. LS byte
	first.
	[15] - reserved. Write as 0b
	[14:0] - Event offsets 14 through 0, respectively.
	1b = assertion event can be generated for this state.
	Upper Threshold Reading Mask (for threshold-based sensors)
	Indicates which upper threshold comparison status is returned via the Get Sensor
	Reading command.
	[15] - reserved. Write as 0b
	[14] - 1b = Upper non-recoverable threshold comparison is returned
	[13] - 1b = Upper critical threshold is comparison returned
	[12] - 1b = Upper non-critical threshold is comparison returned
	Threshold Deassertion Event Mask
	[11] - 1b = deassertion event for upper non-recoverable going high supported
	[10] - 1b = deassertion event for upper non-recoverable going low supported
	[9] - 1b = deassertion event for upper critical going high supported
	[8] - 1b = deassertion event for upper critical going low supported
	[7] - 1b = deassertion event for upper non-critical going high supported
	[6] - 1b = deassertion event for upper non-critical going low supported
	[5] - 1b = deassertion event for lower non-recoverable going high supported
	[4] - 1b = deassertion event for lower non-recoverable going low supported
	[3] - 1b = deassertion event for lower critical going high supported
	[2] - 1b = deassertion event for lower critical going low supported
	[1] - 1b = deassertion event for lower non-critical going high supported
	[0] - 1b = deassertion event for lower non-critical going low supported
	*/

	/* Discrete Reading Mask / Settable Threshold Mask, Readable Threshold Mask 
	   ------------------------------------------------------------------------ */
	unsigned short reading_mask;	/* 19 & 20 - Reading Mask (for non- threshold 
				   based sensors). Indicates what discrete 
	readings can be returned by this sensor, or, for threshold based 
	sensors, this indicates which thresholds are settable and which are
	readable. The Reading Mask bytes are a bit mask that specifies support for 15
	successive states starting with the value from Table 36-1, Event/Reading Type
	Code Ranges. LS byte first.
	[15] - reserved. Write as 0b
	[14:0] - state bits 0 through 14.
	1b = discrete state can be returned by this sensor.
	
	Settable Threshold Mask (for threshold-based sensors)
	-----------------------------------------------------
	Indicates which thresholds are settable via the Set Sensor Thresholds.
       	This mask also indicates which threshold values will be initialized if
       	the ‘Init Events’ bit is set. LS byte first.
	[15:14] - reserved. Write as 00b.
	[13] - 1b = Upper non-recoverable threshold is settable
	[12] - 1b = Upper critical threshold is settable
	[11] - 1b = Upper non-critical threshold is settable
	[10] - 1b = Lower non-recoverable threshold is settable
	[9] - 1b = Lower critical threshold is settable
	[8] - 1b = Lower non-critical threshold is settable
	
	Readable Threshold Mask (for threshold-based sensors)
	-----------------------------------------------------
	Indicates which thresholds are readable via the Get Sensor Thresholds
	command.
	[7:6] - reserved. Write as 00b.
	[5] - 1b = Upper non-recoverable threshold is readable
	[4] - 1b = Upper critical threshold is readable
	[3] - 1b = Upper non-critical threshold is readable
	[2] - 1b = Lower non-recoverable threshold is readable
	[1] - 1b = Lower critical threshold is readable
	[0] - 1b = Lower non-critical threshold is readable */
#ifdef BF_MS_FIRST
	uchar	analog_data_format:2,	/* 21 Sensor Units  
					   [7:6] - Analog (numeric) Data Format.
					   Specifies threshold and ‘analog’ reading,
					   if ‘analog’ reading provided. If neither
					   thresholds nor analog reading are 
					   provided, this field should be written
					   as 00h.
					   00b = unsigned
					   01b = 1’s complement (signed)
					   10b = 2’s complement (signed)
					   11b = Does not return analog (numeric) reading */
		rate_unit:3,		/* [5:3] - Rate unit
					   000b = none
					   001b = per µS
					   010b = per ms
					   011b = per s
					   100b = per minute
					   101b = per hour
					   110b = per day
					   111b = reserved */
		modifier_unit:2,		/* [2:1] - Modifier unit
					   00b = none
					   01b = Basic Unit / Modifier Unit
					   10b = Basic Unit * Modifier Unit
					   11b = reserved */
		percentage:1;		/* [0] - Percentage 0b = no, 1b = yes */
#else
	uchar	percentage:1,
		modifier_unit:2,
		rate_unit:3,
		analog_data_format:2;
#endif
	uchar sensor_units2;	/* 22 Sensor Units 2 - Base Unit  
				   [7:0] - Units Type code: See Table 43-15,
				   Sensor Unit Type Codes. */
	uchar sensor_units3;	/* 23 Sensor Units 3 - Modifier Unit
				   [7:0] - Units Type code, 00h if unused. */
	uchar linearization;	/* 24 Linearization - 
				   [7] - reserved
				   [6:0] - enum (linear, ln, log10, log2, e, 
				   exp10, exp2, 1/x, sqr(x), cube(x), sqrt(x), 
				   cuberoot(x) ) - 
				   70h = non-linear. 
				   71h-7Fh = non-linear, OEM defined. */
	uchar M;		/* 25 M 
				   [7:0] - M: LS 8 bits [2’s complement, 
				   signed, 10 bit ‘M’ value.] */
	uchar M_tolerance;	/* 26 M, Tolerance -
				   [7:6] - M: MS 2 bits
				   [5:0] - Tolerance: 6 bits, unsigned 
				   (Tolerance in +/- ½ raw counts) */
	uchar B;		/* 27 B 
				   [7:0] - B: LS 8 bits [2’s complement, 
				   signed, 10-bit ‘B’ value.] */
	uchar B_accuracy;	/* 28 B, Accuracy 
				   [7:6] - B: MS 2 bits Unsigned, 10-bit
				   Basic Sensor Accuracy in 1/100 percent
				   scaled up by unsigned Accuracy exponent:
				   [5:0] - Accuracy: LS 6 bits */
	uchar accuracy;		/* 29 Accuracy, Accuracy exp, Sensor Direction
				   [7:4] - Accuracy: MS 4 bits
				   [3:2] - Accuracy exp: 2 bits, unsigned
				   [1:0] - Sensor Direction. Indicates whether
				   the sensor is monitoring an input or
				   output relative to the given Entity. E.g. if
				   the sensor is monitoring a current, this can
				   be used to specify whether it is an input 
				   voltage or an output voltage.
				   00b = unspecified / not applicable
				   01b = input
				   10b = output
				   11b = reserved */
	uchar R_B_exp;		/* 30 R exp, B exp 
				   [7:4] - R (result) exponent 4 bits, 2’s 
				   complement, signed
				   [3:0] - B exponent 4 bits, 2’s complement, signed */
	uchar analog_characteristic_flags;	/* 31 Analog characteristic flags 
				   [7:3] - reserved
				   [2] - normal min specified 1b = yes, 
				         0b = normal min field unspecified
				   [1] - normal max specified 1b = yes, 
				         0b = normal max field unspecified
				   [0] - nominal reading specified 1b = yes, 
				         0b = nominal reading field unspecified */
	uchar nominal_reading;	/* 32 Nominal Reading - Given as a raw value. 
				   Must be converted to units-based value using
				   the ‘y=Mx+B’ formula. 1’s or 2’s complement 
				   signed or unsigned per flag bits in Sensor 
				   Units 1. */
	uchar normal_maximum;	/* 33 Normal Maximum - Given as a raw value.
				   Must be converted to units-based value using
				   the ‘y=Mx+B’ formula. 1’s or 2’s complement
				   signed or unsigned per ‘signed’ bit in Sensor
				   Units 1. */
	uchar normal_minimum;	/* 34 Normal Minimum - Given as a raw value.
				   Must be converted to units-based value using
				   the ‘y=Mx+B’ formula. Signed or unsigned per
				   ‘signed’ bit in Sensor Units 1. */
	uchar sensor_maximum_reading;	/* 35 Sensor Maximum Reading - Given as
				   a raw value. Must be converted to units-based
				   value based using the y=Mx+B formula. 
				   Signed or unsigned per ‘signed’ bit in sensor
				   flags. Normally ‘FFh’ for an 8-bit unsigned 
				   sensor, but can be a lesser value if the sensor
				   has a restricted range. If max. reading cannot
				   be pre-specified this value should be set
				   to max value, based on data format, (e.g. FFh
				   for an unsigned sensor, 7Fh for 2’s complement,
				   etc.) */
	uchar sensor_minimum_reading;	/* 36 Sensor Minimum Reading -
				   Given as a raw value. Must be converted to
				   units-based value using the ‘y=Mx+B’ formula.
				   Signed or unsigned per ‘signed’ bit in sensor
				   flags. If min. reading cannot be pre-specified
				   this value should be set to min value, based 
				   on data format, (e.g. 00h for an unsigned 
				   sensor, 80h for 2’s complement, etc.) */
	uchar upper_non_recoverable_threshold;
				/* 37 Upper non-recoverable Threshold - Use of 
				   this field is based on Settable Threshold 
				   Mask. If the corresponding bit is set in the 
				   mask byte and the ‘Init Sensor Thresholds’
				   bit is also set, then this value will be 
				   used for initializing the sensor threshold.
				   Otherwise, this value should be ignored.
				   The thresholds are given as raw values that
				   must be converted to units-based values using
				   the ‘y=Mx+B’ formula. */
	uchar upper_critical_threshold;
				/* 38 Upper critical Threshold - Use of this
				   field is based on Settable Threshold Mask, 
				   above */
	uchar upper_non_critical_threshold;
				/* 39 Upper non-critical Threshold - Use of 
				   this field is based on Settable Threshold Mask,
				   above */
	uchar lower_non_recoverable_threshold;	
				/* 40 Lower non-recoverable Threshold - Use of
				   this field is based on Settable Threshold
				   Mask, above */
	uchar lower_critical_threshold;
				/* 41 Lower critical Threshold - Use of this 
				   field is based on Settable Threshold Mask,
				   above */
	uchar lower_non_critical_threshold;
				/* 42 Lower non-critical Threshold - Use of 
				   this field is based on Settable Threshold 
				   Mask, above */
	uchar positive_going_threshold_hysteresis_value;
				/* 43 Positive-going Threshold Hysteresis value
				   Positive hysteresis is defined as the 
				   unsigned number of counts that are subtracted
				   from the raw threshold values to create the 
				   ‘re-arm’ point for all positive-going 
				   thresholds on the sensor. 0 indicates that
				   there is no hysteresis on positive-going
				   thresholds for this sensor. Hysteresis values
				   are given as raw counts. That is, to find the
				   degree of hysteresis in units, the value must
				   be converted using the ‘y=Mx+B’ formula. */
	uchar negative_going_threshold_hysteresis_value;
				/* 44 Negative-going Threshold Hysteresis value
				   Negative hysteresis is defined as the 
				   unsigned number of counts that are added
				   to the raw threshold value to create the 
				   ‘re-arm’ point for all negative-going
				   thresholds on the sensor. 0 indicates that
				   there is no hysteresis on negative-going
				   thresholds for this sensor. */
	uchar reserved2;	/* 45 reserved. Write as 00h. */
	uchar reserved3;	/* 46 reserved. Write as 00h. */
	uchar oem;		/* 47 OEM - Reserved for OEM use. */
#ifdef BF_MS_FIRST
	uchar	id_string_type:2,
				/* 48 ID String Type/Length Code - Sensor ‘ID’
				   String Type/Length Code, per Section 43.15, 
				   Type/Length Byte Format. (copied here)
				   [7:6] 00 = Unicode
				         01 = BCD plus (see below)
				         10 = 6-bit ASCII, packed
				         11 = 8-bit ASCII + Latin 1. 
				   At least two bytes of data must be present 
				   when this type is used. Therefore, the
				   length (number of data bytes) will be >1 if
				   data is present, 0 if data is not present.
				   A length of 1 is reserved. */
		rsv3:1,		/* [5] reserved. */
		id_string_length:5; /* [4:0] length of following data, in characters.
				   00000b indicates ‘none following’. 
				   11111b = reserved. */
#else
	uchar	id_string_length:5,
		rsv3:1,
		id_string_type:2;
#endif
	uchar id_string_bytes[16];
				/* 49:+N ID String Bytes - Sensor ID String bytes.
				   Only present if non-zero length in Type/Length 
				   field. 16 bytes, maximum. Note: the SDR can 
				   be implemented as a fixed length record.
				   Bytes beyond the ID string bytes are 
				   unspecified and should be ignored. */
} FULL_SENSOR_RECORD;

// Table 43-2, Compact Sensor Record - SDR Type 02h

typedef struct compact_sensor_record {
	 // SENSOR RECORD HEADER
 	uchar	record_id[2];	/* The Record ID is used by the Sensor Data Repository 
				   device for record organization and access. This may 
				   not actually be stored, but may be calculated when 
				   records are accessed.  It is not related to the sensor ID. */
	uchar	sdr_version;	/* SDR Version. Version of the Sensor Model specification
				   that this record is compatible with. 51h for this 
				   specification. This is BCD encoded with bits 7:4 holding
				   the Least Significant digit of the revision and 
				   bits 3:0 holding the Most Significant bits. E.g. 51h
				   corresponds to “1.5”. */
	uchar	record_type;	/* Record Type Number = 02h, Compact Sensor Record */
	uchar	record_len;	/* Record Length. Number of remaining record bytes following. */

	// RECORD KEY BYTES
#ifdef BF_MS_FIRST
	uchar	owner_id:7,	/* 6 Sensor Owner ID - [7:1] - 7-bit I2C Slave
				   Address, or 7-bit system software ID */
		id_type:1;	/* [0] - 0b = ID is IPMB Slave Address, 
				   1b = system software ID */
#else
	uchar	id_type:1,
		owner_id:7;
#endif
#ifdef BF_MS_FIRST
	uchar	channel_num:4,	/* 7 Sensor Owner LUN - 
				   [7:4] - Channel Number. The Channel Number
				   can be, used to specify access to sensors that
				   are located on management controllers that 
				   are connected to the BMC via channels other
				   than the primary IPMB. (Note: In IPMI v1.5 
				   the ordering of bits 7:2 of this byte have
				   changed to support the 4-bit channel number.) */
		fru_owner_lun:2,/* [3:2] - FRU Inventory Device Owner LUN. LUN
				   for Write/Read FRU commands to access FRU
				   information. 00b otherwise. */
		sensor_owner_lun:2; /* [1:0] - Sensor Owner LUN. LUN in the Sensor
				   Owner that is used to send/receive IPMB 
				   messages to access the sensor. 00b if system
				   software is Sensor Owner. */
#else
	uchar	sensor_owner_lun:2,
		fru_owner_lun:2,
		channel_num:4;
#endif
	uchar	sensor_number;	/* Sensor Number - Unique number identifying 
				   the sensor behind the given slave address 
				   and LUN. Code FFh reserved. */
	// RECORD BODY BYTES
	uchar entity_id;	/* 9 Entity ID - Indicates the physical entity
				   that the sensor is monitoring or is otherwise
				   associated with the sensor. See Table 43-13,
				   Entity ID Codes. */
#ifdef BF_MS_FIRST
	uchar	entity_type:1,	/* 10 Entity Instance - 
				   [7] - 0b = treat entity as a physical entity
				   per Entity ID table (ENTITY_TYPE_PHYSICAL)
				   1b = treat entity as a logical container entity. 
				   For example, if this bit is set, and the Entity 
				   ID is ‘Processor’, the container entity would
				   be considered to represent a logical 
				   ‘Processor Group’ rather than a physical 
				   processor. This bit is typically used in 
				   conjunction with an Entity Association record.
				   (ENTITY_TYPE_LOGICAL) */
		entity_instance_num:7; /* [6:0] - Instance number for entity. 
				   (See section 39.1, System- and Device-relative
				   Entity Instance Values for more information)
				   00h-5Fh system-relative Entity Instance. 
				   The Entity Instance number must be unique 
				   for each different entity of the same type 
				   Entity ID in the system.
				   60h-7Fh device-relative Entity Instance.
				   The Entity Instance number must only be 
				   unique relative to the management controller
				   providing access to the Entity. */
#else
	uchar	entity_instance_num:7,
		entity_type:1;
#endif

#ifdef BF_MS_FIRST
	uchar	rsv1:1,		/* 11 Sensor Initialization - [7] - reserved. Write as 0b. */
		init_scanning:1,	/* [6] - Init Scanning 1b = enable scanning 
				   (this bit=1 implies that the sensor accepts
				   the ‘enable/disable scanning’ bit in the 
				   Set Sensor Event Enable command). */
		init_events:1,	/* [5] - Init Events 1b = enable events (per 
				   Sensor Event Message Control Support bits
				   in Sensor Capabilities field, and per the
				   Event Mask fields, below). */
		rsv2:1,		/* [4] - reserved. Write as 0b. */
		init_hysteresis:1, /* [3] - Init Hysteresis 1b = initialize sensor
				   hysteresis (per Sensor Hysteresis Support
				   bits in the Sensor Capabilities field, below). */
		init_sensor_type:1, /* [2] - Init Sensor Type 1b = initialize 
				   Sensor Type and Event / Reading Type code. */

		/* Sensor Default (power up) State 
		   -------------------------------
		   Reports how this sensor comes up on device power
		   up and hardware/cold reset.
		   The Initialization Agent does not use this
		   bit. This bit solely reports to software
		   how the sensor comes prior to being 
		   initialized by the Initialization Agent. */
		powerup_evt_generation:1, /* [1] - 0b = event generation disabled,
					     1b = event generation enabled */
		powerup_sensor_scanning:1; /* [0] - 0b = sensor scanning disabled,
					     1b = sensor scanning enabled */
#else
	uchar	powerup_sensor_scanning:1,
		powerup_evt_generation:1,
		init_sensor_type:1,
		init_hysteresis:1,
		rsv2:1,
		init_events:1,
		init_scanning:1,
		rsv1:1;
#endif
#ifdef BF_MS_FIRST
	uchar	ignore_sensor:1,	/* 12 Sensor Capabilities - [7] 
				   1b = Ignore sensor if Entity is not present 
				   or disabled.
				   0b = don’t ignore sensor */
		/* Sensor Auto Re-arm Support
		   --------------------------
		   Indicates whether the sensor requires manual rearming, or 
		   automatically rearms itself when the event clears. ‘manual’
		   implies that the get sensor event status and rearm sensor
		   events commands are supported */
		sensor_manual_support:1,	/* [6] - 0b = no (manual), 1b = yes (auto) */
				    
		/* Sensor Hysteresis Support
		   ------------------------- */
		sensor_hysteresis_support:2, /* [5:4] 
					00b = No hysteresis, or hysteresis built-in
				       	      but not specified.
					01b = hysteresis is readable.
					10b = hysteresis is readable and settable.
					11b = Fixed, unreadable, hysteresis. 
					      Hysteresis fields values implemented
					      in the sensor. */
					
		/* Sensor Threshold Access Support
		   ------------------------------- */
		sensor_threshold_access:2,	/* [3:2]
					00b = no thresholds.
					01b = thresholds are readable, per Reading
				              Mask, below.
					10b = thresholds are readable and settable
				              per Reading Mask and Settable Threshold 
					      Mask, respectively.
					11b = Fixed, unreadable, thresholds. Which
				              thresholds are supported is
					      reflected by the Reading Mask. The
					      threshold value fields report the
					      values that are ‘hard-coded’ in the
					      sensor. */
				     
		/* Sensor Event Message Control Support
		   ------------------------------------ 
		   Indicates whether this sensor generates Event Messages, and 
		   if so, what type of Event Message control is offered. */
		event_msg_control:2;	/* [1:0] - 
				   00b = per threshold/discrete-state event 
				   enable/disable control (implies that entire
				   sensor and global disable are also supported)
				   01b = entire sensor only (implies that global
				   disable is also supported)
				   10b = global disable only
				   11b = no events from sensor */
#else
	uchar	event_msg_control:2,
		sensor_threshold_access:2,
		sensor_hysteresis_support:2,
		sensor_manual_support:1,
		ignore_sensor:1;
#endif

	uchar sensor_type;	/* 13 Sensor Type - Code representing the sensor 
				   type. From Table 42-3, Sensor Type Codes.
				   E.g. Temperature, Voltage, Processor, etc. */
	uchar event_type_code;	/* 14 Event / Reading Type Code - Event/Reading 
				   Type Code. From Table 42-1, Event/Reading
				   Type Code Ranges. */
	short event_mask;	/* 15 & 16 Assertion Event Mask / Lower
				   Threshold Reading Mask - This field reports
				   the assertion event generation or threshold 
				   event generation capabilities for a discrete
				   or threshold-based sensor, respectively. 
				   This field is also used by the init agent to
				   enable assertion event generation when the ‘Init
				   Events’ bit in the Sensor Capabilities field 
				   is set and the Sensor Event Message Control
				   Support field indicates that the sensor has 
				   ‘per threshold/discrete state’ event enable
				   control. */
	/* Assertion Event Mask (for non- threshold-based sensors)
	   -------------------------------------------------------
	   The Event Mask bytes are a bit mask that specifies support for 15 successive
	   events starting with the event specified by Event/Reading Type Code. LS byte
	   first. 
	   [15] - reserved. Write as ‘0’.
	   [14:0] - Event offsets 14 through 0, respectively.
	   1b = assertion event can be generated by this sensor
	   Lower Threshold Reading Mask (for threshold-based sensors)
	   Indicates which lower threshold comparison status is returned via the Get Sensor
	   Reading command.
	   [15] - reserved. Write as 0b
	   [14] - 1b = Lower non-recoverable threshold comparison is returned
	   [13] - 1b = Lower critical threshold is comparison returned
	   [12] - 1b = Lower non-critical threshold is comparison returned
	   Threshold Assertion Event Mask (for threshold-based sensors)
	   [11] - 1b = assertion event for upper non-recoverable going high supported
	   [10] - 1b = assertion event for upper non-recoverable going low supported
	   [9] - 1b = assertion event for upper critical going high supported
	   [8] - 1b = assertion event for upper critical going low supported
	   [7] - 1b = assertion event for upper non-critical going high supported
	   [6] - 1b = assertion event for upper non-critical going low supported
	   [5] - 1b = assertion event for lower non-recoverable going high supported
	   [4] - 1b = assertion event for lower non-recoverable going low supported
	   [3] - 1b = assertion event for lower critical going high supported
	   [2] - 1b = assertion event for lower critical going low supported
	   [1] - 1b = assertion event for lower non-critical going high supported
	   [0] - 1b = assertion event for lower non-critical going low supported */
	/* Deassertion Event Mask / Upper Threshold Reading Mask
	   ----------------------------------------------------- */
	short deassertion_event_mask;	/* 17 & 18 - Deassertion Event Mask 
					   (for non- threshold-based sensors)
	The Event Mask bytes are a bit mask that specifies support for 15 successive
	events starting with the event specified by Event/Reading Type Code. LS byte
	first.
	[15] - reserved. Write as 0b
	[14:0] - Event offsets 14 through 0, respectively.
	1b = assertion event can be generated for this state.
	Upper Threshold Reading Mask (for threshold-based sensors)
	Indicates which upper threshold comparison status is returned via the Get Sensor
	Reading command.
	[15] - reserved. Write as 0b
	[14] - 1b = Upper non-recoverable threshold comparison is returned
	[13] - 1b = Upper critical threshold is comparison returned
	[12] - 1b = Upper non-critical threshold is comparison returned
	Threshold Deassertion Event Mask
	[11] - 1b = deassertion event for upper non-recoverable going high supported
	[10] - 1b = deassertion event for upper non-recoverable going low supported
	[9] - 1b = deassertion event for upper critical going high supported
	[8] - 1b = deassertion event for upper critical going low supported
	[7] - 1b = deassertion event for upper non-critical going high supported
	[6] - 1b = deassertion event for upper non-critical going low supported
	[5] - 1b = deassertion event for lower non-recoverable going high supported
	[4] - 1b = deassertion event for lower non-recoverable going low supported
	[3] - 1b = deassertion event for lower critical going high supported
	[2] - 1b = deassertion event for lower critical going low supported
	[1] - 1b = deassertion event for lower non-critical going high supported
	[0] - 1b = deassertion event for lower non-critical going low supported
	*/

	/* Discrete Reading Mask / Settable Threshold Mask, Readable Threshold Mask 
	   ------------------------------------------------------------------------ */
	short reading_mask;	/* 19 & 20 - Reading Mask (for non- threshold 
				   based sensors). Indicates what discrete 
	readings can be returned by this sensor, or, for threshold based 
	sensors, this indicates which thresholds are settable and which are
	readable. The Reading Mask bytes are a bit mask that specifies support for 15
	successive states starting with the value from Table 36-1, Event/Reading Type
	Code Ranges. LS byte first.
	[15] - reserved. Write as 0b
	[14:0] - state bits 0 through 14.
	1b = discrete state can be returned by this sensor.
	
	Settable Threshold Mask (for threshold-based sensors)
	-----------------------------------------------------
	Indicates which thresholds are settable via the Set Sensor Thresholds.
       	This mask also indicates which threshold values will be initialized if
       	the ‘Init Events’ bit is set. LS byte first.
	[15:14] - reserved. Write as 00b.
	[13] - 1b = Upper non-recoverable threshold is settable
	[12] - 1b = Upper critical threshold is settable
	[11] - 1b = Upper non-critical threshold is settable
	[10] - 1b = Lower non-recoverable threshold is settable
	[9] - 1b = Lower critical threshold is settable
	[8] - 1b = Lower non-critical threshold is settable
	
	Readable Threshold Mask (for threshold-based sensors)
	-----------------------------------------------------
	Indicates which thresholds are readable via the Get Sensor Thresholds
	command.
	[7:6] - reserved. Write as 00b.
	[5] - 1b = Upper non-recoverable threshold is readable
	[4] - 1b = Upper critical threshold is readable
	[3] - 1b = Upper non-critical threshold is readable
	[2] - 1b = Lower non-recoverable threshold is readable
	[1] - 1b = Lower critical threshold is readable
	[0] - 1b = Lower non-critical threshold is readable */
#ifdef BF_MS_FIRST
	uchar	rsv3:2,			/* reserved */
		rate_unit:3,		/* [5:3] - Rate unit
					   000b = none
					   001b = per µS
					   010b = per ms
					   011b = per s
					   100b = per minute
					   101b = per hour
					   110b = per day
					   111b = reserved */
		modifier_unit:2,		/* [2:1] - Modifier unit
					   00b = none
					   01b = Basic Unit / Modifier Unit
					   10b = Basic Unit * Modifier Unit
					   11b = reserved */
		percentage:1;		/* [0] - Percentage 0b = no, 1b = yes */
#else
	uchar	percentage:1,
		modifier_unit:2,
		rate_unit:3,
		rsv3:2;
#endif
	uchar	sensor_units2;	/* 22 Sensor Units 2 - Base Unit  
				   [7:0] - Units Type code: See Table 43-15,
				   Sensor Unit Type Codes. */
	uchar	sensor_units3;	/* 23 Sensor Units 3 - Modifier Unit
				   [7:0] - Units Type code, 00h if unused. */
	/* 24 Sensor Record Sharing, Sensor Direction */
#ifdef BF_MS_FIRST
	uchar	sensor_direction:2,	/* [7:6] - Sensor Direction. Indicates
			whether the sensor is monitoring an input or output
		       	relative to the given Entity. E.g. if the sensor is
		       	monitoring a current, this can be used to specify whether
		       	it is an input voltage or an output voltage.
			  00b = unspecified / not applicable
			  01b = input
			  10b = output
			  11b = reserved */
		id_str_mod_type:2, /* [5:4] - ID String Instance Modifier Type (The
			instance modifier is a character(s) that software can
		       	append to the end of the ID String. This field selects
		       	whether the appended character(s) will be numeric or 
			alpha. The Instance Modified Offset field, below, selects
		       	the starting value for the character.)
			  00b = numeric
			  01b = alpha */
		share_count:4;	/* [3:0] - Share count (number of sensors sharing
			this record). Sensor numbers sharing this record are 
			sequential starting with the sensor number specified by
		       	the Sensor Number field for this record. E.g. if the 
			starting sensor number was 10, and the share count was 3,
		       	then sensors 10, 11, and 12 would share this record. */
#else
	uchar	share_count:4,
		id_str_mod_type:2,
		sensor_direction:2;
#endif
	/* 25 Entity Instance Sharing */
#ifdef BF_MS_FIRST
	uchar	entity_inst_same:1,	/* [7] - 0b = Entity Instance same for all shared records
					   1b = Entity Instance increments for each shared record */
		id_str_mod_offset:7;	/* [6:0] - ID String Instance Modifier Offset
			Multiple Discrete sensors can share the same sensor data 
			record. The ID String Instance Modifier and Modifier 
			Offset are used to modify the Sensor ID String as follows:
			Suppose sensor ID is “Temp ” for ‘Temperature Sensor’,
		       	share count = 3, ID string instance modifier = numeric,
		       	instance modifier offset = 5 - then the sensors could be
		       	identified as: Temp 5, Temp 6, Temp 7
			If the modifier = alpha, offset=0 corresponds to ‘A’,
		       	offset=25 corresponds to ‘Z’, and offset = 26 corresponds
		       	to ‘AA’, thus, for offset=26 the sensors could be identified
			as: Temp AA, Temp AB, Temp AC (alpha characters are 
			considered to be base 26 for ASCII) */
#else
	uchar	id_str_mod_offset:7,
		entity_inst_same:1;
#endif
	uchar	positive_hysteresis;	/* 26 Positive-going Threshold Hysteresis value
			Positive hysteresis is defined as the unsigned number of 
			counts that are subtracted from the raw threshold values
		       	to create the ‘re-arm’ point for all positive-going thresholds
		       	on the sensor. 0 indicates that there is no hysteresis on
			positive-going thresholds for this sensor. Hysteresis values
		       	are given as raw counts. That is, to find the degree of
		       	hysteresis in units, the value must be converted using the
		       	‘y=Mx+B’ formula. Note: Cannot use shared record if sensors
		       	require individual hysteresis settings. */
	uchar	negative_hysteresis;	/* 27 Negative-going Threshold Hysteresis value
			Negative hysteresis is defined as the unsigned number of
		       	counts that are added to the raw threshold value to create
		       	the ‘re-arm’ point for all negative-going thresholds on the
		       	sensor. 0 indicates that there is no hysteresis on negative-going
			thresholds for this sensor. Note: Cannot use shared record
		       	if sensors require individual hysteresis settings. */
	uchar 	rsv4;		/* 28 reserved Write as 00h. */
	uchar 	rsv5;		/* 29 reserved. Write as 00h. */
	uchar	rsv6;		/* 30 reserved. Write as 00h. */
	uchar	oem;	/* 31 Reserved for OEM use. */
	uchar	id_str_typ_len;	/* 32 Sensor ID String Type/Length Code, per 
				   Section 43.15, Type/Length Byte Format. */
	uchar	sensor_id_str[16];	/* 33:+N Sensor ID String bytes. Only present
			if non-zero length in Type/Length field. 16 bytes, maximum. */
} COMPACT_SENSOR_RECORD;

	
/*
 * SDR Type 12h - Management Controller Device Locator Record
 */
typedef struct mgmt_ctrl_dev_locator_record {
	// RECORD HEADER
	uchar	record_id[2];	/* The Record ID is used by the Sensor Data Repository 
				   device for record organization and access. This may 
				   not actually be stored, but may be calculated when 
				   records are accessed. */
	uchar	sdr_version;	/* SDR Version. Version of the Sensor Model specification
				   that this record is compatible with. 51h for this 
				   specification. This is BCD encoded with bits 7:4 holding
				   the Least Significant digit of the revision and 
				   bits 3:0 holding the Most Significant bits. E.g. 51h
				   corresponds to “1.5”. */
	uchar	record_type;	/* Record Type Number = 12h, Management Controller Locator */
	uchar	record_len;	/* Record Length. Number of remaining record bytes following. */
	// RECORD KEY BYTES
	uchar	dev_slave_addr;	/* Device Slave Address. 
				   [7:1] - 7-bit I2C Slave Address of device on channel.
				   [0] - reserved. */
	uchar	ch_num;		/* Channel Number.
				   [7:4] - reserved
				   [3:0] - Channel number for the channel that the 
				   management controller is on. Use 0h for the primary BMC.
				   (New byte for IPMI v1.5. Note this addition causes some
				   of the following byte offsets to be pushed down when compared
				   to the IPMI v1.0 version of this record.) */
	// RECORD BODY BYTES
	// Power State Notification.
#ifdef BF_MS_FIRST
	uchar	acpi_sys_pwr_st_notify_req:1,	/* [7] - 1b = ACPI System Power State notification required (by system s/w)
						   0b = no ACPI System Power State notification required */
		acpi_dev_pwr_st_notify_req:1,	/* [6] - 1b = ACPI Device Power State notification required (by system s/w)
						   0b = no ACPI Device Power State notification required */
		rsv1:1,	/* [5] - For backward compatibility, this bit does not apply to the BMC, 
			   and should be written as 0b. 
			   0b = Dynamic controller - controller may or may not 
			   be present. Software should not generate error status
			   if this controller is not present.
			   1b = Static controller - this controller is expected
			   to be present in the system at all times. Software may
			   generate an error status if controller is not detected. */
		rsv2:1,	/* [4] - reserved */
	// Global Initialization
		ctrl_logs_init_errs:1,	/* [3] - 1b = Controller logs Initialization 
					   Agent errors (only applicable to Management
					   Controller that implements the initialization
					   agent function. Set to 0b otherwise.) */
		log_init_agent_errs:1,	/* [2] - 1b = Log Initialization Agent errors 
					   accessing this controller (this directs the
					   initialization agent to log any failures
					   setting the Event Receiver) */
		ctrl_init:2;		/* [1:0] 
			00b = Enable event message generation from controller 
			(Init agent will set Event Receiver address into controller)
			01b = Disable event message generation from controller 
			(Init agent will set Event Receiver to FFh). This provides
		       	a temporary fix for a broken controller that floods the 
			system with events. It can also be used for development
		       	/ debug purposes.
			10b = Do not initialize controller. This selection is for
		       	development / debug support.
			11b = reserved. */
#else
	uchar	ctrl_init:2,
		log_init_agent_errs:1,
		ctrl_logs_init_errs:1,
		rsv2:1,
		rsv1:1,
		acpi_dev_pwr_st_notify_req:1,
		acpi_sys_pwr_st_notify_req:1;
#endif
	// Device Capabilities
#ifdef BF_MS_FIRST
	uchar	dev_sup_chassis:1, 	/* [7] - 1b = Chassis Device. (device 
					   functions as chassis device, per ICMB 
					   spec) */
		dev_sup_bridge:1, 	/* [6] - 1b = Bridge (Controller responds 
					   to Bridge NetFn commands) */
		dev_sup_ipmb_evt_gen:1, /* [5] - 1b = IPMB Event Generator (device 
					   generates event messages on IPMB) */
		dev_sup_ipmb_evt_rcv:1, /* [4] - 1b = IPMB Event Receiver (device
					   accepts event messages from IPMB) */
		dev_sup_fru_inv:1, 	/* [3] - 1b = FRU Inventory Device 
					   (accepts FRU commands to FRU Device #0
					   at LUN 00b) */
		dev_sup_sel:1, 		/* [2] - 1b = SEL Device (provides interface
					   to SEL) */
		dev_sup_sdr_rep:1,	/* [1] - 1b = SDR Repository Device (For 
					   BMC, indicates BMC provides interface
					   to 1b = SDR Repository. For other 
					   controller, indicates controller accepts
					   Device SDR commands) */
		dev_sup_sensor:1; 	/* [0] - 1b = Sensor Device (device accepts
					   sensor commands) See Table 37-11, 
					   IPMB/I2C Device Type Codes */
#else
	uchar	dev_sup_sensor:1,
		dev_sup_sdr_rep:1,
		dev_sup_sel:1,
		dev_sup_fru_inv:1,
		dev_sup_ipmb_evt_rcv:1,
		dev_sup_ipmb_evt_gen:1,
		dev_sup_bridge:1,
		dev_sup_chassis:1;
#endif			       
	uchar	rsv[3];
	uchar	entity_id;		/* Entity ID for the FRU associated with 
					   this device. 00h if not specified. If 
					   device supports FRU commands at LUN 00b,
					   this Entity ID applies to both the IPM
					   device and the FRU information accessed
					   via LUN 00b. */
	uchar	entity_instance;	/* Entity Instance. Instance number for entity. */
	uchar	oem;			/* Reserved for OEM use. */
	uchar	dev_id_typ_len;		/* Device ID String Type/Length code per
					   Section 43.15, Type/Length Byte Format.
					   [7:6] 00 = Unicode
						 01 = BCD plus (see below)
						 10 = 6-bit ASCII, packed
						 11 = 8-bit ASCII + Latin 1.
					   [5] reserved.
					   [4:0] length of following data, in characters. 
					   0 indicates ‘none following’. 11111b = reserved. */				   
	uchar	dev_id_str[15];		/* 17:+N Device ID String N Short ‘ID’
					   string for the device. 16 bytes, maximum. */
} MGMT_CTRL_DEV_LOCATOR_RECORD;


/*======================================================================*/
/*
 * 	Event/Reading Type Code Ranges
 * 	(Table 42-1)
 * 
 */
#define EVT_TYPE_CODE_UNSPECIFIED	0x00	/* Event/Reading Type unspecified. */
#define EVT_TYPE_CODE_THRESHOLD		0x01	/* Threshold-based. Indicates a sensor
						   that utilizes values that represent
						   discrete threshold states in sensor
						   access and/or events. The Event/Reading
						   event offsets for the different threshold
						   states are given in Table 42-2, Generic
						   Event/Reading Type Codes */
#define EVT_TYPE_CODE_GENERIC_LO	0x02 	/* Generic Discrete. Indicates a sensor that
						   utilizes an Event/Reading Type code & State
					       	   bit positions / event offsets from one of
					       	   the sets specified for Discrete or ‘digital’
					       	   Discrete Event/Reading class in Table 42-2,
					       	   Generic Event/Reading Type Codes */			
#define EVT_TYPE_CODE_GENERIC_HI	0x0C
#define EVT_TYPE_CODE_SENSOR_SPECIFIC	0x6F	/* Sensor-specific Discrete. Indicates that
						   the discrete state information is specific
						   to the sensor type. State bit positions /
						   event offsets for a particular sensor type
						   are specified in the ‘sensor-specific offset’
						   column in Table 42-3, Sensor Type Codes, below. */
#define EVT_TYPE_CODE_OEM_LO		0x70	/* OEM Discrete. Indicates that the discrete 
						   state information is specific to the OEM
						   identified by the Manufacturer ID for the
						   IPM device that is providing access to the
						   sensor. */
#define EVT_TYPE_CODE_OEM_HI		0x7F 

/*======================================================================*/
/*
 * 	Entity ID Codes
 * 	(Table 43-13)
 * 
 */

#define ENTITY_ID_UNSPECIFIED		0x00	/* unspecified */
#define ENTITY_ID_OTHER			0x01	/* other */
#define ENTITY_ID_UNKNOWN		0x02	/* unknown (unspecified) */
#define ENTITY_ID_PROCESSOR		0x03	/* processor */
#define ENTITY_ID_DISK			0x04	/* disk or disk bay */
#define ENTITY_ID_PERIPHERAL_BAY	0x05	/* peripheral bay */
#define ENTITY_ID_SYS_MGMT_MODULE	0x06	/* system management module */
#define ENTITY_ID_SYSTEM_BOARD		0x07	/* system board (main system board, may
						   also be a processor board and/or internal
						   expansion board) */
#define ENTITY_ID_MEM_MODULE		0x08	/* memory module (board holding memory devices) */
#define ENTITY_ID_PROCESSOR_MODULE	0X09	/* processor module (holds processors,
						   use this designation when processors
						   are not mounted on system board) */
#define ENTITY_ID_POWER_SUPPLY		0x0A	/* power supply (DMI refers to this
						   as a “power unit”, but it’s used
						   to represent a power supply).
						   Use this value for the main power
						   supply (supplies) for the system. */
#define ENTITY_ID_ADD_IN_CARD		0x0B	/* add-in card */
#define ENTITY_ID_FRONT_PANEL_BOARD	0x0C	/* front panel board (control panel) */
#define ENTITY_ID_BACK_PANEL_BOARD	0x0D	/* back panel board */
#define ENTITY_ID_POWER_SYSTEM_BOARD	0x0E	/* power system board */
#define ENTITY_ID_DRIVE_BACKPLANE	0x0F	/* drive backplane */
#define ENTITY_ID_INT_EXPANSION_BOARD	0x10	/* system internal expansion board 
						   (contains expansion slots). */
#define ENTITY_ID_OTHER_SYS_BOARD	0x11	/* Other system board (part of board set) */
#define ENTITY_ID_PROCESSOR_BOARD	0x12	/* processor board (holds 1 or more
						   processors - includes boards that
						   hold SECC modules) */
#define ENTITY_ID_POWER_UNIT		0x13	/* power unit / power domain - This
						   Entity ID is typically used as a
						   pre-defined logical entity for
						   grouping power supplies. */
#define ENTITY_ID_POWER_MODULE		0x14	/* power module / DC-to-DC converter
						   - Use this value for internal converters.
						   Note: You should use Entity ID 10
						   (power supply) for the main power
						   supply even if the main supply is
						   a DC-to-DC converter, e.g. gets
						   external power from a -48 DC source. */
#define ENTITY_ID_POWER_MGMT_BOARD	0x15	/* power management / power distribution board */
#define ENTITY_ID_CHASSIS_BACKPLANE	0x16	/* chassis back panel board */
#define ENTITY_ID_SYSTEM_CHASSIS	0x17	/* system chassis */
#define ENTITY_ID_SUB_CHASSIS		0x18	/* sub-chassis */
#define ENTITY_ID_OTHER_CHASSIS_BOARD	0x19	/* Other chassis board */
#define ENTITY_ID_DISK_DRIVE_BAY	0x1A	/* Disk Drive Bay */
#define ENTITY_ID_PERIPHERAL_BAY2	0x1B	/* Peripheral Bay */
#define ENTITY_ID_DEVICE_BAY		0x1C	/* Device Bay */
#define ENTITY_ID_FAN			0x1D	/* fan / cooling device */
#define ENTITY_ID_COOLING_UNIT		0x1E	/* cooling unit - This Entity ID can
						   be used as a pre-defined logical entity
						   for grouping fans or other cooling devices. */
#define ENTITY_ID_CABLE_INTERCONNECT	0x1F	/* cable / interconnect */
#define ENTITY_ID_MEMORY_DEVICE		0x20	/* memory device -This Entity ID 
						   should be used for replaceable
						   memory devices, e.g. DIMM/SIMM. 
						   It is recommended that Entity IDs
						   not be used for individual non-replaceable
						   memory devices. Rather, monitoring and
						   error reporting should be associated
						   with the FRU [e.g. memory card] holding
						   the memory. */
#define ENTITY_ID_SYS_MGMT_SOFTWARE	0x21	/* System Management Software */
#define ENTITY_ID_BIOS			0x22	/* BIOS */
#define ENTITY_ID_OPERATING_SYSTEM	0x23	/* Operating System */
#define ENTITY_ID_SYSTEM_BUS		0x24	/* system bus */
#define ENTITY_ID_GROUP			0x25	/* Group - This is a logical entity
						   for use with Entity Association records.
						   It is provided to allow an Entity association
						   record to define a grouping of entities when
						   there is no appropriate pre-defined entity
						   for the container entity. This Entity should
						   not be used as a physical entity. */
#define ENTITY_ID_REMOTE_MGMT_COMM_DEV	0x26	/* Remote (Out of Band) Management Communication Device */
#define ENTITY_ID_EXT_ENVIRONMENT	0x27	/* External Environment - This Entity 
						   ID can be used to identify the environment
						   outside the system chassis. For example, a
						   system may have a temperature sensor that
						   monitors the temperature “outside the box”.
						   Such a temperature sensor can be associated
						   with an External Environment entity.
						   This value will typically be used as a single
						   instance physical entity. However, the Entity
						   Instance value can be used to denote a difference
						   in regions of the external environment. For 
						   example, the region around the front of a 
						   chassis may be considered to be different from
						   the region around the back, in which case it would
						   be reasonable to have two different instances of
						   the External Environment entity. */
#define ENTITY_ID_BATTERY		0x28	/* battery */
#define ENTITY_ID_PROCESSING_BLADE	0x29	/* Processing blade (a blade module that 
						   contains processor, memory, and I/O
						   connections that enable it to operate as
						   a processing entity) */
#define ENTITY_ID_CONNECTIVITY_SWITCH	0x2A	/* Connectivity switch (a blade module that
						   provides the fabric or network connection
						   for one or more processing blades or modules) */
#define ENTITY_ID_PROCESSOR_MEM_MODULE	0x2B	/* Processor/memory module (processor and 
						   memory together on a module) */
#define ENTITY_ID_IO_MODULE		0x2C	/* I/O module (a module that contains the
						   main elements of an I/O interface) */
#define ENTITY_ID_PROCESSOR_IO_MODULE	0x2D	/* Processor/ IO module (a module that 
						   contains the main elements of an I/O interface) */
#define ENTITY_ID_MGMT_COTROLLER_FW	0x2E	/* Management Controller Firmware (Represents
						   firmware or software running on a management
						   controller) */
#define ENTITY_ID_IPMI_CHANNEL		0x2F	/* IPMI Channel - This Entity ID enables
						   associating sensors with the IPMI
						   communication channels - for example a
						   Redundancy sensor could be used to report
						   redundancy status for a channel that is
						   composed of multiple physical links. By
						   convention, the Entity Instance corresponds
						   to the channel number. */
#define ENTITY_ID_PCI_BUS		0x30	/* PCI Bus */
#define ENTITY_ID_PCI_EXPRESS_BUS	0x31	/* PCI Express™ Bus */
#define ENTITY_ID_SCSI_BUS		0x32	/* SCSI Bus (parallel) */
#define ENTITY_ID_SATA_SAS_BUS		0x33	/* SATA / SAS bus */
#define ENTITY_ID_PROCESSOR_FSB		0x34	/* Processor / front-side bus */
/* - 90h-AFh Chassis-specific Entities. These IDs are system specific and can be 
 *   assigned by the chassis provider. 
 * - B0h-CFh Board-set specific Entities. These IDs are system specific and can be
 *   assigned by the Board-set provider.
 * - D0h-FFh OEM System Integrator defined. These IDs are system specific and can be
 *   assigned by the system integrator, or OEM.
 * - all other values reserved
 */

/*======================================================================*/
/*
 * 	Sensor Unit Type Codes (abridged)
 * 	(Table 43-15)
 * 
 */
#define SENSOR_UNIT_UNSPECIFIED		0x0
#define SENSOR_UNIT_DEGREES_CELSIUS	0x1
#define SENSOR_UNIT_DEGREES_FAHRENHEIT	0x2
#define SENSOR_UNIT_DEGREES_KELVIN	0x3
#define SENSOR_UNIT_VOLTS		0x4
#define SENSOR_UNIT_AMPS		0x5
#define SENSOR_UNIT_WATTS		0x6
#define SENSOR_UNIT_JOULES		0x7
#define SENSOR_UNIT_HZ			0x19
#define SENSOR_UNIT_MICROSECOND		0x20
#define SENSOR_UNIT_MILLISECOND		0x21
#define SENSOR_UNIT_SECOND		0x22
#define SENSOR_UNIT_MINUTE		0x23
#define SENSOR_UNIT_HOUR		0x24
#define SENSOR_UNIT_DAY			0x25
#define SENSOR_UNIT_WEEK		0x26

/*======================================================================*/
/*
 *    SEL Device Commands
 *
 	Command				NetFn 	CMD 	BMC 	SHM 	IPMC
	--------------------------------------------------------------------
 *    	
 *    Using NETFN_NVSTORE_REQ/NETFN_NVSTORE_RESP
 */
/*======================================================================*/

#define IPMI_STO_CMD_GET_SEL_INFO		0x40	/* Get SEL Info */
#define IPMI_STO_CMD_GET_SEL_ALLOCATION_INFO	0x41	/* Get SEL Allocation Info */
#define IPMI_STO_CMD_RESERVE_SEL		0x42	/* Reserve SEL */
#define IPMI_STO_CMD_GET_SEL_ENTRY		0x43	/* Get SEL Entry */
#define IPMI_STO_CMD_ADD_SEL_ENTRY		0x44	/* Add SEL Entry */
#define IPMI_STO_CMD_PARTIAL_ADD_SEL_ENTRY	0x45	/* Partial Add SEL Entry */
#define IPMI_STO_CMD_DELETE_SEL_ENTRY		0x46	/* Delete SEL Entry */
#define IPMI_STO_CMD_CLEAR_SEL			0x47	/* Clear SEL */
#define IPMI_STO_CMD_GET_SEL_TIME		0x48	/* Get SEL Time */
#define IPMI_STO_CMD_SET_SEL_TIME		0x49	/* Set SEL Time */
#define IPMI_STO_CMD_GET_AUX_LOG_STATUS		0x5A	/* Get Auxiliary Log Status */
#define IPMI_STO_CMD_SET_AUX_LOG_STATUS		0x5B	/* Set Auxiliary Log Status */
/*----------------------------------------------------------------------*/
/*			Get SEL Info command				*/
/*----------------------------------------------------------------------*/

typedef struct get_sel_info_cmd_resp {
	uchar completion_code;	/* Completion Code 81h = cannot execute command,
				   SEL erase in progress */
	uchar sel_version;	/* SEL Version - version number of the SEL 
				   command set for this SEL Device.
				   51h for this specification.
				   (BCD encoded).BCD encoded with bits 7:4 
				   holding the Least Significant digit of the 
				   revision and bits 3:0 holding the Most 
				   Significant bits. */
	uchar entries_lsb;	/* Entries LS Byte - number of log entries in SEL, LS Byte */
	uchar entries_msb;	/* Entries MS Byte - number of log entries in SEL, MS Byte */
	uchar free_space[2];	/* 5:6 Free Space in bytes, LS Byte first. 
				   FFFFh indicates 65535 or more bytes of
				   free space are available. */
	uchar most_recent_addition[4];	/* 7:10 Most recent addition timestamp. 
				   LS byte first.
				   Returns FFFF_FFFFh if no SEL entries have 
				   ever been made or if a component update or 
				   error caused the retained value to be lost. */
	uchar most_recent_erase_timestamp[4];	/* 11:14 Most recent erase timestamp. 
				   Last time that one or more entries were
				   deleted from the log. LS byte first. */
	uchar operation_support;	/* 15 Operation Support
				   [7] - Overflow Flag. 1 = Events have been 
				   dropped due to lack of space in the SEL.
				   [6:4] - reserved. Write as 000
				   [3] - 1b = Delete SEL command supported
				   [2] - 1b = Partial Add SEL Entry command supported
				   [1] - 1b = Reserve SEL command supported
				   [0] - 1b = Get SEL Allocation Information command supported */
} GET_SEL_NFO_CMD_RESP;

/*----------------------------------------------------------------------*/
/*			Get SEL Entry command				*/
/*----------------------------------------------------------------------*/

typedef struct get_sel_entry_cmd_req {
	uchar	command;
	uchar reservation_id[2];	/* 1:2 Reservation ID, LS Byte first. 
				   Only required for partial Get. Use 0000h 
				   otherwise. The reservation ID should be set
				   to 0000h for implementations that don’t 
				   implement the Reserve SEL command. */
	uchar sel_record_id[2];	/* 3:4 SEL Record ID, LS Byte first.
				   0000h = GET FIRST ENTRY
				   FFFFh = GET LAST ENTRY */
	uchar offset_into_record;	/* Offset into record */
	uchar bytes_to_read;	/* Bytes to read. FFh means read entire record. */
} GET_SEL_ENTRY_CMD_REQ;

typedef struct get_sel_entry_cmd_resp {
	uchar completion_code;	/* Response Data 1 Completion Code
				   Return an error completion code if the SEL is empty.
				   81h = cannot execute command, SEL erase in progress. */
	uchar next_sel_record_id[2];	/* 2:3 Next SEL Record ID, LS Byte first
				   (return FFFFh if the record just returned 
				   is the last record.)
				   Note: FFFFh is not allowed as the record ID
				   for an actual record. I.e. the Record ID in 
				   the Record Data for the last record should 
				   not be FFFFh. */
	uchar record_data[20];	/* 4:N Record Data, 16 bytes for entire record */
} GET_SEL_ENTRY_CMD_RESP;

/*----------------------------------------------------------------------*/
/*			Add SEL Entry command				*/
/*----------------------------------------------------------------------*/

typedef struct add_sel_entry_cmd_req {
	uchar	command;
	uchar record_data[16];	/* 1:16 Record Data, 16 bytes. Refer to 
				   section 32, SEL Record Formats */
} ADD_SEL_ENTRY_CMD_REQ;

typedef struct add_sel_entry_cmd_resp {
       	uchar completion_code;	/* Completion Code. Generic, plus 
				   following command specific:
				   80h = operation not supported for this Record Type
				   81h = cannot execute command, SEL erase in progress */
	uchar record_id[2];	/* 2:3 Record ID for added record, LS Byte first. */
} ADD_SEL_ENTRY_CMD_RESP;

/*----------------------------------------------------------------------*/
/*			Partial Add SEL Entry command			*/
/*----------------------------------------------------------------------*/

typedef struct partial_add_sel_entry_cmd_req {
	uchar	command;
	uchar reservation_id[2];	/* 1:2 Reservation ID, LS Byte first. 
					   Only required for partial add. Use
					   0000h for Reservation ID otherwise.
					   The reservation ID should be set to 
					   0000h for implementations that don’t
					   implement the Reserve SEL command. */
	uchar record_id[2];	/* 3:4 Record ID, LS Byte first. Used when 
				   continuing a partial add (nonzero offset 
				   into record). Use 0000h for Record ID 
				   otherwise. */
	uchar offset_into_record;	/* Offset into record. */
	uchar in_progress;	/* In progress.
				   [7:4] - reserved
				   [3:0] - in progress
				   0h = partial add in progress.
				   1h = last record data being transferred 
				        with this request */
	uchar sel_record_data[20];	/* 7:N SEL Record Data */
} PARTIAL_ADD_SEL_ENTRY_CMD_REQ;

typedef struct partial_add_sel_entry_cmd_resp {
	uchar completion_code;	/* Completion Code
				   80h = Record rejected due to mismatch between
				   record length in header data and number of 
				   bytes written. (Verifying the length is an 
				   optional operation for the management 
				   controller)
				   81h = cannot execute command, SEL erase
				   in progress. */
	uchar record_id[2];	/* 2:3 Record ID for added record, LS Byte first. */
} PARTIAL_ADD_SEL_ENTRY_CMD_RESP;

/*----------------------------------------------------------------------*/
/*			Clear SEL command				*/
/*----------------------------------------------------------------------*/

typedef struct clear_sel_cmd_req {
	uchar	command;
	uchar reservation_id[2];	/* 1:2 Reservation ID, LS Byte first.
				   The reservation ID should be set to 0000h 
				   for implementations that don’t implement 
				   the Reserve SEL command. */
	uchar c;		/* 3 ‘C’ (43h) */
	uchar l;		/* 4 ‘L’ (4Ch) */
	uchar r;		/* 5 ‘R’ (52h) */
	uchar erasure_op;	/* 6 AAh = initiate erase.
				   00h = get erasure status. */
} CLEAR_SEL_CMD_REQ;

typedef struct clear_sel_cmd_resp {
	uchar completion_code;	/* Completion Code */
	uchar erasure_progress;	/* 2 Erasure progress.
				   [7:4] - reserved
				   [3:0] - erasure progress
				   0h = erasure in progress.
				   1h = erase completed. */
} CLEAR_SEL_CMD_RESP;

/*----------------------------------------------------------------------*/
/*			Get SEL Time command				*/
/*----------------------------------------------------------------------*/

typedef struct get_sel_time_cmd_resp {
	uchar completion_code;	/* Completion Code */
	uchar preset_timestamp_clock_reading[4];
	/* 2:5 Present Timestamp clock reading. LS byte first. See Section 37,
	   Timestamp Format. */
} GET_SEL_TIME_CMD_RESP;

/*----------------------------------------------------------------------*/
/*			Set SEL Time command				*/
/*----------------------------------------------------------------------*/
typedef struct set_sel_time_cmd_req {
	uchar	command;
	uchar data[4];	/* 1:4 Time in four-byte format. LS byte first. 
			   See Section 37, Timestamp Format. */
} SET_SEL_TIME_CMD_REQ;

typedef struct set_sel_time_cmd_resp {
	uchar completion_code;
} SET_SEL_TIME_CMD_RESP;

/*======================================================================*/
/*    LAN Device Commands
 *  	
	Command				NetFn 	CMD 	BMC 	SHM 	IPMC
	--------------------------------------------------------------------
	Set LAN Configuration Parameters Transport 01h	O/M 	M 	O/M
	Get LAN Configuration Parameters Transport 02h 	O/M 	M 	O/M
	Suspend BMC ARPs 		Transport 03h	O/M 	O/M 	O/M
	Get IP/UDP/RMCP Statistics 	Transport 04h 	O 	O 	O
 */
/*======================================================================*/


/*======================================================================*/
/*    Serial/Modem Device Commands
 *    
	Command				NetFn 	CMD 	BMC 	SHM 	IPMC
	--------------------------------------------------------------------
	Set Serial/Modem Configuration 	Transport 10h 	O/M 	O/M 	O/M
	Get Serial/Modem Configuration 	Transport 11h 	O/M 	O/M 	O/M
	Set Serial/Modem Mux 		Transport 12h 	O 	O 	O
	Get TAP Response Codes 		Transport 13h 	O 	O 	O
	Set PPP UDP Proxy Transmit Data Transport 14h 	O 	O 	O
	Get PPP UDP Proxy Transmit Data Transport 15h 	O 	O 	O
	Send PPP UDP Proxy Packet 	Transport 16h 	O 	O 	O
	Get PPP UDP Proxy Receive Data 	Transport 17h 	O 	O 	O
	Serial/Modem Connection Active 	Transport 18h 	O/M 	O/M 	O/M
	Callback 			Transport 19h 	O 	O 	O
	Set User Callback Options 	Transport 1Ah 	O 	O 	O
	Get User Callback Options 	Transport 1Bh 	O 	O 	O

 */
/*======================================================================*/


/*======================================================================*/
/*    Bridge management Commands
 *  	
	Command				NetFn 	CMD 	BMC 	SHM 	IPMC
	--------------------------------------------------------------------
	Get Bridge State 		Bridge 	00h 	O/M 	O/M 	O
	Set Bridge State 		Bridge 	01h 	O/M 	O/M 	O
	Get ICMB Address 		Bridge 	02h 	O/M 	O/M 	O
	Set ICMB Address 		Bridge 	03h 	O/M 	O/M 	O
	Set Bridge Proxy Address 	Bridge 	04h 	O/M 	O/M 	O
	Get Bridge Statistics 		Bridge 	05h 	O/M 	O/M 	O
	Get ICMB Capabilities 		Bridge 	06h 	O/M 	O/M 	O
	Clear Bridge Statistics 	Bridge 	08h 	O/M 	O/M 	O
	Get Bridge Proxy Address 	Bridge 	09h 	O/M 	O/M 	O
	Get ICMB Connector Info 	Bridge 	0Ah 	O/M 	O/M 	O
	Get ICMB Connection ID 		Bridge 	0Bh 	O/M 	O/M 	O
	Send ICMB Connection ID 	Bridge 	0Ch 	O/M 	O/M 	O
 */
/*======================================================================*/


/*======================================================================*/
/*    Discovery Commands (ICMB)
 *  	
	Command				NetFn 	CMD 	BMC 	SHM 	IPMC
	--------------------------------------------------------------------
	Prepare For Discovery 		Bridge 	10h 	O/M 	O/M 	O
	Get Addresses 			Bridge 	11h 	O/M 	O/M 	O
	Set Discovered 			Bridge 	12h 	O/M 	O/M 	O
	Get Chassis Device ID 		Bridge 	13h 	O/M 	O/M 	O
	Set Chassis Device ID 		Bridge 	14h 	O/M 	O/M 	O
 */
/*======================================================================*/


/*======================================================================*/
/*    Bridging Commands (ICMB)
 *  	
	Command				NetFn 	CMD 	BMC 	SHM 	IPMC
	--------------------------------------------------------------------
	Bridge Request 			Bridge 	20h 	O/M 	O/M 	O
	Bridge Message 			Bridge 	21h 	O/M 	O/M 	O
 */
/*======================================================================*/


/*======================================================================*/
/*    Event Commands(ICMB)
 *  	
	Command				NetFn 	CMD 	BMC 	SHM 	IPMC
	--------------------------------------------------------------------
	Get Event Count 		Bridge 	30h 	O/M 	O/M 	O
	Set Event Destination 		Bridge 	31h 	O/M 	O/M 	O
	Set Event Reception State 	Bridge 	32h 	O/M 	O/M 	O
	Send ICMB Event Message 	Bridge 	33h 	O/M 	O/M 	O
	Get Event Destination 		Bridge 	34h 	O/M 	O/M 	O
	Get Event Reception State 	Bridge 	35h 	O/M 	O/M 	O
 */
/*======================================================================*/


/*======================================================================*/
/*    OEM Commands for Bridge
 *  	
	Command				NetFn 	CMD 	BMC 	SHM 	IPMC
	--------------------------------------------------------------------
	OEM Commands 			Bridge C0h-FEh 	O/M 	O/M 	O
*/
/*======================================================================*/


/*======================================================================*/
/*    Other Bridge Commands
 *  	
	Command				NetFn 	CMD 	BMC 	SHM 	IPMC
	--------------------------------------------------------------------
	Error Report 			Bridge 	FFh 	O/M 	O/M 	O
 */
/*======================================================================*/


/*======================================================================*/
/*    AdvancedTCA® and PICMG® specific request commands
 *

	Command				NetFn 	CMD 	BMC 	SHM 	IPMC
	--------------------------------------------------------------------
	Get PICMG Properties 		PICMG 	00h 		M 	M
	Get Address Info 		PICMG 	01h 		M 	M
	Get Shelf Address Info 		PICMG 	02h 		M 	O
	Set Shelf Address Info 		PICMG 	03h 		M 	O
	FRU Control 			PICMG 	04h 		M 	M
	Get FRU LED Properties 		PICMG 	05h 		M 	M
	Get LED Color Capabilities 	PICMG 	06h 		M 	M
	Set FRU LED State 		PICMG 	07h 		M 	M
	Get FRU LED State 		PICMG 	08h 		M 	M
	Set IPMB State 			PICMG 	09h 		M 	M
	Set FRU Activation Policy 	PICMG 	0Ah 		M 	M
	Get FRU Activation Policy 	PICMG 	0Bh 		M 	M
	Set FRU Activation 		PICMG 	0Ch 		M 	M
	Get Device Locator Record ID 	PICMG 	0Dh 		O/M 	M
	Set Port State 			PICMG 	0Eh 		O/M 	O/M
	Get Port State 			PICMG 	0Fh 		O/M 	O/M
	Compute Power Properties 	PICMG 	10h 		M 	M
	Set Power Level 		PICMG 	11h 		M 	M
	Get Power Level 		PICMG 	12h 		M 	M
	Renegotiate Power 		PICMG 	13h 		M 	O
	Get Fan Speed Properties 	PICMG 	14h 		M 	M
	Set Fan Level 			PICMG 	15h 		O/M 	O/M
	Get Fan Level 			PICMG 	16h 		O/M 	O/M
	Bused Resource 			PICMG 	17h 		O/M 	O/M
	Get IPMB Link Info 		PICMG 	18h 		O/M 	O/M
 */
/*======================================================================*/

#define ATCA_CMD_GET_PICMG_PROPERTIES		0x00	/* Get PICMG Properties */
#define ATCA_CMD_GET_ADDRESS_INFO		0x01	/* Get Address Info */
#define ATCA_CMD_GET_SHELF_ADDRESS_INFO		0x02	/* Get Shelf Address Info */
#define ATCA_CMD_SET_SHELF_ADDRESS_INFO		0x03	/* Set Shelf Address Info */
#define ATCA_CMD_FRU_CONTROL			0x04	/* FRU Control */
#define ATCA_CMD_GET_FRU_LED_PROPERTIES		0x05	/* Get FRU LED Properties */
#define ATCA_CMD_GET_LED_COLOR			0x06	/* Get LED Color Capabilities */
#define ATCA_CMD_SET_FRU_LED_STATE		0x07	/* Set FRU LED State */
#define ATCA_CMD_GET_FRU_LED_STATE		0x08	/* Get FRU LED State */
#define ATCA_CMD_SET_IPMB_STATE			0x09	/* Set IPMB State */
#define ATCA_CMD_SET_FRU_ACTIVATION_POLICY	0x0a	/* Set FRU Activation Policy */
#define ATCA_CMD_GET_FRU_ACTIVATION_POLICY	0x0b	/* Get FRU Activation Policy */
#define ATCA_CMD_SET_FRU_ACTIVATION		0x0c	/* Set FRU Activation */
#define ATCA_CMD_GET_DEVICE_LOCATOR_REC_ID	0x0d	/* Get Device Locator Record ID */
#define ATCA_CMD_SET_PORT_STATE			0x0e	/* Set Port State */
#define ATCA_CMD_GET_PORT_STATE			0x0f	/* Get Port State */
#define ATCA_CMD_COMPUTE_POWER_PROPERTIES	0x10	/* Compute Power Properties */
#define ATCA_CMD_SET_POWER_LEVEL		0x11	/* Set Power Level */
#define ATCA_CMD_GET_POWER_LEVEL		0x12	/* Get Power Level */
#define ATCA_CMD_RENEGOTIATE_POWER		0x13	/* Renegotiate Power */
#define ATCA_CMD_GET_FAN_SPEED_PROPERTIES	0x14	/* Get Fan Speed Properties */
#define ATCA_CMD_SET_FAN_LEVEL			0x15	/* Set Fan Level */
#define ATCA_CMD_GET_FAN_LEVEL			0x16	/* Get Fan Level */
#define ATCA_CMD_BUSED_RESOURCE_CONTROL		0x17	/* Bused Resource Control */
#define ATCA_CMD_GET_IPMB_LINK_INFO		0x18	/* Get IPMB Link Info */

/* AMC commands */
#define ATCA_CMD_SET_AMC_PORT_STATE		0x19
#define ATCA_CMD_GET_AMC_PORT_STATE		0x1A
#define ATCA_CMD_SET_CLOCK_STATE		0x2C
#define ATCA_CMD_GET_CLOCK_STATE		0x2D

/* V3.0 commands */
#define ATCA_CMD_FRU_CONTROL_CAPABILITIES	0x1E	/* FRU control capabilities */

#define FRU_STATE_M0_NOT_INSTALLED		0	/* M0 – FRU Not Installed */
#define FRU_STATE_M1_INACTIVE			1	/* M1 – FRU Inactive */
#define FRU_STATE_M2_ACTIVATION_REQUEST		2	/* M2 – FRU Activation Request */
#define FRU_STATE_M3_ACTIVATION_IN_PROGRESS	3	/* M3 – FRU Activation In Progress */
#define FRU_STATE_M4_ACTIVE			4	/* M4 – FRU Active */
#define FRU_STATE_M5_DEACTIVATION_REQUEST	5	/* M5 – FRU Deactivation Request */
#define FRU_STATE_M6_DEACTIVATION_IN_PROGRESS	6	/* M6 – FRU Deactivation In Progress */
#define FRU_STATE_M7_COMMUNICATION_LOST		7	/* M7 – FRU Communication Lost */

typedef struct fru_fan_info {
	uchar min_speed_level;
	uchar max_speed_level;
	uchar norm_operating_level;
	uchar fan_tray_prop;
	uchar override_fan_level;
	uchar local_control_fan_level;
	uchar fan_control;
} FRU_FAN_INFO;

#define FAN_CONTROL_LOCAL	0
#define FAN_CONTROL_OVERRIDE	1
#define FAN_CONTROL_SHUTDOWN	2

#define PICMG_ID		0
/*----------------------------------------------------------------------*/
/*			PICMG Generic Response				*/
/*----------------------------------------------------------------------*/
typedef struct picmg_cmd_resp {
	uchar	completion_code;	/* Completion Code. */
	uchar	picmg_id;		/* PICMG Identifier. Indicates that 
					   this is a PICMG®-defined group
					   extension command. A value of 
					   00h shall be used. */
} PICMG_CMD_RESP;

/*----------------------------------------------------------------------*/
/* 			Get PICMG Properties 				*/
/*----------------------------------------------------------------------*/

typedef struct get_picmg_properties_cmd_req {
	uchar	command;
	uchar	picmg_id;		/* PICMG Identifier. Indicates that
					   this is a PICMG®-defined group
					   extension command. A value of 
					   00h shall be used. */
} GET_PICMG_PROPERTIES_CMD_REQ;

/* Response data */
typedef struct get_picmg_properties_cmd_resp {
	uchar	completion_code;	/* Completion Code. */
	uchar	picmg_id;		/* PICMG Identifier. Indicates that
					   this is a PICMG®-defined group
					   extension command. A value of 
					   00h shall be used. */
#ifdef BF_MS_FIRST
	uchar	picmg_extension_ver_minor:4,
		picmg_extension_ver_major:4;
				  	/* PICMG Extension Version. 
					   Indicates the version of PICMG®
					   extensions implemented by the 
					   IPM Controller.
					   [7:4] = BCD encoded minor version
					   [3:0] = BCD encoded major version
					   This specification defines version 
					   2.1 of the PICMG® extensions. IPM
					   Controllers implementing the extensions 
					   as defined by this specification shall 
					   report a value of 12h.The value 00h 
					   is reserved. */
#else
	uchar	picmg_extension_ver_major:4,
		picmg_extension_ver_minor:4;
#endif
	uchar	max_fru_dev_id;		/* Max FRU Device ID. The numerically
					   largest FRU Device ID for the
					   Managed FRUs implemented by this 
					   IPM Controller, excluding FRU
					   Device IDs reserved at the top of 
					   the range for special purposes, as
					   detailed in Table 3-10, 
					   “Reserved FRU Device IDs.” */
	uchar	ipmc_fru_dev_id;	/* FRU Device ID for IPM Controller. 
					   Indicates a FRU Device ID for the
					   FRU containing the IPM Controller. 
					   IPM Controllers implementing the
					   extensions defined by this specification 
					   shall report 0. */
} GET_PICMG_PROPERTIES_CMD_RESP;

#define FRU_CONTROL_DEACTIVATE_FRU	0
#define	FRU_CONTROL_ACTIVATE_FRU	1

/*----------------------------------------------------------------------*/
/* 			Set FRU Activation command 			*/
/*----------------------------------------------------------------------*/

/* Request data */ 
typedef struct set_fru_activation_cmd_req {
	uchar	command;
	uchar	picmg_id;		/* PICMG Identifier. Indicates that
					   this is a PICMG®-defined group
					   extension command. A value of 
					   00h shall be used. */
	uchar	fru_dev_id;		/* FRU Device ID. */
	uchar	fru_activation;		/* FRU Activation/Deactivation
					   00h = Deactivate FRU
					   01h = Activate FRU
					   02h-FFh = Reserved. */
} SET_FRU_ACTIVATION_CMD_REQ;

/* Response data */
typedef struct set_fru_activation_cmd_resp {
	uchar	completion_code;	/* Completion Code. */
	uchar	picmg_id;		/* PICMG Identifier. Indicates that 
					   this is a PICMG®-defined group
					   extension command. A value of 
					   00h shall be used. */
} SET_FRU_ACTIVATION_CMD_RESP;

/*----------------------------------------------------------------------*/
/*			Get Address Info command 			*/
/*----------------------------------------------------------------------*/

typedef struct get_address_info_cmd_req {
	uchar	command;
	uchar	picmg_id;		/* PICMG Identifier. Indicates that 
					   this is a PICMG®-defined group 
					   extension command. A value of 00h
					   shall be used. */
	uchar	fru_dev_id;		/* FRU Device ID. Indicates an individual 
					   FRU. This field is optional. If the 
					   field is not present, the command 
					   shall return addressing information 
					   for the FRU containing the IPM 
					   Controller that implements the command. 
					   This is ignored when Address Key Type 
					   is set to Physical Address. This 
					   field is required if Address Key Type 
					   is present. */
	uchar	addr_key_type;		/* Address Key Type. This defines the
					   type of address that is being provided 
					   in the Address Key field. This field 
					   is optional. If this field is not present,
					   the command shall return addressing 
					   information for the FRU specified by
					   the FRU Device ID. */
#define AKT_HW_ADDR	0x00		/* Hardware Address */
#define AKT_IPMB0_ADDR	0x01		/* IPMB-0 Address */
#define AKT_RESERVED	0x02		/* Reserved for PICMG® 2.9. */
#define AKT_PHYS_ADDR	0x03		/* Physical Address */
					/* All other values reserved. */
	uchar	addr_key;		/* Address Key. This is the address to
					   look-up in the table. This field is 
					   required if Address Key Type is present.
					   This holds a Hardware Address, IPMB address,
					   or Site Number depending on what is in 
					   the Address Key Type. */
	uchar	site_type;		/* Site Type. This field is required 
					   if Address Key Type is a Physical 
					   Address. */
} GET_ADDRESS_INFO_CMD_REQ;

typedef struct get_address_info_cmd_resp {
	uchar	completion_code;	/* Completion Code. */
	uchar	picmg_id;		/* PICMG Identifier. Indicates that this 
					   is a PICMG®-defined group extension
					   command. A value of 00h shall be used. */
	uchar	hw_addr;		/* Hardware Address. */
	uchar	ipmb0_addr;		/* IPMB-0 Address. Indicates the IPMB 
					   address for IPMB-0, if implemented. For
					   PICMG 2.9. a value of FFh indicates that 
					   IPMB-0 is not implemented. */
	uchar	reserved;		/* Reserved. Shall have a value of FFh. 
					   Other values reserved in PICMG® 2.9. */
	uchar	fru_dev_id;		/* FRU device ID. */
	uchar	site_id;		/* Site ID. */
	uchar	site_type;		/* Site Type. */
#define SITE_TYPE_ATCA			0x00	/* AdvancedTCA® Board */
#define SITE_TYPE_PEM			0x01	/* Power Entry Module */
#define SITE_TYPE_SHELF_FRU_INFO	0x02	/* Shelf FRU Information */
#define SITE_TYPE_DEDICATED_SHMC	0x03	/* Dedicated ShMC */
#define SITE_TYPE_FAN_TRAY		0x04	/* Fan Tray */
#define SITE_TYPE_FAN_FILTER_TRAY	0x05	/* Fan Filter Tray */
#define SITE_TYPE_ALARM			0x06	/* Alarm */
#define SITE_TYPE_AMC			0x07	/* AdvancedMC® Module (mezzanine) */
#define	SITE_TYPE_PMC			0x08	/* PMC */
#define SITE_TYPE_RTM			0x09	/* Rear Transition Module */
						/* C0h - CFh = OEM */
						/* All other values reserved. */
} GET_ADDRESS_INFO_CMD_RESP;

typedef struct picmg_address_info {
	uchar hw_addr;
	uchar ipmb0_addr;
	uchar phys_addr;
	uchar fru_dev_id;
	uchar site_id;
} PICMG_ADDRESS_INFO;

/*----------------------------------------------------------------------*/
/*			Set Shelf Address Info command 			*/
/*----------------------------------------------------------------------*/

typedef struct set_shelf_address_info_cmd_req {
	uchar	command;
	uchar	picmg_id;		/* PICMG Identifier. Indicates that 
					   this is a PICMG®-defined group
					   extension command. A value of 
					   00h shall be used. */
	uchar	shelf_addr_type;	/* Shelf Address Type/Length Byte. 
					   Indicates the Type and Length 
					   (in characters, occupying N bytes,
					   depending on the packing signaled
					   by Type) of the following Shelf
					   address. */
	uchar	shelf_addr_data[16];	/* 3 + N Shelf Address bytes. 
					   Lists the Shelf address bytes for
					   the Shelf that contains the IPM 
					   Controller. Note: 16 arbitrarily
					   selected, review*/
} SET_SHELF_ADDRESS_INFO_CMD_REQ;

typedef struct get_shelf_address_info_cmd_resp {
	uchar	completion_code;	/* Completion Code. */
	uchar	picmg_id;		/* PICMG Identifier. Indicates that 
					   this is a PICMG®-defined group
					   extension command. A value of 00h
					   shall be used. */
	uchar	shelf_addr_type;	/* Shelf Address Type/Length Byte. 
					   Indicates the Type and Length (in
					   characters, occupying N bytes, 
					   depending on the packing signaled 
					   by Type) of the following Shelf 
					   address */
	uchar	shelf_addr_data;	/* 4 + N Shelf Address bytes. 
					   Lists the Shelf address bytes for 
					   the Shelf containing the IPM 
					   Controller. */
} GET_SHELF_ADDRESS_INFO_CMD_RESP;

#define FRU_ACTIVATION_POLICY_DEACTIVATION_LOCK		0x2
#define FRU_ACTIVATION_POLICY_LOCK			0x1
/*----------------------------------------------------------------------*/
/*			 Set FRU Activation Policy command		*/
/*----------------------------------------------------------------------*/

typedef struct set_fru_activation_policy_cmd_req {
	uchar	command;
	uchar	picmg_id;		/* PICMG Identifier. Indicates that 
					   this is a PICMG®-defined group
					   extension command. A value of 
					   00h shall be used. */
	uchar	fru_dev_id;		/* FRU Device ID. */
	uchar	act_policy_mask;	/* FRU Activation Policy Mask Bits
					   [7:2] = Reserved. Write as 0000 00b
					   [1] = 1 – Bit in Byte 4 of command 
					   will affect the Deactivation-Locked
					   bit
					   [0] = 1 – Bit in Byte 4 of command 
					   will affect the Locked bit */
	uchar	act_policy_set;		/* FRU Activation Policy Set Bits
					   [7:2] = Reserved. Write as 0000 00b
					   [1] = Set/Clear Deactivation-Locked
					   – FRU cannot/can transition from
					   M4 to M5 (ignored if Bit 1 of Byte 3= 0)
					   [0] = Set/Clear Locked – FRU 
					   cannot/can transition from M1 to M2
					   (ignored if Bit 0 of Byte 3= 0) */
} SET_FRU_ACTIVATION_POLICY_CMD_REQ;

typedef struct set_fru_activation_policy_cmd_resp {
	uchar	completion_code;	/* Completion Code. */
	uchar	picmg_id;		/* PICMG Identifier. Indicates that 
					   this is a PICMG®-defined group
					   extension command. A value of 00h
					   shall be used. */
} SET_FRU_ACTIVATION_POLICY_CMD_RESP;

/*----------------------------------------------------------------------*/
/*			 Get FRU Activation Policy command		*/
/*----------------------------------------------------------------------*/

typedef struct get_fru_activation_policy_cmd_req {
	uchar	command;
	uchar	picmg_id;		/* PICMG Identifier. Indicates that 
					   this is a PICMG®-defined group
					   extension command. A value of 
					   00h shall be used. */
	uchar	fru_dev_id;		/* FRU Device ID. */
} GET_FRU_ACTIVATION_POLICY_CMD_REQ;

typedef struct get_fru_activation_policy_cmd_resp {
	uchar	completion_code;	/* Completion Code. */
	uchar	picmg_id;		/* PICMG Identifier. Indicates that 
					   this is a PICMG®-defined group
					   extension command. A value of 00h
					   shall be used. */
#ifdef BF_MS_FIRST
	uchar	:6,			/* FRU Activation Policy
					   [7:2] = Reserved. Write as 0000 00b */
		deactivation_locked:1,	/* [1] = Deactivation-Locked bit. 
					   A 1b indicates the FRU is Deactivation-
					   Locked. If it is in the M4 state, 
					   it will not transition to M5 until
					   this bit is cleared. */
		locked:1;		/* [0] = Locked bit. A 1b indicates 
					   the FRU is locked and if it is in
					   the M1 state, it will not transition
					   on to M2 until unlocked. */
#else
	uchar	locked:1,
		deactivation_locked:1,
		:6;
#endif
		       
} GET_FRU_ACTIVATION_POLICY_CMD_RESP;

/*----------------------------------------------------------------------*/
/*			FRU Hot Swap Event Message 			*/
/*----------------------------------------------------------------------*/

typedef struct fru_hot_swap_event_msg_req {
	uchar	command;
	uchar	evt_msg_rev;		/* Event Message Rev = 04h (IPMI v1.5) */
	uchar	sensor_type;		/* Sensor Type = F0h (Hot Swap Event)  */
	uchar	sensor_number;		/* Sensor Number = 0xxh (Implementation specific) */
	uchar	evt_direction;		/* Event Direction (bit7) = 0b (Assertion)
					   Event Type [6:0] = 6Fh (Generic Availability) */
	uchar	evt_data1;		/* Event Data 1
					   [7:4] = Ah (OEM code in Event Data 2,
					   OEM code in Event Data 3)
					   [3:0] = Current State  */
/*
0 = M0 – FRU Not Installed
1 = M1 – FRU Inactive
2 = M2 – FRU Activation Request
3 = M3 – FRU Activation In Progress
4 = M4 – FRU Active
5 = M5 – FRU Deactivation Request
6 = M6 – FRU Deactivation In Progress
7 = M7 – FRU Communication Lost
8-Fh = Reserved
*/
	uchar	evt_data2;		/* Event Data 2
					   [7:4] = Cause of state change. 
					   See Table 3-20, “Cause of State Change values,” 
					   for values.
					   [3:0] = Previous State */
/*
0 = M0 – FRU Not Installed
1 = M1 – FRU Inactive
2 = M2 – FRU Activation Request
3 = M3 – FRU Activation In Progress
4 = M4 – FRU Active
5 = M5 – FRU Deactivation Request
6 = M6 – FRU Deactivation In Progress
7 = M7 – FRU Communication Lost
8-Fh = Reserved
*/
	uchar	evt_data3;		/* Event Data 3
					   [7:0] = FRU Device ID */
} FRU_HOT_SWAP_EVENT_MSG_REQ;

typedef struct fru_hot_swap_event_msg_resp {
	uchar	completion_code;	/* Completion Code */
} FRU_HOT_SWAP_EVENT_MSG_RESP;

/* Table 3-20 Cause of State Change values */

#define	STATE_CH_NORMAL		0x00
/*
Normal State Change. This is used when the FRU is proceeding normally through the state chart.
For instance, an M3 to M4 transition is a normal state change. Other values in this table can be
used to provide greater levels of detail about what initiated a transition. Valid for the M0 to M1, M1
to M2, M2 to M3, M3 to M4, M4 to M5, M5 to M6, and M6 to M1 transitions.
*/

#define STATE_CH_SH_MGR		0x01
/*
Change Commanded by Shelf Manager with Set FRU Activation. The Shelf Manager has issued a
command to change states, typically during an insertion or extraction. Valid for the M2 to M1, M2
to M3, M4 to M6, M5 to M4, and M5 to M6 transitions.
*/

#define STATE_CH_OPERATOR	0x02
/*
State Change due to operator changing a Handle Switch. The FRU has changed states as a result
of an operator changing the state of a Handle Switch. Valid for the M1 to M2, M3 to M6, M4 to M5,
and M5 to M4 transitions.
*/

#define STATE_CH_FRU_ACTION	0x03
/* 
State Change due to FRU programmatic action. The FRU has changed states due to some
non-operator related internal requirement (such as Locked bit being cleared). Valid for the M1 to
M2, M3 to M6, M4 to M5, and M5 to M4 transitions.
*/

#define STATE_CH_COMM_CHANGE	0x04
/*
Communication Lost or Regained. The Shelf Manager has lost or regained contact with the FRU
and generated an event on its behalf. Valid for the M2 to M7, M3 to M7, M4 to M7, M5 to M7, and
M6 to M7, M7 to M1, M7 to M2, M7 to M3, M7 to M4, M7 to M5 and M7 to M6 transitions.
*/

#define STATE_CH_COMM_CHANGE_LOC_DETECTED	0x05
/*
Communication Lost or Regained–locally detected. The FRU has changed state as a result of an
internal detection by the IPM Controller. This is only valid for FRUs represented by a physically
separate IPM Controller (e.g., mezzanine cards). Valid for the M2 to M7, M3 to M7, M4 to M7, M5
to M7, M6 to M7, M7 to M1, M7 to M2, M7 to M3, M7 to M4, M7 to M5 and M7 to M6 transitions.
*/

#define STATE_CH_SURPRISE	0x06
/*
Surprise State Change due to extraction. The FRU has changed state abruptly to M0 due to a
non-compliant removal from the system. This is only valid for FRUs represented by a physically
separate IPM Controller (e.g., mezzanine cards). Valid for the M2 to M0, M3 to M0, M4 to M0, M5
to M0, M6 to M0, and M7 to M0 transitions.
*/

#define STATE_CH_INFO		0x07
/*
State Change due to provided information. A new state is known for the FRU that could not be
deduced previously. This is used when a user verifies that a FRU has been extracted from the
Shelf and is no longer available. Valid for the M7 to M0 state transition.
*/

#define STATE_CH_INVALID_HW_ADDR	0x08
/*
Invalid Hardware Address Detected. This is an error condition where the Hardware Address did
not pass the parity check. Valid for the M0 to M0 transition.
9h Unexpected Deactivation. The FRU has transitioned to deactivating without requesting
permission from the Shelf Manager first. Valid for M4 to M6 transition.
*/

#define STATE_CH_UNKNOWN	0x0f
/*
State Change, Cause Unknown. No cause could be determined.
*/

/*
All other values Reserved.
*/

/*----------------------------------------------------------------------*/
/*		Get Sensor Reading (FRU Hot Swap Sensor) command	*/
/*----------------------------------------------------------------------*/

typedef struct get_sensor_reading_hot_swap_cmd_req {
	uchar	command;
	uchar	sensor_number;		/* Sensor Number (FFh = reserved) */
} GET_SENSOR_READING_HOT_SWAP_CMD_REQ;

typedef struct get_sensor_reading_cmd_resp {
	uchar	completion_code;	/* Completion Code */
	uchar	sensor_reading;		/* Sensor Reading. 
					   [7:0] - Not used. Write as 00h. */
	uchar	std_ipmi_byte;		/* Standard IPMI byte (See “Get 
					   Sensor Reading” in IPMI specification):
					   [7] - 0b = All Event Messages
					   disabled from this sensor
					   [6] - 0b = sensor scanning disabled
					   [5] - 1b = initial update in progress. 
	This bit is set to indicate that a “Re-arm Sensor Events” or “Set 
	Event Receiver” command has been used to request an update of the
	sensor status, and that update has not occurred yet. Software
	should use this bit to avoid getting an incorrect status while the first sensor
	update is in progress. This bit is only required if it is possible for the IPM
	Controller to receive and process a “Get Sensor Reading or Get Sensor Event
	Status” command for the sensor before the update has completed. This is most
	likely to be the case for sensors, such as fan RPM sensors, that may require
	seconds to accumulate the first reading after a re-arm.
					  [4:0] – reserved. Ignore on read. */
	uchar	current_state_mask;	/* Current State Mask */
/*
[7] – 1b = FRU Operational State M7 - Communication Lost
[6] – 1b = FRU Operational State M6 - FRU Deactivation In Progress
[5] – 1b = FRU Operational State M5 - FRU Deactivation Request
[4] – 1b = FRU Operational State M4 - FRU Active
[3] – 1b = FRU Operational State M3 - FRU Activation in Progress
[2] – 1b = FRU Operational State M2 - FRU Activation Request
[1] – 1b = FRU Operational State M1 - FRU Inactive
[0] – 1b = FRU Operational State M0 - FRU Not Installed
*/
/*
(5) [7:0] – Optional/Reserved. If provided, write as 80h (IPMI restriction). 
Ignore on read.
*/
} GET_SENSOR_READING_CMD_RESP;

/*----------------------------------------------------------------------*/
/*			FRU Control command	 			*/
/*----------------------------------------------------------------------*/

typedef struct fru_control_cmd_req {
	uchar	command;
	uchar	picmg_id;		/* PICMG Identifier. Indicates that 
					   this is a PICMG®-defined group
					   extension command. A value of 
					   00h shall be used. */
	uchar	fru_dev_id;		/* FRU Device ID. */
	uchar	fru_control_options;	/* FRU Control Options are: */
#define FRU_CONTROL_COLD_RESET		0x00	/* 00h = Cold Reset */
#define FRU_CONTROL_WARM_RESET		0x01	/* 01h = Warm Reset */
#define FRU_CONTROL_GRACEFUL_REBOOT	0x02	/* 02h = Graceful Reboot */
#define FRU_CONTROL_ISSUE_DIAG_INT	0x03	/* 03h = Issue Diagnostic Interrupt */
#define FRU_CONTROL_QUIESCE		0x04	/* 04h = Quiesce payload */
					/* 05h - FFh = Reserved */
} FRU_CONTROL_CMD_REQ;

typedef struct fru_control_cmd_resp {
	uchar	completion_code;	/* Completion Code. */
	uchar	picmg_id;		/* PICMG Identifier. Indicates that 
					   this is a PICMG®-defined group
					   extension command. A value of 00h
					   shall be used. */
} FRU_CONTROL_CMD_RESP;

/*----------------------------------------------------------------------*/
/*			Get FRU LED Properties command 			*/
/*----------------------------------------------------------------------*/

/* LED ID assignments (as defined in Section 2.2.8, “LEDs” */
#define FRU_LED_BLUE		0x00	/* This serves as an indication of 
					   whether or not the Front Board 
					   can be safely extracted. */

#define FRU_LED1		0x01	/* It is common practice in many 
					   telco and data center equipment 
					   designs to provide at least one
					   status LED used to indicate 
					   operational failure of Payload 
					   resources. LED 1 is intended to
					   serve this function. The color 
					   and details of meaning of this
					   LED vary from industry to industry.
					   In North American telco applications,
					   this LED is required by GR-2914 to
					   be RED and to be illuminated when
					   the Front Board is to be removed
					   from the Shelf. In European telco
					   applications, this LED is typically
					   AMBER and illuminated when the Front
					   Board is in a failed state. */			   
#define FRU_LED2		0x02	/* defined by the system implementer. */
#define FRU_LED3		0x03	/* defined by the system implementer. */
					/* 04h-FEh Application specific LEDs */
					/* FFh Reserved */


typedef struct get_led_properties_cmd_req {
	uchar	command;
	uchar	picmg_id;		/* PICMG Identifier. Indicates that 
					   this is a PICMG®-defined group
					   extension command. A value of 
					   00h shall be used. */
	uchar	fru_dev_id;		/* FRU Device ID. */
} GET_LED_PROPERTIES_CMD_REQ;

typedef struct get_led_properties_cmd_resp {
	uchar	completion_code;	/* Completion Code. */
	uchar	picmg_id;		/* PICMG Identifier. Indicates that 
					   this is a PICMG®-defined group
					   extension command. A value of 00h
					   shall be used. */

	uchar	gen_status_led_prop;	/* General Status LED Properties
					   A bit field indicating the FRU’s 
					   ability to control the four
					   LEDs defined in Section 2.2.8.1,
					   “General status LEDs.”
					   When a bit is set, the FRU can 
					   control the associated LED.
					   [7:4] Reserved, shall be set to 0. */
#define GS_LED3			0x8	/* [3] LED3 */
#define GS_LED2			0x4	/* [2] LED2 */
#define	GS_LED1			0x2	/* [1] LED1 */
#define GS_LED_BLUE		0x1	/* [0] BLUE LED */

	uchar	app_spec_led_count;	/* Application Specific LED Count
					   The number of application specific 
					   LEDs under control of the IPM Controller.
					   00h-FB Number of application specific
					   LEDs under control by IPM Controller. 
					   If no application specific LEDs are 
					   present this field is 00h.
					   FCh-FFh Reserved */
} GET_LED_PROPERTIES_CMD_RESP;

/*----------------------------------------------------------------------*/
/*			Get LED Color Capabilities command		*/
/*----------------------------------------------------------------------*/

typedef struct get_led_color_capabilities_cmd_req {
	uchar	command;
	uchar	picmg_id;		/* PICMG Identifier. Indicates that 
					   this is a PICMG®-defined group
					   extension command. A value of 
					   00h shall be used. */
	uchar	fru_dev_id;		/* FRU Device ID. */
	uchar	led_id;			/* LED ID - FFh Reserved */
} GET_LED_COLOR_CAPABILITIES_CMD_REQ;

typedef struct	get_led_color_capabilities_cmd_resp {
	uchar	completion_code;	/* Completion Code
					   CCH–If the LED ID contained in
					   the Request data is not present
					   on the FRU. */
	uchar	picmg_id;		/* PICMG Identifier. Indicates that 
					   this is a PICMG®-defined group
					   extension command. A value of 
					   00h shall be used. */
	uchar	led_color_capabilities;	/* LED Color Capabilities
					   A bit field. When the bit set, 
					   the LED supports the color. */
					/* [7] Reserved, shall be set to 0. */
#define LED_SUPPORT_WHITE	0x40	/* [6] LED supports WHITE */
#define LED_SUPPORT_ORANGE	0x20	/* [5] LED supports ORANGE */
#define LED_SUPPORT_AMBER	0x10	/* [4] LED supports AMBER */
#define LED_SUPPORT_GREEN	0x08	/* [3] LED supports GREEN */
#define LED_SUPPORT_RED		0x04	/* [2] LED supports RED */
#define LED_SUPPORT_BLUE	0x02	/* [1] LED supports BLUE */
					/* [0] Reserved, shall be set to 0. */
	uchar	default_led_color;	/* Default LED Color in Local Control State
					   [7:4] Reserved, shall be set to 0.
					   [3:0] Hex Value Function : */
					/* 0h Reserved */
#define LED_COLOR_BLUE		0x01	/* 1h BLUE   */
#define LED_COLOR_RED		0x02	/* 2h RED    */
#define LED_COLOR_GREEN		0x03	/* 3h GREEN  */
#define LED_COLOR_AMBER		0x04	/* 4h AMBER  */
#define LED_COLOR_ORANGE	0x05	/* 5h ORANGE */
#define LED_COLOR_WHITE		0x06	/* 6h WHITE  */
					/* 7h-Fh Reserved */
	uchar	default_led_color_override;	/* Default LED Color in Override State
					   [7:4] Reserved, shall be set to 0.
					   [3:0] Hex Value Function */
					/* 0h Reserved    */
					/* 1h BLUE        */
					/* 2h RED         */
					/* 3h GREEN       */
					/* 4h AMBER       */
					/* 5h ORANGE      */
					/* 6h WHITE       */
					/* 7h-Fh Reserved */
} GET_COLOR_CAPABILITIES_CMD_RESP;

/*----------------------------------------------------------------------*/
/*			Set FRU LED State command 			*/
/*----------------------------------------------------------------------*/

typedef struct set_fru_led_state_cmd_req {
	uchar	command;
	uchar	picmg_id;		/* PICMG Identifier. Indicates that 
					   this is a PICMG®-defined group
					   extension command. A value of 
					   00h shall be used. */
	uchar	fru_dev_id;		/* FRU Device ID. */
	uchar	led_id;			/* LED ID - FFh Reserved */

	/* LED ID assignments (as defined in Section 2.2.8, “LEDs” */
	/* 
	 * LED_BLUE		0x00
	 * LED1			0x01
	 * LED2			0x02
	 * LED3			0x03
	 */

#define LED_TEST_ALL		0xff	/* Lamp Test (All LEDs under 
					   management control are addressed) */
	uchar	led_function;		/* LED Function
					   00h = LED off override
					   01h - FAh = LED BLINKING override. 
					   The off duration is specified by 
					   the value of this byte and the on 
					   duration is specified by the value
					   of byte 5. Both values specify the
					   time in tens of milliseconds 
					   (10 ms –2.5 s)
					   FBh = LAMP TEST state. 
					   Turn on LED(s) specified in byte 
					   3 for duration specified in byte 
					   5 (in hundreds of milliseconds) 
					   then return to the highest priority
					   state.
					   FCh = LED state restored to Local 
					   Control state
					   FDh-FEh Reserved
					   FFh = LED on override */
	uchar	on_duration;		/* On-duration: LED on-time in tens 
					   of milliseconds if (1 = Byte 4 = FAh)
					   Lamp Test time in hundreds of 
					   milliseconds if (Byte 4 = FBh. 
					   Lamp Test time value must be less
					   than 128. Other values when Byte 4
					   = FBh are reserved.
					   Otherwise, this field is ignored
					   and shall be set to 0h. */

	uchar	color;			/* Color when illuminated. This byte
					   sets the override color when
					   LED Function is 01h–FAh and FFh. 
					   This byte sets the Local Control
					   color when LED Function is FCh. 
					   This byte may be ignored during 
					   Lamp Test or may be used to control
					   the color during the lamp test when
					   LED Function is FBh. 
					   [7:4] Reserved, shall be set to 0.
					   [3:0] Hex Value Function
					   0h Reserved
					   1h Use BLUE
					   2h Use RED
					   3h Use GREEN
					   4h Use AMBER
					   5h Use ORANGE
					   6h Use WHITE
					   7h-Dh Reserved
					   Eh Do not change
					   Fh Use default color */
} SET_FRU_LED_STATE_CMD_REQ;

typedef struct set_fru_led_state_cmd_resp {
		uchar	completion_code;	/* Completion Code */
		uchar	picmg_id;		/* PICMG Identifier. Indicates that 
					   this is a PICMG®-defined group
					   extension command. A value of 
					   00h shall be used. */
} SET_FRU_LED_STATE_CMD_RESP;

/*----------------------------------------------------------------------*/
/*			Get FRU LED State command 			*/
/*----------------------------------------------------------------------*/

typedef struct get_fru_led_state_cmd_req {
	uchar	command;
	uchar	picmg_id;		/* PICMG Identifier. Indicates that 
					   this is a PICMG®-defined group
					   extension command. A value of 
					   00h shall be used. */
	uchar	fru_dev_id;		/* FRU Device ID. */
	uchar	led_id;			/* LED ID - FFh Reserved */

	/* LED ID assignments (as defined in Section 2.2.8, “LEDs” */
	/* 
	 * LED_BLUE		0x00
	 * LED1			0x01
	 * LED2			0x02
	 * LED3			0x03
	 */
} GET_FRU_LED_STATE_CMD_REQ;

typedef struct get_fru_led_state_cmd_resp {
	uchar	completion_code;	/* Completion Code
					   CCH–If the LED ID contained in
					   the Request data is not present
					   on the FRU. */
	uchar	picmg_id;		/* PICMG Identifier. Indicates that 
					   this is a PICMG®-defined group
					   extension command. A value of 
					   00h shall be used. */
	uchar	led_states;		/* LED States:
					   [7:3] - Reserved
					   [2] - 1b if Lamp Test has been enabled.
					   [1] - 1b if override state has been 
					         enabled.
					   [0] - 1b if IPM Controller has a 
					         Local Control state. */
	uchar local_control_led_func;	/* Local Control LED Function
					   00h = LED is off. This is a default
					   value if the IPM Controller does not
					   support Local Control.
					   01h - FAh = LED is BLINKING. 
					   The off duration is specified by the
					   value of this byte, and the on duration
					   is specified by the value of byte 5. 
					   Both values specify the time in tens
					   of milliseconds (10 ms–2.5 s).
					   FB - FEh = Reserved
					   FFh = LED is on */
	uchar	local_control_on_duration;	/* Local Control On-duration
					   The LED on-time in tens of milliseconds 
					   if (01h = Byte 4 = FAh) and ignored otherwise.
					   Otherwise, this field is ignored and
					   shall be set to 0h. */

	uchar	local_control_color;	/* [7:4] Reserved, shall be set to 0.
					   [3:0] Hex Value Function : */
					/* 0h Reserved */
					/* LED_COLOR_BLUE
					 * LED_COLOR_RED
					 * LED_COLOR_GREEN
					 * LED_COLOR_AMBER
					 * LED_COLOR_ORANGE
					 * LED_COLOR_WHITE
					 * 7h-Fh Reserved */

	uchar	override_led_state_func;/* Override state LED Function. This 
					   byte is required if either override
					   state or Lamp Test is in effect.
					   00h = LED override state is off.
					   01h - FAh = LED override state is BLINKING. 
					   The off duration is specified by 
					   the value of this byte and the on 
					   duration is specified by the value 
					   of byte 8. Both values specify the
					   time in tens of milliseconds (10 ms–2.5 s).
					   FBh - FEh = Reserved
					   FFh = LED override state is on */
	uchar	override_state_on_duration;	/* Override State On-duration. 
					   This byte is required if either override
					   state or Lamp Test is in effect.
					   The LED on-time in tens of milliseconds
					   if (01h = Byte 7 = FAh) and ignored 
					   otherwise. */
	uchar	override_state_color;	/* Override State Color. This byte is 
					   required if either override state or
					   Lamp Test is in effect. */
					/* [7:4] Reserved, shall be set to 0.
					   [3:0] Hex Value Function : */
					/* 0h Reserved */
					/* LED_COLOR_BLUE
					 * LED_COLOR_RED
					 * LED_COLOR_GREEN
					 * LED_COLOR_AMBER
					 * LED_COLOR_ORANGE
					 * LED_COLOR_WHITE
					 * 7h-Fh Reserved */
	uchar led_test_duration;	/* Lamp Test Duration. This byte 
					   is optional if Lamp Test is not 
					   in effect. This byte contains the
					   Lamp Test time in hundreds of 
					   milliseconds and the value must 
					   be < 128. Other values reserved. */
} GET_FRU_LED_STATE_CMD_RESP;

typedef struct fru_led_state {
	uchar	led_states;		/* LED States:
					   [7:3] - Reserved
					   [2] - 1b if Lamp Test has been enabled.
					   [1] - 1b if override state has been 
					         enabled.
					   [0] - 1b if IPM Controller has a 
					         Local Control state. */
	uchar local_control_led_func;	/* Local Control LED Function
					   00h = LED is off. This is a default
					   value if the IPM Controller does not
					   support Local Control.
					   01h - FAh = LED is BLINKING. 
					   The off duration is specified by the
					   value of this byte, and the on duration
					   is specified by the value of byte 5. 
					   Both values specify the time in tens
					   of milliseconds (10 ms–2.5 s).
					   FB - FEh = Reserved
					   FFh = LED is on */
	uchar	local_control_on_duration;	/* Local Control On-duration
					   The LED on-time in tens of milliseconds 
					   if (01h = Byte 4 = FAh) and ignored otherwise.
					   Otherwise, this field is ignored and
					   shall be set to 0h. */

	uchar	local_control_color;	/* [7:4] Reserved, shall be set to 0.
					   [3:0] Hex Value Function : */
					/* 0h Reserved */
					/* LED_COLOR_BLUE
					 * LED_COLOR_RED
					 * LED_COLOR_GREEN
					 * LED_COLOR_AMBER
					 * LED_COLOR_ORANGE
					 * LED_COLOR_WHITE
					 * 7h-Fh Reserved */

	uchar	override_led_state_func;/* Override state LED Function. This 
					   byte is required if either override
					   state or Lamp Test is in effect.
					   00h = LED override state is off.
					   01h - FAh = LED override state is BLINKING. 
					   The off duration is specified by 
					   the value of this byte and the on 
					   duration is specified by the value 
					   of byte 8. Both values specify the
					   time in tens of milliseconds (10 ms–2.5 s).
					   FBh - FEh = Reserved
					   FFh = LED override state is on */
	uchar	override_state_on_duration;	/* Override State On-duration. 
					   This byte is required if either override
					   state or Lamp Test is in effect.
					   The LED on-time in tens of milliseconds
					   if (01h = Byte 7 = FAh) and ignored 
					   otherwise. */
	uchar	override_state_color;	/* Override State Color. This byte is 
					   required if either override state or
					   Lamp Test is in effect. */
					/* [7:4] Reserved, shall be set to 0.
					   [3:0] Hex Value Function : */
					/* 0h Reserved */
					/* LED_COLOR_BLUE
					 * LED_COLOR_RED
					 * LED_COLOR_GREEN
					 * LED_COLOR_AMBER
					 * LED_COLOR_ORANGE
					 * LED_COLOR_WHITE
					 * 7h-Fh Reserved */
	uchar led_test_duration;	/* Lamp Test Duration. This byte 
					   is optional if Lamp Test is not 
					   in effect. This byte contains the
					   Lamp Test time in hundreds of 
					   milliseconds and the value must 
					   be < 128. Other values reserved. */
} FRU_LED_STATE;

typedef struct fru_led_capabilities {
	uchar led_color_capabilities;
	uchar default_led_color;
	uchar default_led_color_override;
} FRU_LED_CAPABILITIES;

/* Per FRU data structure 
 * Include data for:
 * 	FRU Control
 * 	FRU LED Properties
 * 	FRU LED Color Capabilities
 * 	FRU LED State
 * 	FRU Activation Policy
 * 	FRU Device Locator Record ID
 * 	FRU Power Properties
 * 	FRU Power Level
 */
typedef struct fru_info {
	uchar	state;
	uchar	fru_activation;		/* FRU Activation/Deactivation
					   00h = FRU Deactivated
					   01h = FRU Activated
					   02h-FFh = Reserved. */
	uchar	hw_addr;		/* Hardware Address. */
	uchar	ipmb0_addr;		/* IPMB-0 Address. */
	uchar	fru_dev_id;		/* FRU device ID. */
	uchar	site_id;		/* Site ID. */
	uchar	site_type;		/* Site Type. */
	uchar	deactivation_locked;	/* FRU Activation Policy - deactivation_locked bit */
	uchar	locked;			/* FRU Activation Policy - (activation) locked bit */
	uchar	gen_status_led_prop;	/* General Status LED Properties. */
	uchar	app_spec_led_count;	/* Application Specific LED Count */
	FRU_LED_CAPABILITIES	led_capabilities[4];	/* LED Color Capabilities. */
	uchar	default_led_color;	/* Default LED Color in Local Control State */
	FRU_LED_STATE	led_state[4];
	uchar	record_id_LSB;		/* Record ID LS. Contains the least 
					   significant byte of the Record ID
					   for the appropriate Device Locator
					   SDR. */
	uchar	record_id_MSB;		/* Record ID MS. Contains the most 
					   significant byte of the Record ID
					   for the appropriate Device Locator
					   SDR. */
	uchar	dyn_power_config;	/* Dynamic Power Configuration. Set 
					   to 1b if the FRU supports dynamic 
					   reconfiguration of power */
	uchar	power_level_steady_state; /* Power Level.*/
	uchar	power_level_desired_steady_state;
	uchar	power_level_early_draw;
	uchar	power_level_early_draw_desired;
	uchar	delay_to_stable_power;	/* Delay to Stable Power.  */
	uchar	power_multiplier;	/* Power Multiplier. */
	uchar	*power_draw_table;	/* Power Draw[1..N].  */
} FRU_INFO;

/*----------------------------------------------------------------------*/
/*			Get Device Locator Record ID Command		*/
/*----------------------------------------------------------------------*/

typedef struct get_device_locator_record_id_cmd_req {
	uchar	command;
	uchar	picmg_id;		/* PICMG Identifier. Indicates that 
					   this is a PICMG®-defined group
					   extension command. A value of 
					   00h shall be used. */
	uchar	fru_dev_id;		/* FRU Device ID. This contains
					   the FRU Device ID to use when 
					   returning the Record ID. A value 
					   of zero returns the “Management 
					   Controller Device Locator Record” 
					   ID. A value between 1 and Max FRU 
					   Device ID (see Table 3-9, “Get 
					   PICMG Properties command”) returns
					   the “FRU Device Locator Record” ID. 
					   As per the IPMI specification, a 
					   value of FFh is reserved Response 
					   data. */
} GET_DEVICE_LOCATOR_RECORD_ID_CMD_REQ; 

typedef struct get_device_locator_record_id_cmd_resp {
	uchar	completion_code;	/* Completion Code
					   CCH–If the LED ID contained in
					   the Request data is not present
					   on the FRU. */
	uchar	picmg_id;		/* PICMG Identifier. Indicates that 
					   this is a PICMG®-defined group
					   extension command. A value of 
					   00h shall be used. */
	uchar	record_id_LSB;		/* Record ID LS. Contains the least 
					   significant byte of the Record ID
					   for the appropriate Device Locator
					   SDR. */
	uchar	record_id_MSB;		/* Record ID MS. Contains the most 
					   significant byte of the Record ID
					   for the appropriate Device Locator
					   SDR. */
} GET_DEVICE_LOCATOR_RECORD_ID_CMD_RESP;



/* Table 3-30 PICMG Entity ID assignments */
#define PICMG_ENTITY_FRONT_BOARD	0xA0	/* PICMG Front Board */
						/* Reserved A1h-AFh */
#define PICMG_ENTITY_REAR_TRANS_MODULE	0xC0	/* PICMG Rear Transition Module */
						/* Reserved C1h-CFh */
#define PICMG_ENTITY_SH_MGMT_CONTROLLER	0xF0	/* PICMG Shelf Management Controller */
#define PICMG_ENTITY_FILTRATION_UNIT	0xF1	/* PICMG Filtration Unit */
#define PICMG_ENTITY_SHELF_FRU_INFO	0xF2	/* PICMG Shelf FRU Information */
						/* Reserved F3h-FFh */
#define PICMG_ENTITY_PWR_FILTER		0xA0	/* Power Filtering and Circuit Protection */
#define PICMG_ENTITY_FAN_TRAY		0x1E	/* Fan Tray or other cooling unit */


/*----------------------------------------------------------------------*/
/*			Set Port State command				*/
/*----------------------------------------------------------------------*/
typedef struct link_info {
#ifdef BF_MS_FIRST
	unsigned long
		link_grouping_id:8,
		link_type_extension:4,
		link_type:8,
		port3_bit_flag:1,
		port2_bit_flag:1,
		port1_bit_flag:1,
		port0_bit_flag:1,
		interface:2,
		channel_number:6;
#else
	unsigned long
		channel_number:6,
		interface:2,
		port0_bit_flag:1,
		port1_bit_flag:1,
		port2_bit_flag:1,
		port3_bit_flag:1,
		link_type:8,
		link_type_extension:4,
		link_grouping_id:8;
#endif			    
} LINK_INFO;

typedef struct set_port_state_cmd_req {
	uchar	command;
	uchar	picmg_id;		/* PICMG Identifier. Indicates that 
					   this is a PICMG®-defined group
					   extension command. A value of 
					   00h shall be used. */

	/* Link Info. LS Byte first. Describes the Link that should 
	   be enabled or disabled.
		[31:24] — Link Grouping ID
		[23:20] — Link Type Extension
		[19:12] — Link Type
		[11] — Port 3 Bit Flag
		[10] — Port 2 Bit Flag
		[09] — Port 1 Bit Flag
		[08] — Port 0 Bit Flag
		[07:06] — Interface
		[05:00] — Channel Number */
	LINK_INFO	link_info;
	uchar	state;			/* State. Indicates the desired 
					   state of the Link as described 
					   by Link Info. */
#define PORT_STATE_ENABLE	0x1	/* Enable */
#define PORT_STATE_DISABLE	0x0	/* Disable */	
					/* All other values reserved. */
} SET_PORT_STATE_CMD_REQ;

typedef struct link_info_entry {
	LINK_INFO	link_info;
	uchar	state;
	uchar	entry_in_use;
} LINK_INFO_ENTRY;

/*----------------------------------------------------------------------*/
/*			Get Port State command				*/
/*----------------------------------------------------------------------*/

typedef struct get_port_state_cmd_req {
	uchar	command;
	uchar	picmg_id;		/* PICMG Identifier */
#ifdef BF_MS_FIRST
	/* Channel being queried.
	   [07:06] - Interface
	   [05:00] - Channel Number */
	uchar	interface:2,
		channel_number:6;
#else
	uchar	channel_number:6,
		interface:2;
#endif
} GET_PORT_STATE_CMD_REQ;


typedef struct get_port_state_cmd_resp {
	uchar	completion_code;	/* Completion Code
					   CCH–If the LED ID contained in
					   the Request data is not present
					   on the FRU. */
	uchar	picmg_id;		/* PICMG Identifier. Indicates that 
					   this is a PICMG®-defined group
					   extension command. A value of 
					   00h shall be used. */
	LINK_INFO link_info1;		/* Link Info 1. LS Byte first. Optional. 
					   Describes information about Link one
					   on the specified Channel. If this set of
					   bytes is not provided, the Channel 
					   does not have any Links on the Channel.
						[31:24] — Link Grouping ID
						[23:20] — Link Type Extension
						[19:12] — Link Type
						[11] - Port 3 Bit Flag
						[10] - Port 2 Bit Flag
						[09] - Port 1 Bit Flag
						[08] - Port 0 Bit Flag
						[07:06] - Interface. See interface types below.
						[05:00] - Channel Number */
	uchar	state1;			/* State 1. Must be present if Link 
					   Info 1 is present. Indicates
					   the current state of the Port(s)
					   on the Channel.
   					     PORT_STATE_ENABLE = 1
					     PORT_STATE_DISABLE = 0 */	
	LINK_INFO link_info2;		/* Link Info 2. LS Byte first. Optional. 
					   Similar to Link Info 1. Used for cases 
					   where a second Link has been established. */
	uchar	state2;			/* State 2. Similar to State 1. */
	LINK_INFO link_info3;		/* Link Info 3. LS Byte first. Optional. 
					   Similar to Link Info 1. Used for cases
					   where a third Link has been established. */
	uchar	state3;			/* State 3. Similar to State 1. */
	LINK_INFO link_info4;		/* Link Info 4. LS Byte first. Optional. 
					   Similar to Link Info 1.
					   Used for cases where a fourth Link
					   has been established. */
	uchar	state4;			/* State 4. Similar to State 1. */
} GET_PORT_STATE_CMD_RESP;

/* Interface types */
#define BASE_INTERFACE			0	/* Base Interface */
#define FABRIC_INTERFACE		1	/* Fabric Interface */
#define UPDATE_CHANNEL_INTERFACE	2	/* Update Channel Interface */
						/* 3 = Reserved */

/*----------------------------------------------------------------------*/
/*			Bused Resource Control command 			*/
/*----------------------------------------------------------------------*/


typedef struct bused_resource_control_cmd_req {
	uchar	command;
	uchar	picmg_id;		/* PICMG Identifier. Indicates that 
					   this is a PICMG®-defined group
					   extension command. A value of 
					   00h shall be used. */

	uchar	command_types;			/* Command Types for Shelf Manager to Board
					   0 = Query, ask a Board if it has control 
					   of the bus
					   1 = Release, request a Board to release 
					   control of the bus when no longer in-use
					   2 = Force, command a Board to release
					   control of bus immediately. For
					   the Metallic Test Bus, the Board must 
					   cease any and all use of the bus
					   (i.e., disconnect from the bus). For
					   the Synchronization Clock group, the
					   Board must stop driving the bus 
					   (receiving is not governed by EKeying).
					   3=Bus Free, inform a Board that 
					   Requested the bus previously (and
					   was given a Defer) that the bus is 
					   now available.
					   Command Types for Board to Shelf Manager
					   0 = Request, request to seize control 
					   of the bus
					   1 = Relinquish, indicate to the Shelf
					   Manager that the Board has relinquished 
					   control of the bus and that use of 
					   the bus by all Boards in its application
					   has ceased. Indicates that the Shelf 
					   Manager can reassign control of the bus.
					   2 = Notify, notify the Shelf Manager
					   that control of the bused resource
					   has been transferred to this Board 
					   from another authorized Board
					   All other values reserved */
	uchar	bused_resource_id;	/* Bused Resource ID
						0 = Metallic Test Bus pair #1
						1 = Metallic Test Bus pair #2
						2 = Synch clock group 1 (CLK1A and CLK1B pairs)
						3 = Synch clock group 2 (CLK2A and CLK2B pairs)
						4 = Synch clock group 3 (CLK3A and CLK3B pairs)
						All other values reserved */
} BUSED_RESOURCE_CONTROL_CMD_REQ;

typedef struct bused_resource_control_cmd_resp {
	uchar	completion_code;	/* Completion Code
					   CCH–If the LED ID contained in
					   the Request data is not present
					   on the FRU. */
	uchar	picmg_id;		/* PICMG Identifier. Indicates that 
					   this is a PICMG®-defined group
					   extension command. A value of 
					   00h shall be used. */
	uchar	status;			/* Status. See Table 3-45, 
					   “Bused Resource Control command status,” for
					   values. Any values not specified in the table 
					   are reserved. */
} BUSED_RESOURCE_CONTROL_CMD_RESP;

/* Table 3-45 Bused Resource Control command status 
Command
direction
Command
type Status Meaning
Shelf Manager
to Board Query 0 = In Control Shelf Manager queries if the Board is using the resource and
the Board acknowledges it is in use.
1 = No Control Shelf Manager queries if the Board is using the resource and
the Board denies that it has control.
Release 0 = Ack
Shelf Manager requests the Board to Release control of the
resource and the Board acknowledges it will do so when
operations are complete. Bus is not available to be reassigned
until the Board sends a Relinquish message.
1 = Refused
Shelf Manager requests the Board to Release the resource and
the Board acknowledges control but cannot release the
resource without disrupting operations. Board does not release
control.
2 = No Control Shelf Manager requests the Board to Release the resource and
the Board denies that it has control.
Force 0 = Ack
Shelf Manager commands the Board to immediately relinquish
the resource and the Board acknowledges it must do so.
Receipt and acknowledgement of this command may trigger
additional application specific commands or actions needed to
release the bus. The Shelf Manager cannot reassign control of
the bus until the Board sends a Relinquish message.
1 = No Control Shelf Manager requests the Board to immediately relinquish the
resource and the Board denies that it has control.
Bus Free 0 = Accept
Shelf Manager informs the Board that the resource has become
free and may be taken by the Board and the Board accepts
control of the resource.
1 = Not Needed
Shelf Manager informs the Board that the resource has become
free, but the Board is not interested in controlling the resource
any longer.
Board to Shelf
Manager Request 0 = Grant Board requests and Shelf Manager grants control of the
resource to the Board.
1 = Busy
Board requests control but Shelf Manager knows the resource
is currently assigned to another Board. The Shelf Manager will
make a Release request to the Board currently in control of the
resource. The Board must try again later to see if it can get
access to the resource.
2 = Defer
Board requests control and the resource is assigned to another
Board. The Shelf Manager responds with Defer then it will send
a Bus Free message to the Board when the resource becomes
free at a later time. This allows the Board to skip the polling that
is required by the Busy response.
3 = Deny
Board requests and Shelf Manager denies (i.e., rejects) control
of the resource to the Board. In this case, the Shelf Manager
does not request the Board in control to release the resource.
Relinquish 0 = Ack Shelf Manager acknowledges that the Board relinquished
control of the bused resource and is no longer using the bus.
1 = Error
Shelf Manager acknowledges that the Board relinquished
control of the resource and is no longer using the bus, but the
Shelf Manager believes that the Board should not have been in
control of the resource. (Optional.)
Notify 0 = Ack Shelf Manager acknowledges that the Board now has control of
the bused resource.
1 = Error
Shelf Manager acknowledges that the Board now has control of
the bused resource, but the Shelf Manager believes that the
Board should not have been given control of the resource.
(Optional.)
2 = Deny
Shelf Manager denies (i.e., rejects) control of the resource by
the Board. In this case, the Board stops using the resource
immediately.
*/

/*----------------------------------------------------------------------*/
/*	Get Sensor Reading command (Physical IPMB-0 Sensor)		*/
/*----------------------------------------------------------------------*/

typedef struct get_sensor_reading_ipmb_cmd_req {
	uchar	command;
	uchar	sensor_number;		/* Sensor Number (FFh = reserved) */
} GET_SENSOR_READING_IPMB_CMD_REQ;

typedef struct get_sensor_reading_ipmb_cmd_resp {
	uchar	completion_code;
#ifdef BF_MS_FIRST
	uchar	ipbm_b_override_state:1,	/* [7] – IPMB B Override State
						   0b = Override state, bus isolated
						   1b = Local Control state - IPM 
						   Controller determines state of bus. */
		ipmb_b_local_status:3,		/* [6:4] = IPMB B Local Status */
#define IPMB_STATUS_NO_FAILURE		0x0	/* 0h = No Failure. Bus enabled if no override in effect. */
#define IPMB_STATUS_UNABLE_CLOCK_HI	0x1	/* 1h = Unable to drive clock HI */
#define IPMB_STATUS_UNABLE_DATA_HI	0x2	/* 2h = Unable to drive data HI  */
#define IPMB_STATUS_UNABLE_CLOCK_LO	0x3	/* 3h = Unable to drive clock LO */
#define IPMB_STATUS_UNABLE_DATA_LO	0x4	/* 4h = Unable to drive data LO */
#define IPMB_STATUS_CLOCK_LO_TIMEOUT	0x5	/* 5h = Clock low timeout */
#define IPMB_STATUS_UNDER_TEST		0x6	/* 6h = Under test (the IPM 
						        Controller is attempting to 
						        determine if it is causing a
						        bus hang). */
#define IPMB_STATUS_UNDIAG_COM_FAILURE	0x7	/* 7h = Undiagnosed Communications Failure */
		ipmb_a_override_state:1,	/* [3] – IPMB A Override State
						   0b = Override state, bus isolated
						   1b = Local Control state - IPM 
						   Controller determines state of bus. */
		ipmb_a_local_status:3;		/* [2:0] = IPMB A Local Status (see IPMB_STATUS above) */
#else
	uchar	ipmb_a_local_status:3,
		ipmb_a_override_state:1,
		ipmb_b_local_status:3,
		ipbm_b_override_state:1;
#endif
	uchar	std_ipmi_byte;			/* Standard IPMI byte (see “Get Sensor Reading” 
						   in IPMI specification)
				[7] – 0b = All Event Messages disabled from this sensor
				[6] – 0b = Sensor scanning disabled
				[5] – 1b = Initial update in progress. 
				This bit is set to indicate that a
				“Re-arm Sensor Events” or “Set Event Receiver” command has
				been used to request an update of the sensor status, and that
				update has not occurred yet. Software should use this bit to avoid
				getting an incorrect status while the first sensor update is in
				progress. This bit is only required if it is possible for the controller
				to receive and process a “Get Sensor Reading” or “Get Sensor
				Event Status” command for the sensor before the update has
				completed. This is most likely to be the case for sensors, such as
				fan RPM sensors, that may require seconds to accumulate the
				first reading after a re-arm.
				[4:0] – Reserved. Ignore on read. */
#ifdef BF_MS_FIRST
	uchar	:4,					/* [7:4] – Reserved. Write as 0h, ignore on read */
		ipmb_a_enabled_ipmb_b_enabled:1,	/* [3] 1b = IPMB A enabled, IPMB-B enabled */
		ipmb_a_disabled_ipmb_b_enabled:1,	/* [2] 1b = IPMB A disabled, IPMB-B enabled */
		ipmb_a_enabled_ipmb_b_disabled:1,	/* [1] 1b = IPMB-A enabled, IPMB-B disabled */
		ipmb_a_disabled_ipmb_b_disabled:1;	/* [0] 1b = IPMB A disabled, IPMB-B disabled */
#else
	uchar	ipmb_a_disabled_ipmb_b_disabled:1,	/* [0] 1b = IPMB A disabled, IPMB-B disabled */
		ipmb_a_enabled_ipmb_b_disabled:1,	/* [1] 1b = IPMB-A enabled, IPMB-B disabled */
		ipmb_a_disabled_ipmb_b_enabled:1,	/* [2] 1b = IPMB A disabled, IPMB-B enabled */
		ipmb_a_enabled_ipmb_b_enabled:1,	/* [3] 1b = IPMB A enabled, IPMB-B enabled */
		:4;					      
#endif
	uchar 	reserved2;			/* [7:0] – Optional/Reserved. If provided,
						   write as 80h (IPMI restriction). Ignore on read. */
} GET_SENSOR_READING_IPMB_CMD_RESP;

/*----------------------------------------------------------------------*/
/*		Physical IPMB-0 Status Change Event Message 		*/
/*----------------------------------------------------------------------*/

typedef struct ipmb_status_change_event_msg {
	uchar	event_msg_rev;		/* Event message Rev = 04h (IPMI v1.5) */
	uchar	sensor_type;		/* Sensor Type = F1h (Physical IPMB-0 Sensor) */
	uchar	sensor_number;		/* Sensor Number = XXh (Implementation-specific) */
#ifdef BF_MS_FIRST
	uchar 	event_direction:1,	/* Event Direction [7] - 0b (Assertion) */
		event_type:7;		/* Event Type [6:0] = 6Fh (Generic Availability) */
#else
	uchar	event_type:7,
		event_direction:1;
#endif
#ifdef BF_MS_FIRST
	uchar	event_data1:4,		/* Event Data 1 [7:4] = Ah (OEM code 
					   in Event Data 2, OEM code in Event Data 3) */
		offset:4;		/* [3:0] = Offset
					   00h – IPMB-A disabled, IPMB-B disabled
					   01h – IPMB-A enabled, IPMB-B disabled
					   02h – IPMB-A disabled, IPMB-B enabled
					   03h – IPMB-A enabled, IPMP-B enabled */
#else
	uchar	offset:4,
		event_data1:4;
#endif
#ifdef BF_MS_FIRST
	uchar	event_data2:4,		/* Event Data 2 [7:4] = Channel Number. For 
					   AdvancedTCA®, this will typically
					   be 0h to indicate IPMB-0 */
		:4;			/* [3:0] = Reserved */
#else
	uchar	:4,
		event_data2:4;
#endif
#ifdef BF_MS_FIRST
	uchar 	ipmb_b_override_state:1,/* Event Data 3 [7] – IPMB B Override State
					   0b = Override state, bus isolated
					   1b = Local Control state – IPM 
					   Controller determines state of bus. */
		ipmb_b_local_status:3,	/* [6:4] = IPMB B Local Status (see IPMB_STATUS above) */
		ipmb_a_override_state:1,/* [3] – IPMB A Override Status
					   0b = Override status, bus isolated
					   1b = Local Control state – IPM 
					   Controller determines state of bus. */
		ipmb_a_local_status:3;	/* [2:0] = IPMB A Local Status (see IPMB_STATUS above) */
#else
	uchar	ipmb_a_local_status:3,
		ipmb_a_override_state:1,
		ipmb_b_local_status:3,
		ipmb_b_override_state:1;
#endif
} IPMB_STATUS_CHANGE_EVENT_MSG;
/*		
		Response data 1 Completion Code.
*/

/*----------------------------------------------------------------------*/
/*			Set IPMB State command				*/
/*----------------------------------------------------------------------*/

typedef struct set_ipmb_state_cmd_req {
	uchar	command;
	uchar	picmg_id;		/* PICMG Identifier. Indicates that 
					   this is a PICMG®-defined group
					   extension command. A value of 
					   00h shall be used. */
#ifdef BF_MS_FIRST
	uchar	ipmb_a_state:1,		/* IPMB-A State. Indicates the state of 
					   IPMB-A. Options are:
					   [0] - IPMB-A State
					   0h = Override state - Isolate (disable)
					   1h = Local Control state - IPM 
					   Controller determines state of IPMB-A */
		ipmb_a_link_id:7;	/* [1:7] - IPMB-0 Link Identification
					   00h = Select All IPMB-0 Links (also 
					   the value for bused IPMB-0)
					   01h - 5Fh = IPMB-0 Link Number 1 to 95
					   60h - 7Eh = reserved
					   If Byte 2 = FFh, command does not change 
					   current state of IPMB-A */
#else
	uchar	ipmb_a_link_id:7,
		ipmb_a_state:1;
#endif
#ifdef BF_MS_FIRST
	uchar	ipmb_b_state:1,		/* IPMB-B State. Indicates the state of 
					   IPMB-B. Options are:
					   [0] - IPMB-B State
					   0h = Override state - Isolate (disable)
					   1h = Local Control state - IPM 
					   Controller determines state of IPMB-B */
		ipmb_b_link_id:7;	/* [1:7] - IPMB-0 Link Identification
					   00h = Select All IPMB-0 Links (also 
					   the value for bused IPMB-0)
					   01h - 5Fh = IPMB-0 Link Number 1 to 95
					   60h - 7Eh = reserved
					   If Byte 2 = FFh, command does not change 
					   current state of IPMB-B */
#else
	uchar	ipmb_b_link_id:7,
		ipmb_b_state:1;
#endif
} SET_IPMB_STATE_CMD_REQ;

typedef struct set_ipmb_state_cmd_resp {
	uchar	completion_code;	/* Completion Code */
	uchar	picmg_id;		/* PICMG Identifier. Indicates that 
					   this is a PICMG®-defined group
					   extension command. A value of 
					   00h shall be used. */
} SET_IPMB_STATE_CMD_RESP;

/*----------------------------------------------------------------------*/
/*			Compute Power Properties command 		*/
/*----------------------------------------------------------------------*/

typedef struct compute_power_properties_cmd_req {
	uchar	command;
	uchar	picmg_id;		/* PICMG Identifier. Indicates that 
					   this is a PICMG®-defined group
					   extension command. A value of 
					   00h shall be used. */
	uchar	fru_dev_id;		/* FRU Device ID. Indicates an 
					   individual FRU device to lock 
					   in the power properties. */
} COMPUTE_POWER_PROPERTIES_CMD_REQ;

typedef struct compute_power_properties_cmd_resp {
	uchar	completion_code;	/* Completion Code */
	uchar	picmg_id;		/* PICMG Identifier. Indicates that 
					   this is a PICMG®-defined group
					   extension command. A value of 
					   00h shall be used. */
	uchar	num_slots;		/* Number of Spanned Slots. Indicates 
					   the total number of Slots spanned 
					   by the Board. This shall be 1 for 
					   a Board that inserts into a single
					   Slot and any non-Board FRU. */
	uchar	ipm_location;		/* IPM Controller Location. 
					   When a Board spans more than one 
					   Slot (a multi-Slot Board), the Shelf 
					   Manager needs to know which Slot
					   contains the IPM Controller. This 
					   number shall treat the left most Slot
					   location of the multi-Slot Board 
					   as 0 and increment by single units
					   moving to the right. This value shall
					   be 0 for a Board that inserts into
					   a single Slot and any non-Board FRU. 
					   This value shall represent the
					   location of the Zone 1 connection 
					   for the Board. */
} COMPUTE_POWER_PROPERTIES_CMD_RESP;


/*----------------------------------------------------------------------*/
/*			Get Power Level command 			*/
/*----------------------------------------------------------------------*/

typedef struct get_power_level_cmd_req {
	uchar	command;
	uchar	picmg_id;		/* PICMG Identifier. Indicates that 
					   this is a PICMG®-defined group
					   extension command. A value of 
					   00h shall be used. */
	uchar	fru_dev_id;		/* FRU Device ID. Indicates an
					   individual FRU device to query. */
	uchar	power_type;		/* Power Type. TODO
00h = Steady state power draw levels
01h = Desired steady state draw levels
02h = Early power draw levels
03h = Desired early levels
All other values reserved.
*/
} GET_POWER_LEVEL_CMD_REQ;


typedef struct get_power_level_cmd_resp {
	uchar	completion_code;	/* Completion Code */
	uchar	picmg_id;		/* PICMG Identifier. Indicates that 
					   this is a PICMG®-defined group
					   extension command. A value of 
					   00h shall be used. */
#ifdef BF_MS_FIRST
	uchar	dyn_power_config:1,	/* Properties. This holds properties about the FRU.
					   [7] Dynamic Power Configuration. Set 
					   to 1b if the FRU supports dynamic 
					   reconfiguration of power (i.e., 
					   the Payload service is uninterrupted 
					   when power levels are altered). */
		:2,			/* [6:5] Reserved. */
		power_level:5;		/* [4:0] Power Level. When requesting 
					   “Steady state power draw levels”, 
					   this represents the power level of 
					   the FRU. When requesting desired power 
					   levels, this represents the power 
					   level the FRU would like to have. */
#else
	uchar	power_level:5,
		:2,
		dyn_power_config:1;
#endif
	uchar	delay_to_stable_power;	/* Delay to Stable Power. This byte 
					   shall be written as 00h when Power
					   Type is “Steady state power draw levels”
					   or “Desired steady state draw levels”. 
					   Otherwise, this byte shall contain the
					   amount of time before power transitions
					   from the early power levels to the normal
					   levels. This value is returned in tenths
					   of a second. */
	uchar	power_multiplier;	/* Power Multiplier. This defines the 
					   number of tenths of a Watt by
					   which to multiply all values held 
					   in bytes 6 and beyond. This is
					   included to allow a FRU that spans
					   multiple locations to specify
					   higher power draws. For instance, 
					   if this byte holds a 50, then bytes 6
					   and beyond specify 5 W increments. */
	uchar	power_draw[20];		/* Power Draw[1..N]. The first entry 
					   reflects the lowest level of power
					   (minimum power level) used by the FRU’s Payload.
					   Power Draw[Max]. The last entry 
					   reflects the highest level of power
					   used by the FRU’s Payload. Everything
					   is powered full capacity. Any bytes
					   past the 6th byte are optional. The 
					   maximum value of N is 25 (which 
					   corresponds to a Max value of 20) 
					   due to IPMI message size restrictions. */

} GET_POWER_LEVEL_CMD_RESP;

/*----------------------------------------------------------------------*/
/*			Set Power Level command 			*/
/*----------------------------------------------------------------------*/

typedef struct set_power_level_cmd_req {
	uchar	command;
	uchar	picmg_id;		/* PICMG Identifier. Indicates that 
					   this is a PICMG®-defined group
					   extension command. A value of 
					   00h shall be used. */
	uchar	fru_dev_id;		/* FRU Device ID. Indicates an
					   individual FRU device to query. */
	uchar	power_level;		/* Power Level.
					   00 = Power off, only the mandatory
					   support circuitry is running (e.g.,
					   IPM Controller, fans running to cool
					   management circuitry, PEMs, FRU devices).
					   01h - 14h = Select the power level, 
					   if available.
					   FFh = Do not change current power level.
					   All other values reserved. */
	uchar	set_present_level;	/* Set Present Levels to Desired Levels
					   00h = Do not change present levels
					   01h = Copy desired levels to present levels.
					   All other values reserved. */
} SET_POWER_LEVEL_CMD_REQ;

typedef struct set_power_level_cmd_resp {
	uchar	completion_code;	/* Completion Code */
	uchar	picmg_id;		/* PICMG Identifier. Indicates that 
					   this is a PICMG®-defined group
					   extension command. A value of 
					   00h shall be used. */
} SET_POWER_LEVEL_CMD_RESP;

/*----------------------------------------------------------------------*/
/*			Get Fan Speed Properties command		*/
/*----------------------------------------------------------------------*/
typedef struct get_fan_speed_properties_cmd_req {
	uchar	command;
	uchar	picmg_id;		/* PICMG Identifier. Indicates that 
					   this is a PICMG®-defined group
					   extension command. A value of 
					   00h shall be used. */
	uchar	fru_dev_id;		/* FRU Device ID. Indicates an
					   individual FRU device to query. */
} GET_FAN_SPEED_PROPERTIES_CMD_REQ;

typedef struct get_fan_speed_properties_cmd_resp {
	uchar	completion_code;	/* Completion Code */
	uchar	picmg_id;		/* PICMG Identifier. Indicates that 
					   this is a PICMG®-defined group
					   extension command. A value of 
					   00h shall be used. */

	uchar	min_speed_level;	/* Minimum Speed Level. This field 
					   describes the minimum setting that
					   is accepted by the Set Fan Level 
					   command. */
	uchar	max_speed_level;	/* Maximum Speed Level. This field 
					   describes the maximum setting that
					   is accepted by the Set Fan Level 
					   command. */
	uchar	norm_operating_level;	/* Normal Operating Level. This 
					   field represents the default 
					   normal fan speed recommended by 
					   the fan manufacturer. */
	uchar	fan_tray_prop;		/* Fan Tray Properties. This field 
					   holds properties of the Fan Tray.
					   [7] – Local Control Mode Supported. 
					   This bit is set to 1b if the Fan
					   Tray supports automatic adjustment 
					   of the fan speed.
					   [6:0] – Reserved. */
#define	FAN_TRAY_LOCAL_CONTROL_MODE_SUPPORTED	0x80
#define FAN_TRAY_LOCAL_CONTROL_MODE_UNSUPPORTED	0x00
} GET_FAN_SPEED_PROPERTIES_CMD_RESP;

/*----------------------------------------------------------------------*/
/*			Get Fan Level command				*/
/*----------------------------------------------------------------------*/


typedef struct get_fan_level_cmd_req {
	uchar	command;
	uchar	picmg_id;		/* PICMG Identifier. Indicates that 
					   this is a PICMG®-defined group
					   extension command. A value of 
					   00h shall be used. */
	uchar	fru_dev_id;		/* FRU Device ID. Indicates an
					   individual FRU device to query. */
} GET_FAN_LEVEL_CMD_REQ;

typedef struct get_fan_level_cmd_resp {
	uchar	completion_code;	/* Completion Code */
	uchar	picmg_id;		/* PICMG Identifier. Indicates that 
					   this is a PICMG®-defined group
					   extension command. A value of 
					   00h shall be used. */
	uchar	override_fan_level;	/* Override Fan Level. Indicates the 
					   fan level that the Shelf Manager
					   has selected, which must be in the
					   range Minimum Speed Level to
					   Maximum Speed Level, or equal 
					   to FEh or FFh.
					   FEh = Fan has been placed in 
					   “Emergency Shut Down” by the 
					   Shelf Manager
					   FFh = Fan operating in Local 
					   Control mode */
	uchar	local_control_fan_level;/* Local Control Fan Level - This 
					   byte is optional if the Fan Tray does
					   not support Local Control. When 
					   present, this byte always indicates
					   the Local Control fan level as 
					   determined by the Fan Tray controller.
					   When Local Control is supported, 
					   the actual fan level is: 
					   1) the value of this byte if 
					   Override Fan Level is FFh, or
					   2) the larger of this byte and
					   Override Fan Level, or 
					   3) Emergency Shut Down if either
					   Override Fan Level or this byte 
					   has a value FEh. */
} GET_FAN_LEVEL_CMD_RESP;

/*----------------------------------------------------------------------*/
/*			Set Fan Level command				*/
/*----------------------------------------------------------------------*/


typedef struct set_fan_level_cmd_req {
	uchar	command;
	uchar	picmg_id;		/* PICMG Identifier. Indicates that 
					   this is a PICMG®-defined group
					   extension command. A value of 
					   00h shall be used. */
	uchar	fru_dev_id;		/* FRU Device ID. Indicates the FRU 
					   device for which the command is
					   intended. */
	uchar	fan_level;		/* Fan Level. To be accepted, this 
					   value shall be: 1) greater than or
					   equal to Minimum Speed Level and
					   less than or equal to Maximum
					   Speed Level, or 2) FEh (Emergency 
					   Shut Down) or 3) FFh (Local Control). */
} SET_FAN_LEVEL_CMD_REQ;

typedef struct set_fan_level_cmd_resp {
	uchar	completion_code;	/* Completion Code */
	uchar	picmg_id;		/* PICMG Identifier. Indicates that 
					   this is a PICMG®-defined group
					   extension command. A value of 
					   00h shall be used. */
} SET_FAN_LEVEL_CMD_RESP;

/*----------------------------------------------------------------------*/
/*			Renegotiate Power command 			*/
/*----------------------------------------------------------------------*/

typedef struct renegotiate_power_cmd_req {
	uchar	command;
	uchar	picmg_id;		/* PICMG Identifier. Indicates that 
					   this is a PICMG®-defined group
					   extension command. A value of 
					   00h shall be used. */
	uchar	fru_dev_id;		/* Requesting FRU Device ID. This 
					   byte is optional. If this is present,
					   designates which FRU Device ID 
					   wishes to have power levels
					   renegotiated. If this byte is not
					   present, the Shelf Manager
					   renegotiates with all FRUs
					   under the requesting IPM Controller. */
} RENEGOTIATE_POWER_CMD_REQ;

typedef struct renegotiate_power_cmd_resp {
	uchar	completion_code;	/* Completion Code */
	uchar	picmg_id;		/* PICMG Identifier. Indicates that 
					   this is a PICMG®-defined group
					   extension command. A value of 
					   00h shall be used. */
} RENEGOTIATE_POWER_CMD_RESP;


/*----------------------------------------------------------------------*/
/*			Get IPMB Link Info command 			*/
/*----------------------------------------------------------------------*/

typedef struct get_ipmb_link_info_cmd_req {
	uchar	command;
	uchar	picmg_id;		/* PICMG Identifier. Indicates that 
					   this is a PICMG®-defined group
					   extension command. A value of 
					   00h shall be used. */

	uchar	qualifier;	/* 00 = Byte 3 of the request contains an IPMB-0 Link Number (1-95).
				   01 = Byte 3 of the request contains an IPMB-0 Sensor Number. */
	uchar	link_info_key;	/* Link Info Key. The content of this byte depends
				   on the value of byte 2 of the request. */
} GET_IPMB_LINK_INFO_CMD_REQ;

typedef struct get_ipmb_link_info_cmd_resp {
	uchar	completion_code;	/* Completion Code */
	uchar	picmg_id;		/* PICMG Identifier. Indicates that 
					   this is a PICMG®-defined group
					   extension command. A value of 
					   00h shall be used. */
	uchar	link_number;		/* IPMB-0 Link Number */
	uchar	sensor_number;		/* IPMB-0 Sensor Number */
} GET_IPMB_LINK_INFO_CMD_RESP;

#define IPMB_LINK_INFO_QUAL_LINK_NUMBER		0
#define IPMB_LINK_INFO_QUAL_SENSOR_NUMBER	1

#define EVT_DATA2_UNSPECIFIED		0x0
#define EVT_DATA2_TRIGGER_READING	0x1
#define EVT_DATA2_OEM_CODE		0x2
#define EVT_DATA2_SENSOR_SPECIFIC	0x3

#define EVT_DATA3_UNSPECIFIED		0x0
#define EVT_DATA3_TRIGGER_THRESHOLD	0x1
#define EVT_DATA3_OEM_CODE		0x2
#define EVT_DATA3_SENSOR_SPECIFIC	0x3

#define UPPER_NON_CRITICAL_GOING_HIGH	0x7
#define UPPER_CRITICAL_GOING_HIGH	0x9
#define UPPER_NON_RECOVERABLE_GOING_HIGH	0xB

/*----------------------------------------------------------------------*/
/*			Temperature Event Message			*/
/*----------------------------------------------------------------------*/

typedef struct fru_temperature_event_msg_req {
	uchar	command;
	uchar	evt_msg_rev;		/* Event Message Rev = 04h (IPMI v1.5) */
	uchar	sensor_type;		/* Sensor Type = 01h (Threshold)  */
	uchar	sensor_number;		/* Sensor Number = 0xxh (Implementation specific) */
	uchar	evt_direction;		/* Event Direction (bit7) = 0b (Assertion) and 1b (Deassertion)
					   Event Type [6:0] = 01h (Temperature) */
	/* Event Data 1 */
	uchar	evt_data2_qual:2,	/* [7:6] - Event data 2 qualifier
					   00b = Unspecified Event Data 2
					   01b = Trigger reading in Event Data 2
					   10b = OEM code in Event Data 2
					   11b = Sensor-specific event extension 
						 code in Event Data 2 */
		evt_data3_qual:2,	/* [5:4] - Event data 3 qualifier 
					   00b = Unspecified Event Data 3
					   01b = Trigger threshold value in Event Data 3
					   10b = OEM code in Event Data 3
					   11b = Sensor-specific event extension code in Event Data 3 */
		
		evt_reason:4;		/* [3:0] - reason for the event
					   00h - 05h = IPMI Lower Thresholds
					   07h = Upper Non-critical (minor) - going high
					   09h = Upper Critical (major) - going high
					   0Bh = Upper Non-recoverable (critical) - going high */
	/* Event Data 2 */
	uchar	temp_reading;		/* Event Data 2 - reading that triggered event, 
					   FFh or not present if unspecified. */
	/* Event Data 3 */
	uchar	threshold;		/* Event Data 3 - threshold value that triggered 
					   event, FFh or not present if unspecified. 
					   If present, byte 2 must be present. */
} FRU_TEMPERATURE_EVENT_MSG_REQ;

#define ATCA_CMD_GET_PICMG_PROPERTIES_STR		"Get PICMG Properties"
#define ATCA_CMD_GET_ADDRESS_INFO_STR			"Get Address Info"
#define ATCA_CMD_GET_SHELF_ADDRESS_INFO_STR		"Shelf Address Info"
#define ATCA_CMD_SET_SHELF_ADDRESS_INFO_STR		"Set Shelf Address Info"
#define ATCA_CMD_FRU_CONTROL_STR			"FRU Control"
#define ATCA_CMD_GET_FRU_LED_PROPERTIES_STR		"Get FRU LED Properties"
#define ATCA_CMD_GET_LED_COLOR_STR			"Get LED Color Capabilities"
#define ATCA_CMD_SET_FRU_LED_STATE_STR			"Set FRU LED State"
#define ATCA_CMD_GET_FRU_LED_STATE_STR			"Get FRU LED State"
#define ATCA_CMD_SET_IPMB_STATE_STR			"Set IPMB State"
#define ATCA_CMD_SET_FRU_ACTIVATION_POLICY_STR		"Set FRU Activation Policy"
#define ATCA_CMD_GET_FRU_ACTIVATION_POLICY_STR		"Get FRU Activation Policy"
#define ATCA_CMD_SET_FRU_ACTIVATION_STR			"Set FRU Activation"
#define ATCA_CMD_GET_DEVICE_LOCATOR_REC_ID_STR		"Get Device Locator Record ID"
#define ATCA_CMD_SET_PORT_STATE_STR			"Set Port State"
#define ATCA_CMD_GET_PORT_STATE_STR			"Get Port State"
#define ATCA_CMD_COMPUTE_POWER_PROPERTIES_STR		"Compute Power Properties"
#define ATCA_CMD_SET_POWER_LEVEL_STR			"Set Power Level"
#define ATCA_CMD_GET_POWER_LEVEL_STR			"Get Power Level"
#define ATCA_CMD_RENEGOTIATE_POWER_STR			"Renegotiate Power"
#define ATCA_CMD_GET_FAN_SPEED_PROPERTIES_STR		"Get Fan Speed Properties"
#define ATCA_CMD_SET_FAN_LEVEL_STR			"Set Fan Level"
#define ATCA_CMD_GET_FAN_LEVEL_STR			"Get Fan Level"
#define ATCA_CMD_BUSED_RESOURCE_STR			"Bused Resource"
#define ATCA_CMD_GET_IPMB_LINK_INFO_STR			"Get IPMB Link Info"


	//   IPM Device “Global” Commands
#define	IPMI_CMD_GET_DEVICE_ID_STR		"Get Device ID"
//#define		"Cold Reset"
//#define		"Warm Reset"
#define	IPMI_CMD_GET_SELF_TEST_RESULTS_STR	"Get Self Test Results"
//#define		"Manufacturing Test On"
//#define		"Set ACPI Power State"
//#define		"Get ACPI Power State"
//#define		"Get Device GUID"
//#define		"Broadcast Get Device ID"
#define	IPMI_CMD_RESET_WATCHDOG_TIMER_STR	"Reset Watchdog Timer"
#define	IPMI_CMD_SET_WATCHDOG_TIMER_STR		"Set Watchdog Timer"
#define	IPMI_CMD_GET_WATCHDOG_TIMER_STR		"Get Watchdog Timer"
//#define		"Set BMC Global Enables"
//#define		"Get BMC Global Enables"
//#define		"Clear Message Flags"
//#define		"Get Message Flags"
//#define		"Enable Message Channel Receive"
//#define		"Get Message"
#define	IPMI_CMD_SEND_MESSAGE_STR		"Send Message"
//#define		"Read Event Message Buffer"
//#define		"Get BT Interface Capabilities"
//#define		"Master Write-Read"
//#define		"Get System GUID"
//#define		"Get Channel Authentication Capabilities"
//#define		"Get Session Challenge"
//#define		"Activate Session"
//#define		"Set Session Privilege Level"
//#define		"Close Session"
//#define		"Get Session Info"
//#define		"Get AuthCode"
//#define		"Set Channel Access"
//#define		"Get Channel Access"
//#define		"Get Channel Info"
//#define		"Set User Access"
//#define		"Get User Access"
//#define		"Set User Name"
//#define		"Get User Name"
//#define		"Set User Password"


typedef struct str_lst{
	int id;
	char *str;
} STR_LST;	



void ipmi_process_pkt( IPMI_WS *ws ); 
void ipmi_initialize( void );
unsigned char ipmi_get_next_seq( unsigned char *seq );
unsigned char ipmi_calculate_checksum( unsigned char *ptr, int numchar );
