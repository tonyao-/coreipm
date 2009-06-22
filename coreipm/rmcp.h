/*
-------------------------------------------------------------------------------
coreIPM/rmcp.h

Author: Gokhan Sozmen
-------------------------------------------------------------------------------
Copyright (C) 2007-2009 Gokhan Sozmen
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

// RMCP Port Numbers
#define PRIMARY_RMCP_PORT	"623"
#define SECONDARY_RMCP_PORT	"664"

#define RMCP_MSG_NORMAL 0	// used by IPMI
#define RMCP_MSG_ACK	1

#ifndef uchar
#define uchar unsigned char
#endif

// RMCP Message Header
typedef struct rmcp_msg_hdr {
	uchar	version;	// Version, 06h = RMCP Version 1.0
	uchar	reserved;	// Reserved 00h
	uchar	seq_num;	// Sequence Number
	/* Class of Message. All messages of class ASF (6) conform to
	   the formats defined in this specification and can be extended
	   via an OEM IANA. */
#ifdef BF_MS_FIRST
	uchar	packet_type:1,	/* Bit 7 RMCP ACK
				   0 - Normal RMCP message
				   1 - RMCP ACK message */
		:2,		// Bit 6:5 Reserved
		msg_class:5;	/* Bit 4:0 Message Class
				   0-5 = Reserved
				   6 = ASF
				   7 = IPMI
				   8 = OEM defined
				   all other = Reserved */
#else
	uchar	msg_class:5,
		:2,
		packet_type:1;
#endif
	uchar	data;		// RMCP Data. Variable data based class of message
} RMCP_MSG_HDR;

typedef struct ipmi_session_hdr {
	/* Auth Type / Format */
	uchar	:4,		// [7:4] - reserved
		auth_type:4;	/* [3:0] - Authentication Type / Format
					0h = none
					1h = MD2
					2h = MD5
					3h = reserved
					4h = straight password / key
					5h = OEM proprietary
					6h = Format = RMCP+ (IPMI v2.0 only)
					all other = reserved */
	uchar	session_seq_num[4]; /* Session Sequence Number. For IPMI v2.0
				   “RMCP+” there are separate sequence numbers 
				   tracked for authenticated and unauthenticated
				   packets. 0000_0000h is used for packets that 
				   are sent ‘outside’ of a session. */
	uchar	session_id[4];	/* IPMI v1.5 Session ID. Session ID is
				   0000_0000h for messages that are sent 
				   ‘outside’ of a session. */
	uchar	msg_auth[16];	/* Msg. Auth. Code Code (AuthCode)
				   (not present when Authentication Type set 
				   to ‘none’.) */
	uchar	payload_len;	/* IPMI Msg/Payload length in bytes. 1-based. */
} IPMI_SESSION_HDR;

/*
  IPMI Payload - RMCP

	+------
	| Payload Data (variable size)
	|   For IPMI v2.0: IPMI, SOL, KVM, etc. per Payload Type field.
	+------
*/

/*
  IPMI Payload - RMCP+

	+-------
	| Confidentiality Header (variable size)
	|   For encrypted payloads, based on encryption type for
	|   given payload. The confidentiality header is not
	|   encrypted.
	+------
	| Payload Data (variable size)
	|   For IPMI v2.0: IPMI, SOL, KVM, etc. per Payload Type field.
	+------
	| Confidentiality Trailer (variable size )
	|   For encrypted payloads, based on encryption type for
	|   given payload. The confidentiality trailer is typically 
	|   encrypted along with the Payload Data.
	+------	
*/

#define AUTH_TYPE_NONE	0
#define AUTH_TYPE_MD2	1
#define AUTH_TYPE_MD5	2
#define AUTH_TYPE_STR	4	// straight password/key
#define AUTH_TYPE_OEM	5
#define AUTH_TYPE_RMCPP	6	// RMCP+

typedef struct ipmi_session_hdr_plus {
	/* Auth Type / Format */
	uchar	:4,		// [7:4] - reserved
		auth_type:4;	/* [3:0] - Authentication Type / Format
					AUTH_TYPE_xx
					------------
			   	   	0h = none
					1h = MD2
					2h = MD5
					3h = reserved
					4h = straight password / key
					5h = OEM proprietary
					6h = Format = RMCP+ (IPMI v2.0 only)
					all other = reserved */

	/* Payload Type 1 Payload Type */
	uchar	payload_encrypted:1,	/* [7] - 0b = payload is unencrypted
					         1b = payload is encrypted */
		payload_authenticated:1, /* [6] - 0b = payload is unauthenticated 
					    	       (no AuthCode field)
						  1b = payload is authenticated
						       (AuthCode field is present) */
		payload_type:6;	/* [5:0] = payload type. See Table 13-16, 
				   Payload Type Numbers in IPMI spec. */
	uchar	oem_iana[4];	/* OEM IANA 4 This field is only present
				   when Payload Type = 02h (OEM Explicit)
				   byte 1:3 - OEM IANA
				   byte 4 - reserved */
	uchar	oem_payload_id[2]; /* OEM Payload ID. This field is only present
				   when Payload Type = 02h (OEM Explicit). 
				   The definition and values of this field
				   are specified by the company or body 
				   identified by the OEM IANA field. */
	uchar	session_id[4];	/* IPMI v2.0 RMCP+ Session ID. Session ID is
				   0000_0000h for messages that are sent ‘outside’
				   of a session. */
	uchar	session_seq_num[4]; /* Session Sequence Number. For IPMI v2.0
				   “RMCP+” there are separate sequence numbers 
				   tracked for authenticated and unauthenticated
				   packets. 0000_0000h is used for packets that 
				   are sent ‘outside’ of a session. */
	uchar	payload_len;	/* IPMI Msg/Payload length in bytes. 1-based. */
} IPMI_SESSION_HDR_PLUS;

/*
  IPMI Session Trailer

	+-------
	| Integrity PAD (variable size)
	|   Added as needed to cause the number of bytes in the data range 
	|   covered by the AuthCode (Integrity Data) field to be a multiple 
	|   of 4 bytes (DWORD). If present, each Integrity Pad byte is
	|   set to FFh.
	+-------
	| AuthCode (Integrity Data) (variable size) 
	|   For IPMI v1.5 this field is as specified by Auth Type.
	|   For IPMI v2.0 (RMCP+) if this field is present, then it is
	|   calculated according to the Integrity Algorithm that was
	|   negotiated during the session open process. See Table 13-18, 
	|   Integrity Algorithm Numbers.
	|   This field is absent when the packet is unauthenticated.
	+-------
*/

/*
RMCP ACK messages are not required for IPMI messaging, since IPMI already has 
its own messaging retry policies. In addition, some Network Controllers usable
for IPMI messaging do not automatically generate RMCP ACK messages. In these 
implementations, the BMC would have to generate the RMCP ACK, resulting in 
additional, unnecessary traffic from the BMC. Therefore, RMCP ACK messages 
should not be used for IPMI messaging.

- RMCP messages with class=IPMI must have their RMCP sequence number set to
255 (FFh) to indicate that RMCP ACK messages are not to be generated by the
message receiver.

*/ 

typedef struct rmcp_ack_msg {
	uchar	version;	// Version, 06h = RMCP Version 1.0
	uchar	reserved;	// Reserved 00h
	uchar	seq_num;	// Sequence Number
#ifdef BF_MS_FIRST
	uchar	rmcp_ack:1,	// 1 = RMCP ACK message
		:7;
#else
	uchar	:7,
		rmcp_ack:1;
#endif
} RMCP_ACK_MSG;




