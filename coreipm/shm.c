/*
3.9 Shelf Power and Cooling
---------------------------
Managing power and cooling involves three stages of Shelf status:
Discovery, Normal Operation and Abnormal Operation.

In the Discovery stage, the Shelf Manager collects the following data:
— Shelf Data. 
  
— FRU Consumption. Each FRU must provide information about how much power it (and
any subsidiary components) needs to operate both to initially power on and under steady
state conditions. IPMI commands are defined to provide this data to the Shelf Manager.

— Fan Tray Participation. The Fan Trays in the system can be discovered and queried for
their abilities by the Shelf Manager. Once it understands their abilities, it can control their
speeds during normal and abnormal operation.

Shelf data include the following:

  Shelf Activation and Power Management Record (Table 3-58):

SHELF_ACTIVATION_POWER_RECORD {
	  .
	t_activation_readiness; // number of seconds after system startup
				// that FRUs have to transition to state 
				// M3 and maintain their power up sequence
				// position.
	number_fru_power_records = n;
	  .
	FRU_ACTIVATION_POWER_RECORD p[0] {
		hw_addr;
		fru_dev_id;
		max_fru_power_capability; // in Watts
		sh_mgr_controlled_activation;	// If true, Shelf Manager activates this FRU
		delay_before_next_power_on;	// delay before powering up any other
	       					// FRU after powering this FRU
	}
	  .
	  .
	FRU_ACTIVATION_POWER_RECORD p[n];
}

  Shelf Power Distribution Record(s) (Table 3-55, Table 3-56, Table 3-57):
  • Number of power Feeds
  • Maximum External Available Current
  • Maximum Internal Current
  • Minimum Expected Operating Voltage
  • Feed-to-FRU (Identify which FRU is powered by which Feed.)
  
The Shelf FRU Information may include several Shelf Power Distribution Records
if a single record is not sufficient for describing all Feed-to-FRU mappings.
Any power Feed is always entirely described in a single Shelf Power Distribution
Record. Thus the list of all Feed descriptions is a simple concatenation of the
Power Distribution Map lists of all present records, and the total number of the
power Feeds that supply power to the Shelf is the sum of Number of Power Feeds
fields of all present records.

SHELF_POWER_RECORD feed[0] {
	  .
	number_of_power_feeds = n;
	  .
	POWER_DISTRIBUTION_MAP m[0] {  // one for each feed
		  .
		max_external_current;
		max_internal_current;
		min_expected_operation_voltage;
		number_map_entries = p;
		  .
		FEED_TO_FRU_MAPPING fm[0] {
			hw_id
			fru_dev_id
		} 
		  .
		  .
		FEED_TO_FRU_MAPPING fm[p]  
          .
	  .
	}
	POWER_DISTRIBUTION_MAP m[n] {

	}
}

The Maximum External Available Current field may be set in the field to indicate
the maximum wattage that has been allocated for the Shelf. This can be used to
throttle a Shelf’s use of power on a particular Feed or to indicate the amount
of power that has been routed to that Feed. Note the Shelf Manager will use the
lesser of the values found in the Maximum External Available Current and Maximum
Internal Current fields to determine the Actual Power Available.

The order of the descriptors is important for determining the power on
sequence. The Shelf Manager checks the existence and operational state of each FRU in the order
they are described in the Shelf FRU Information. As the Shelf Manager steps through the FRU
Activation and Power Descriptors table, each FRU that has reached state M3 is powered on

After
powering up a FRU, the Shelf Manager ensures that another FRU is not assigned a non-zero power
level within Delay Before Next Power On tenths of a second. If a FRU for which activation is
controlled by the Shelf Manager has not reached state M3 by the time the Shelf Manager starts
processing its descriptor in the FRU Activation and Power Descriptors table, the Shelf Manager
waits for the earlier of: 1) the FRU transitioning to M3 and 2) Allowance for FRU Activation
Readiness seconds after initial system startup. After the allowance period, if a FRU is not in M3
when its FRU Activation and Power Descriptor is processed, the Shelf Manager proceeds to the
next descriptor without waiting for that FRU.

The Shelf Manager shall reserve power for the management circuitry at each FRU location by
decreasing the Actual Power Available for each Feed by 10 W times the Feed-to-FRU
Mapping Entries Count defined in Table 3-56, “Power Distribution Map.” This is done so that
the power budget is not exceeded when a new FRU is inserted and the management circuitry
automatically powers up.

If a FRU is found at a Hardware
Address for which a FRU Activation and Power Descriptor is not provided, the Shelf Manager
should not apply power to this FRU. The Shelf Manager may consider such a situation to be a
serious fault and assume that the Shelf FRU Information is corrupted or not up-to-date.


*/


/* Table 3-59 FRU Activation and Power Descriptor */
typedef struct fru_activation_power_record {
	uchar	hw_addr;	/* This is the Hardware Address of the device. */
	uchar	fru_dev_id;	
		/* FRU Device ID. Since the order of this table determines the 
		   power-on sequencing, the order cannot be used to determine 
		   which FRU Device ID is being referenced. A value of FEh shall
		   indicate that all FRU Device IDs at the Hardware Address be
		   considered as a unit. This would be true for Boards that have
		   mezzanine cards attached. */
	uchar	max_fru_power_capability_msb;
	uchar	max_fru_power_capability_lsb;
		/* Maximum FRU Power Capability. This field contains the maximum
		   wattage that can be routed to the Hardware Address/FRU Device ID
		   location. This is stored as a number of Watts in 1 W increments. */
		/* The following byte contains activation and power configuration
		   parameters for the Hardware Address/FRU Device ID location. */
	uchar	reserved:1,	/* [7] Reserved. Shall be set to 0. */
	     	shm_controlled_activation:1,
		/* [6] Shelf Manager Controlled Activation. This flag determines
		   if the Shelf Manager activates the FRU residing at this location,
		   when it reaches M2.
		   - 0h = Disabled. The Shelf Manager does not activate this FRU.
		     The FRU waits in M2 until the System Manager decides to 
		     activate it.
		   - 1h = Enabled. The Shelf Manager activates this FRU, and it 
		     proceeds to M3. */
		delay_before_next_power_on:6;
		/* [5:0] Delay Before Next Power On. Tenths of a second to delay
		   before powering up any other FRU after powering this FRU. 
		   Note: for entries with FRU Device ID set to FEh, this delay
		   is performed after powering on each FRU represented by a 
		   particular IPM Controller, not just once after powering on
		   the last FRU. */
} FRU_ACTIVATION_POWER_RECORD;

/* Table 3-58 Shelf Activation and Power Management Record */

typedef struct shelf_activation_power_record {
	uchar rec_type_id;	/* Record Type ID. For all records defined in this
				   specification, a value of C0h (OEM) shall be used. */
	uchar 	eol:1,		/* [7:7] End of list. Set to one for the last record */
	      	reserved:3,	/* [6:4] Reserved, write as 0h.*/
		version:4;	/* [3:0] record format version (=2h for this definition) */
	uchar	record_len;	/* Record Length. */
	uchar	record_cksum;	/* Record Checksum. Holds the zero checksum of the record. */
	uchar	header_cksum;	/* Header Checksum. Holds the zero checksum of the header. */
	uchar	manuf_id[3];	/* Manufacturer ID. LS Byte first. Write as the
				   three byte ID assigned to PICMG®. For this
				   specification, the value 12634 (00315Ah) shall
				   be used. */
	uchar	picmg_rec_id;	/* PICMG Record ID. For the Shelf Power 
				   Distribution Record, the value 12h shall be
				   used. */
	uchar	rec_fmt_ver;	/* Record Format Version. For this specification,
				   the value 0h shall be used. */
	uchar	t_activation_readiness;
				/* Allowance for FRU Activation Readiness. This field
				   contains the number of seconds after system startup
				   that FRUs have to transition to state M3 and maintain
				   their power up sequence position. */
	uchar	number_fru_power_records;
				/* FRU Activation and Power Descriptor Count. This
				   contains a count of the number of entries (M) in
				   the FRU Activation and Power Descriptor Table. */
	FRU_ACTIVATION_POWER_RECORD fru_power_rec[16];
				/* FRU Activation and Power Descriptors. This 
				   contains an array of size [M*] activation and
				   power descriptors for each FRU location. Each
				   descriptor follows the format found in Table 3-59,
				   “FRU Activation and Power Descriptor,” and is 5
				   bytes in size. */
} SHELF_ACTIVATION_POWER_RECORD;


/* Table 3-55 Shelf Power Distribution Record */
typedef struct shelf_power_record {
	uchar record_type_id;	/* Record Type ID. For all records defined in
				   this specification, a value of C0h (OEM)
				   shall be used. */
	uchar 	eol:1,		/* [7:7] End of list. Set to one for the last record */
	      	reserved:3,	/* [6:4] Reserved, write as 0h.*/
		version:4;	/* [3:0] record format version (=2h for this definition) */
	uchar	record_len;	/* Record Length. */
	uchar	record_cksum;	/* Record Checksum. Holds the zero checksum of the record. */
	uchar	header_cksum;	/* Header Checksum. Holds the zero checksum of the header. */
	uchar	manuf_id[3];	/* Manufacturer ID. LS Byte first. Write as the
				   three byte ID assigned to PICMG®. For this
				   specification, the value 12634 (00315Ah) shall
				   be used. */
	uchar	picmg_rec_id;	/* PICMG Record ID. For the Shelf Power 
				   Distribution Record, the value 11h shall be
				   used. */
	uchar	rec_fmt_ver;	/* Record Format Version. For this specification,
				   the value 0h shall be used. */
	uchar	num_pw_feeds;	/* Number of Power Feeds. This field specifies
				   the number of power Feeds (N) defined in this
				   Shelf Power Distribution Record. */
	POWER_DISTRIBUTION_MAP	pw_map;
				/* Power Distribution Map. This table contains
				   N variable sized Power Distribution Maps, one
				   for each Feed (see Table 3-56, “Power 
				   Distribution Map”). */
	
} SHELF_POWER_RECORD;


/*
Each of the Power Distribution Maps describes the properties of a single Feed into the Shelf.
Shelf. The format of this data is shown below in Table 3-56, “Power Distribution Map.”
 
Table 3-56 Power Distribution Map
*/
typedef struct power_distribution_map {
	uchar max_ext_current_lsb;
	uchar max_ext_current_msb;	
		/* Maximum External Available Current.
		   LS Byte first. This holds the maximum current, in 1/10th Amps,
		   that is available into the Feed to the Shelf. */
	uchar max_int_current_lsb;
	uchar max_int_current_msb;
		/* Maximum Internal Current. 
		   LS Byte first. This holds the maximum current, in 1/10th Amps,
		   that the internal Shelf circuitry can handle for this Feed.
		   The Shelf manufacturer sets this value. */
	uchar min_exp_op_voltage;	
		/* Minimum Expected Operating Voltage. Holds the minimum Voltage,
		   in 1/2 V increments, used to determine worse case current draw
		   of the Shelf. Service personnel may enter a value for this field.
		   A value of FFh shall indicate that no value has been set. Only
		   values from 48h (equivalent to 72 decimal indicating –36 V) to
		   and including 90h (equivalent to 144 decimal indicating –72 V)
		   shall be considered valid for purposes of computation. A default
		   value of 48h (indicating – 36 V) shall be used when a valid entry
		   is not found in this field. */
	uchar num_map_entries;
		/* Feed-to-FRU Mapping Entries Count. This contains a count of the
		   number of entries (M) in the Feed-to-FRU Mapping Entries Table. */
	FEED_TO_FRU_MAPPING map_entries;
		/* Feed-to-FRU Mapping Entries. This array of size [M*2] describes
		   all the FRUs that are powered from this Feed. Each entry follows
		   the format found in Table 3-57, “Feed-to-FRU Mapping entry,” and
		   is 2 bytes in size. */
} POWER_DISTRIBUTION_MAP;

/* Table 3-57 Feed-to-FRU Mapping entry */
typedef struct feed_to_fru_mapping {
	uchar hw_addr;
		/* Hardware Address. This is the Hardware Address of the Intelligent
		   FRU that represents this FRU. Since a single Hardware Address may
		   have multiple associated FRUs, the Feed-to-FRU mapping needs to
		   have both a Hardware Address and FRU Device ID. */
	uchar fru_dev_id;
		/* FRU Device ID. A value of FEh shall indicate that all FRU Device IDs
		   at the Hardware Address be considered as a unit. For instance, this
		   would be true for Boards that have mezzanines attached. */
} FEED_TO_FRU_MAPPING;



/*
During Normal Operation the Shelf Manager awaits event messages from Boards and/or FRUs to
make any adjustments to Shelf cooling or power distribution from the current operation conditions.
As FRUs are inserted and extracted, the Shelf Manager is alerted and alters power budgets, EKeying,
and possibly cooling based on these conditions. In addition, FRUs may have conditions
change that the Shelf Manager is not directly aware of, so those FRUs can renegotiate power (up or
down) at any point during normal operation. The Shelf Manager may choose to deny the request.

The Abnormal Operation Stage occurs when a Board or FRU generates an Event Message
requesting Shelf services from the Shelf Manager. Typically, the Shelf Manager will adjust system
cooling (e.g., fan rotational velocity) or adjust power to one or more FRUs in an attempt to return
the Shelf to the Normal Operation Stage.

— Cooling Control. Once the Shelf Manager realizes that cooling needs have changed (due
to an event generated by a FRU), it must be able to control the Fan Tray speed to provide
more (or less) cooling (Section 3.9.3.3, “Cooling management,” for details). The “Get Fan
Speed Properties” command (see Table 3-63, “Get Fan Speed Properties command”)
describes how the Shelf Manager queries the Fan Tray device to understand how to
control its behavior. This command returns the Minimum and Maximum Speed Level that
the Fan Tray supports. In addition, the Normal Operating Level describes the default
normal fan speed recommended by the fan manufacturer. Depending on the fan
manufacturer, the Normal Operating Level may be less than the Maximum Speed Level
order to reserve some headroom in case the operating conditions are not optimal.
— Power Control. The Shelf Manager might decide that conditions warrant decreasing
power settings to a FRU or possibly even powering the FRU off entirely. Thus, a FRU can
receive a “Set Power Level” at any time during operation.
*/





/*
At initial startup the Shelf Manager allows Allowance for FRU Activation Readiness seconds for
FRUs that need activation to come to the M3 state. During this time the Shelf Manager processes
hot swap events from those FRUs. When a FRU indicates that it has reached the M2 state, the Shelf
Manager checks its FRU Activation and Power Descriptor in the “Shelf Activation and Power
Management Record” in the Shelf FRU Information. Each FRU that has the Shelf Manager
Controlled Activation field set to Enabled is transitioned to M3 state.
At initial startup the Shelf Manager steps through the FRU Activation and Power Descriptors in
the Shelf FRU Information. The order of the descriptors is important for determining the power on
sequence. The Shelf Manager checks the existence and operational state of each FRU in the order
they are described in the Shelf FRU Information. As the Shelf Manager steps through the FRU
Activation and Power Descriptors table, each FRU that has reached state M3 is powered on via the
multi-step process summarized in Figure 3-14, “Power discovery Board/FRU participation.” After
powering up a FRU, the Shelf Manager ensures that another FRU is not assigned a non-zero power
level within Delay Before Next Power On tenths of a second. If a FRU for which activation is
controlled by the Shelf Manager has not reached state M3 by the time the Shelf Manager starts
processing its descriptor in the FRU Activation and Power Descriptors table, the Shelf Manager
waits for the earlier of: 1) the FRU transitioning to M3 and 2) Allowance for FRU Activation
Readiness seconds after initial system startup. After the allowance period, if a FRU is not in M3
when its FRU Activation and Power Descriptor is processed, the Shelf Manager proceeds to the
next descriptor without waiting for that FRU.
This sequence provides a mechanism for the op

This sequence provides a mechanism for the operator or Shelf vendor to specify how the FRU
should act when power is applied to the Shelf. At a minimum, this mechanism enables specifying
which FRUs must be powered up for the Shelf Manager to have connectivity to the System
Manager. At the extreme, this mechanism allows a fully autonomous Shelf.

The Shelf FRU Information may include several Shelf Activation and Power Management Records
if a single record is not sufficient for describing all FRU locations. If several Shelf Activation and
Power Management Records are present, during the initial Shelf power up sequence, these records
are processed in the order they reside in the Shelf FRU Information.

3.9.1.3 Fan Tray participation
The “Get Fan Speed Properties” command allows the Shelf Manager to query the
Fan Tray to understand how to control its behavior. This command returns the
Minimum and Maximum Speed Level that the Fan Tray supports. In addition, the
Normal Operating Level describes the default normal fan speed recommended by
the fan manufacturer. Depending on the fan manufacturer, the Normal Operating
Level may be less than the Maximum Speed Level in order to reserve some 
headroom in case the operating conditions are not optimal.

When a Fan Tray supports Local Control mode, it has the resources to make 
intelligent decisions about the speed of the fans. For instance, this might
include a set of temperature sensors that enable it to speed up or slow down
the fans, independent of the Shelf Manager. When Local Control mode is supported,
any speeds that the Shelf Manager sets are considered overrides. The Fan Tray
controller selects the maximum of the override or local control speeds.

In certain emergency situations (such as in a fire condition), either the Shelf
Manager or an intelligent Fan Tray may conclude that an emergency shut down of
the fans is necessary.

• Fan Trays shall reserve enough power envelope such that changing the Fan Level to the
Maximum Speed Level requires no change to the Fan Tray power budget.
• All Fan Trays that support more than one speed shall accept the “Get Fan Speed Properties”,
“Get Fan Level”, and “Set Fan Level” commands.
• In the “Get Fan Speed Properties” command, the Fan Tray IPM Controller shall set the
Minimum Speed Level to be the minimum value that is accepted in the “Set Fan Level”
command.

*/

SHELF_ACTIVATION_POWER_RECORD sh_activation_power_record;

void
fru_m2( void )
/*When the FRU reaches the M2 state, the Shelf Manager builds partial SDR Repository entries for
the FRU and begins periodic verification of the presence of the FRU */
{

}

void
fru_m3( void )
/* Once the FRU reaches the M3 state, it sends the M2 to M3 event to the Shelf Manager and waits
for the Shelf Manager to begin power and/or cooling negotiation 

The first command of the power negotiation sequence, Compute Power Properties, is the
“Compute Power Properties command.” This command is sent by the Shelf Manager to
the IPM Controller to inform the device that it should “lock” in its desired power and cooling
levels. The IPM Controller returns part of this data in the response to this command. In particular,
the data returned is the data that applies to the IPM Controller or FRU itself. This includes
information such as the number of Slots that the FRU spans, if the FRU is a Board and where the
IPM Controller is located.

The Shelf Manager determines the
proper power allocation and sends a “Set Power Level” command to inform the FRU of the power
budget it has been allocated. The M3 state is also the place where the Shelf Manager computes the
E-Keying requirement; however, the FRU can transition to M4 prior to having received its EKeying.
Though E-Keys are computed in M3, the E-keys may be read earlier in the FRU’s life
cycle (i.e., in either M2 or M3). When the Shelf Manager figures out which E-Keying to enable for
the FRU, it sends “Set Port State” command(s) to inform the FRU of the enabled and disabled Ports
Another step the IPM Controller takes when it receives the Compute Power Properties is that it
prepares for the receipt of the “Get Power Level” command.This is especially important when the
IPM Controller is undergoing a power renegotiation. Conceptually, each FRU has an array of up to
20 entries for storing different levels of power draw. This has been further divided into four
categories. The first category is the “Steady state power draw levels”. When an IPM Controller has
powered up and reached a steady state, this array represents the maximum power envelope the
FRU needs to operate at each of the 20 levels. The second is “Desired steady state draw levels”.
This supports a case when a FRU is operating with its “Steady state power draw levels”, but wants
to negotiate for a new power level, it must have a mechanism to communicate the new values to the
Shelf Manager without losing the old values in case the Shelf Manager is unable to allow the new
levels. The third is “Early power draw levels”. This array of values returns the amount of power
draw that the FRU uses up until it is able to reach its steady state power level. For example, if a
Payload is capable of operating at various low power states, but must come up at full power prior to
entering a low power state, then the “Early power draw levels” would be used to indicate that for a
relatively short duration the FRU requires more power than its steady state power draw. The fourth
category covers the “Desired early levels” which describes the desired levels for which the FRU is
negotiating.
To make the negotiation and renegotiation schemes consistent, whenever the FRU receives a
“Compute Power Properties” command, it caches the “desired” levels. Once the IPM Controller
responds to the “Compute Power Properties” command, the Shelf Manager sends the “Get Power
Level” command twice, once for each of the “desired” power levels. The Shelf Manager uses the
information in two responses to see what Power Level the FRU would like to run at and what the
various levels of power draw are.
If the Shelf Manager has the power budget to allow the FRU to change power levels, up or down, it
sends a “Set Power Level” command. Notice that power values returned in bytes 6 and beyond of
the “Get Power Level” command do not need to be linear, but do need to be monotonically
increasing. For example, power level 1 could be 38 W, power level 2 could be 150 W, and power
level 3 could be 200 W. The Shelf Manager simply uses the returned power values to compute the
appropriate Power Level (1 through 3 in the above example) and then sends the “Set Power Level”
command to set the correct level. Power Level 0 is reserved as a power off command for the FRU.
A Shelf Manager that simply wants to control power as off or maximum uses a Power Level of 0 or
Max where Max is the index of the maximum Power Draw provided in the response.
Using the “Set Power Level” command, the Shelf Manager can instruct the FRU about its new
(possibly unchanged) Power Level. In addition, the command allows the Shelf Manager to accept
the desired power levels by “Set Present Levels to Desired Levels”. When the IPM Controller
receives this directive, it adopts the desired levels as its new operating levels. From that point
forward, the FRU must not exceed the power envelope described by the Power Draw[Power
Level] * Power Multiplier.

*/
{

}


void
fru_m4( void )
/*
Once a FRU has reached the M4 state, the Shelf Manager’s job becomes monitoring the FRU for
health related events and, for each Front Board, managing changes to the E-Keying based on
insertion or extraction of other Front Boards that share an interface with that Front Board.
*/
{

}

void
fru_extraction( void )
/* The FRU transitions to M5 and then sends an event to the Shelf Manager that the FRU
wishes to deactivate. 
the Shelf/System Manager determines if it is valid for the FRU
to deactivate and sends a “Set FRU Activation
(Deactivate FRU)” command to allow the FRU to begin deactivation.

When the Shelf Manager receives the M6 to M1
transition event, it reclaims the FRU’s power budget, removes the FRU from the SDR Repository,
and if the FRU is a Front Board, it disables all Ports on other Front Boards that share an interface
with this Front Board.

*/
{

}
#define FRU_DATA_TYPE_BINARY		0x0	// binary or unspecified.
#define FRU_DATA_TYPE_BCD		0x1	// BCD plus
#define FRU_DATA_TYPE_SIX_BIT_ASCII	0x2	// 6-bit ASCII, packed (overrides Language Codes).
#define FRU_DATA_TYPE_CHAR_CODE		0x4	
		/* Interpretation depends on Language Codes. 11b indicates 8-bit
		   ASCII + Latin 1 if the Language Code is English for the area
		   or record containing the field, or 2-byte UNICODE (least
		   significant byte first) if the Language Code is not English.
		   At least two bytes of data must be present when this type is
		   used. Therefore, the length (number of data bytes) will always
		   be >1 if data is present, 0 if data is not present. */

/* 
FRU Records and additional IPMI sensors defined by PICMG® 2.9 and PICMG® 3.0.
Shelf FRU Information is a collection of data records describing Shelf power, cooling, and interconnect
capabilities at a minimum.

All the Shelf FRU Information shall be available by querying the Shelf Manager (at address
20h) and accessing the Chassis Info Area and MultiRecord Area of FRU Device ID 254,
LUN 0.

Table 3-71 PICMG® FRU Records: type ID = C0h (OEM)

Record			PICMG® 2.9 ECN 			PICMG®		Record
description						record ID	present in
----------------------------------------------------------------------------------
Backplane		Table 3-32, “Backplane		0x04		Shelf FRU
Point-to-Point		Point-to-Point Connectivity
Connectivity		Record”

							PICMG® 3.0	Record ID
Address Table		Table 3-6, “Address Table” 	0x10		Shelf FRU

Shelf Power 		Table 3-55, “Shelf Power	0x11		Shelf FRU
Distribution		Distribution Record”

Shelf Activation 	Table 3-58, “Shelf Activation	0X12		Shelf FRU
and Power Management 	and Power Management Record”

Shelf Manager		Table 3-28, “Shelf Manager	0x13		Shelf FRU
IP Connection		IP Connection Record”

Board Point-to-Point	Table 3-35, “Board Point-to-	0x14		Shelf FRU
Connectivity		Point Connectivity Record”

Radial IPMB-0		Table 3-46, “Radial IPMB-0	0x15		Shelf FRU
Link Mapping		Link Mapping Record"
*/

/* Table 3-72 PICMG® sensors: Event/Reading Type Code = Sensor specific (6Fh) */
#define ST_FRU_HOT_SWAP		0xf0	// FRU Hot Swap Table 3-19, “FRU Hot Swap Event Message”
#define ST_IPMB_PHYS_LINK	0xf1	// IPMB Physical Link Table 3-50, “Physical IPMB-0 Status Change Event Message”

/* Table 3-73 PICMG Entity IDs */
#define PICMG_ENTITY_FRONT_BOARD	0xa0	// PICMG Front Board
#define PICMG_REAR_TRANSITION_MODULE	0xc0	// PICMG Rear Transition Module
#define PICMG_SHELF_MGMT_CONTROLLER	0xf0	// PICMG Shelf Management Controller
#define PICMG_FILTRATION_UNIT		0xf1	// PICMG Filtration Unit
#define PICMG_SHELF_FRU_INFORMATION	0xf2	// PICMG Shelf FRU Information


/* Table 3-32 Backplane Point-to-Point Connectivity Record */

typedef struct backplane_p2p_conn_record {
	uchar	record_type_id;	/* Record Type ID. For all records defined
				   in this specification a value of C0h (OEM)
				   shall be used. */
	uchar 	eol:1,		/* [7:7] End of list. Set to one for the last record */
	      	reserved:3,	/* [6:4] Reserved, write as 0h.*/
		version:4;	/* [3:0] record format version (=2h for this definition) */
	uchar	record_len;	/* Record Length. */
	uchar	record_cksum;	/* Record Checksum. Holds the zero checksum of the record. */
	uchar	header_cksum;	/* Header Checksum. Holds the zero checksum of the header. */
	uchar	manuf_id[3];	/* Manufacturer ID. LS Byte first. Write as the
				   three byte ID assigned to PICMG®. For this
				   specification, the value 12634 (00315Ah) shall
				   be used. */
	uchar	picmg_rec_id;	/* PICMG Record ID. For the Shelf Power 
				   Distribution Record, the value 11h shall be
				   used. */
	uchar	rec_fmt_ver;	/* Record Format Version. For this specification,
				   the value 0h shall be used. */
	P2P_SLOT_DESCRIPTOR	p2p_slot_descr[16];	
				/* Point-to-Point Slot Descriptor List. A list of
				   variable length Point-to-Point Slot Descriptors
				   (see Table 3-33, “Point-to-Point Slot Descriptor”).
				   Each Point-to-Point Slot Descriptor describes the
				   number of Channels and the connectivity for a
				   specific type of point-to-point Channel from 
				   one Slot. */
} BACKPLANE_P2P_CONN_RECORD;

#define IF_SINGLE_PORT_FABRIC	0x08	// PICMG® 3.0 Single Port Fabric Interface
#define IF_DOUBLE_PORT_FABRIC	0x09	// PICMG®3.0 Double Port Fabric Interface
#define IF_FULL_CHANNEL_FABRIC	0x0A	// PICMG® 3.0 Full Channel Fabric Interface
#define IF_BASE			0x0B	// PICMG® 3.0 Base Interface
#define IF_UPDATE_CHANNEL	0x0C	// PICMG® 3.0 Update Channel Interface

/* Table 3-33 Point-to-Point Slot Descriptor */
typedef struct p2p_slot_descriptor {
	uchar	p2p_channel;	/* Point-to-Point Channel Type : IF_xx */

	uchar	slot_address;	/* Slot address for this Slot. For PICMG® 3.0
				   systems, this is the Hardware Address. */
	uchar	p2p_channel_count;
				/* number of point-to-point Channels in this
				   Slot of the type specified in Point-to-Point
				   Channel Type. */
	P2P_CH_DESCR	p2p_channel_descr[n];  // TODO: determine n
} P2P_SLOT_DESCRIPTOR;

/* Point-to-Point Channel Descriptors. An array of n Point-to-Point
Channel Descriptors (each with LS Byte first) where n is specified in
the Point-to-Point Channel Count byte. */
typedef struct p2p_ch_descr {
	lsb;
	midb;
	msb;
} P2P_CH_DESCR;

/* Table 3-35 Board Point-to-Point Connectivity Record */
typedef struct board_p2p_conn_record {
	uchar	record_type_id;	/* Record Type ID. For all records defined
				   in this specification a value of C0h (OEM)
				   shall be used. */
	uchar 	eol:1,		/* [7:7] End of list. Set to one for the last record */
	      	reserved:3,	/* [6:4] Reserved, write as 0h.*/
		version:4;	/* [3:0] record format version (=2h for this definition) */
	uchar	record_len;	/* Record Length. */
	uchar	record_cksum;	/* Record Checksum. Holds the zero checksum of the record. */
	uchar	header_cksum;	/* Header Checksum. Holds the zero checksum of the header. */
	uchar	manuf_id[3];	/* Manufacturer ID. LS Byte first. Write as the
				   three byte ID assigned to PICMG®. For this
				   specification, the value 12634 (00315Ah) shall
				   be used. */
	uchar	picmg_rec_id;	/* PICMG Record ID. For the Shelf Power 
				   Distribution Record, the value 11h shall be
				   used. */
	uchar	rec_fmt_ver;	/* Record Format Version. For this specification,
				   the value 0h shall be used. */
	uchar	oem_guid_count;	/* The number, n, of OEM GUIDs defined in this record. */
	OEM_GUID oem_guid_list[n];
				/* A list 16*n bytes of OEM GUIDs. */
	link_desrc_list;	/* Link Descriptor list. A variable length list 
				   of four byte Link Descriptors (LS Byte first)
				   (see Table 3-36, “Link Descriptor”; Table 3-37,
				   “Link Designator”; and Table 3-38, “Link Type”)
				   totaling m bytes in length.
			  	   Each Link Descriptor details one type of 
				   point-to-point protocol supported by the
				   referenced Channels. */
} BOARD_P2P_CONN_RECORD;


/* Table 3-36 Link Descriptor */
typedef struct link_descriptor {
	unsigned long	link_grouping_id:8,	
		/* [31:24] Link Grouping ID. Indicates whether the Ports of this
		   Channel are operated together with Ports in other Channels. 
		   A value of 0 always indicates a Single-Channel Link. A common,
		   non-zero Link Grouping ID in multiple Link Descriptors indicates
		   that the Ports covered by those Link Descriptors must be operated
		   together. A unique non-zero Link Grouping ID also indicates
		   Single-Channel Link. */
			link_type_extension:4,
		/* [23:20] Link Type Extension. Identifies the subset of a 
		   subsidiary specification that is implemented and is defined
		   entirely by the subsidiary specification identified in the 
		   Link Type field. */
			link_type:8,
		/* [19:12] Link Type. Identifies the PICMG® 3.x subsidiary
		   specification that governs this description or identifies the
		   description as proprietary; see Table 3-38, “Link Type.” */
		   	link_designator:12;
		/* [11:0] Link Designator. Identifies the Interface Channel and
		   the Ports within the Channel that are being described; see
		   Table 3-37, “Link Designator.” */
} LINK_DESCRIPTOR;


/* Table 3-37 Link Designator */
typedef struct link_designator {
	uint16_t	:4,
			port3_bit_flag:1, /* [11] Port 3 Bit Flag (1 = Port Included; 0 = Port Excluded) */
			port2_bit_flag:1, /* [10] Port 2 Bit Flag (1 = Port Included; 0 = Port Excluded) */
			port1_bit_flag:1, /* [9] Port 1 Bit Flag (1 = Port Included; 0 = Port Excluded) */
			port0_bit_flag:1, /* [8] Port 0 Bit Flag (1 = Port Included; 0 = Port Excluded) */
			interface:2;	/* [7:6] Interface.
					   00b = Base Interface
					   01b = Fabric Interface
					   10b = Update Channel Interface
					   11b = Reserved */
			channel_number:6; /* [5:0] Channel Number.
					     Base Interface: 01h = Channel 1 … 10h = Channel 16
					     Fabric Interface: 01h = Channel 1… 0Fh = Channel 15
					     Update Channel Interface: 01h = Channel 1 */
} LINK_DESIGNATOR;


/* Table 3-38 Link Type */
#define LINK_TYPE_BASE		0x01	// PICMG 3.0 Base Interface 10/100/1000 BASE-T
#define LINK_TYPE_ETHERNET	0x02	// PICMG 3.1 Ethernet Fabric Interface
#define LINK_TYPE_INFINIBAND	0x03	// PICMG 3.2 Infiniband Fabric Interface
#define LINK_TYPE_STARFABRIC	0x04	// PICMG 3.3 StarFabric Fabric Interface
#define LINK_TYPE_PCI_EXPRESS	0x05	// PICMG 3.4 PCI Express Fabric Interface
/*
00h Reserved
06h... EFh Reserved
F0h... FEh E-Keying OEM GUID Definition
FFh Reserved
*/

/* Table 3-46 Radial IPMB-0 Link Mapping Record */
typedef struct link_mapping_record {
	uchar	record_type_id;	/* Record Type ID. For all records defined
				   in this specification a value of C0h (OEM)
				   shall be used. */
	uchar 	eol:1,		/* [7:7] End of list. Set to one for the last record */
	      	reserved:3,	/* [6:4] Reserved, write as 0h.*/
		version:4;	/* [3:0] record format version (=2h for this definition) */
	uchar	record_len;	/* Record Length. */
	uchar	record_cksum;	/* Record Checksum. Holds the zero checksum of the record. */
	uchar	header_cksum;	/* Header Checksum. Holds the zero checksum of the header. */
	uchar	manuf_id[3];	/* Manufacturer ID. LS Byte first. Write as the
				   three byte ID assigned to PICMG®. For this
				   specification, the value 12634 (00315Ah) shall
				   be used. */
	uchar	picmg_rec_id;	/* PICMG Record ID. For the Shelf Power 
				   Distribution Record, the value 11h shall be
				   used. */
	uchar	rec_fmt_ver;	/* Record Format Version. For this specification,
				   the value 0h shall be used. */
	uchar	ipmb0_conn_lsb;	/* IPMB-0 Connector Definer. Three-byte ID (LS 
				   byte first) assigned by IANA to the organization
				   that has defined the mapping of IPMB-0 Links to
				   the connector through which an IPMB-0 Hub connects
				   to the Backplane. */
	uchar	ipmb0_conn_mid;
	uchar	ipmb0_conn_mid;
	uchar	ipmb0_conn_ver_id_lsb;
				/* IPMB-0 Connector Version ID. This two-byte field
				  (LS byte first) identifies the connector mapping
				  version among those of the IPMB-0 Connector Definer. */
	uchar	ipmb0_conn_ver_id_msb;
		
	uchar	addr_entry_count; /* Indicates the number of IPMB-0 Link Mapping Entries. */
	LINK_MAPPING_ENTRY link_mapping_entry[N];
				/* 16 N IPMB-0 Link Mapping Entries. These are the
				   mapping entries. N = (Address Entry Count * 2). */
} LINK_MAPPING_RECORD;

/* Table 3-47 IPMB-0 Link Mapping Entries */
typedef struct link_mapping_entry {
	uchar	hw_address;	/* Hardware Address. The Hardware Address of a
				   FRU from the Address Table. */
	uchar	ipmb0_link_entry; /* IPMB-0 Link Entry. The IPMB-0 Link Number 
				     (1 to 95) by which the Hardware Address is
				     reached. */
} LINK_MAPPING_ENTRY;


/* Table 3-6, “Address Table” */
typedef struct addr_table {
	uchar	record_type_id;	/* Record Type ID. For all records defined
				   in this specification a value of C0h (OEM)
				   shall be used. */
	uchar 	eol:1,		/* [7:7] End of list. Set to one for the last record */
	      	reserved:3,	/* [6:4] Reserved, write as 0h.*/
		version:4;	/* [3:0] record format version (=2h for this definition) */
	uchar	record_len;	/* Record Length. */
	uchar	record_cksum;	/* Record Checksum. Holds the zero checksum of the record. */
	uchar	header_cksum;	/* Header Checksum. Holds the zero checksum of the header. */
	uchar	manuf_id[3];	/* Manufacturer ID. LS Byte first. Write as the
				   three byte ID assigned to PICMG®. For this
				   specification, the value 12634 (00315Ah) shall
				   be used. */
	uchar	picmg_rec_id;	/* PICMG Record ID. For the Shelf Power 
				   Distribution Record, the value 11h shall be
				   used. */
	uchar	rec_fmt_ver;	/* Record Format Version. For this specification,
				   the value 0h shall be used. */
	uchar	type:2,		/* Type/Length Byte (FRU Information variant)
				   [7:6] - Type - FRU_DATA_TYPE_xxx */
		length:6;	/* [5:0] - Length */
	uchar	shelf_address[20];
			/* Shelf Address bytes as a fixed size field, with Length
			   bytes of the Shelf address, encoded according to the
			   Type field in byte 10 and starting at byte 11. The 
			   values of the unused bytes of the fixed size field—
			   bytes (11 + Length)—30 are undefined.
			   Shelf Address is a  variable length, variable format
			   descriptor of up to 20 bytes in length providing a
			   unique identifier for each Shelf within a management 
			   domain. */
	uchar	addr_entry_count;	/* 31 - Address Table Entries Count. 
					   Indicates the number of entries in 
					   the Address Table Entries. */
	ADDR_TABLE_ENTRY addr_entry[16];	
		/* 32-N - Address Table Entries. This is essentially an array of
		   Address Table Entries. N = (Address Table Entries Count * 3) */
} ADDR_TABLE;

typedef struct addr_table_entry {
	uchar hw_address;
	uchar site_number;
	uchar site_type;
} ADDR_TABLE_ENTRY;

ADDR_TABLE address_table = {
	0xc0,				// record type id
	1,				// end of list
	0,				// reserved
	2,				// record format version
	0,				// record len. This and the following 2 entries are calculated at init time
	0,				// record_cksum,
	0,				// header_cksum
	{ 0x51, 0x31, 0x00 },		// manuf id, lsb first
	0x11,				// PICMG record ID
	0,				// record format version
	FRU_DATA_TYPE_SIX_BIT_ASCII,	// type
	20,				// length of shelf address
	// shelf address
	{ '0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0' },
	16,				// entry count
	{ 0x41, 1, SITE_TYPE_ATCA },	// hw_address, site_number, site_type
	{ 0x42, 2, SITE_TYPE_ATCA },
	{ 0x43, 3, SITE_TYPE_ATCA },
	{ 0x44, 4, SITE_TYPE_ATCA },
	{ 0x45, 5, SITE_TYPE_ATCA },
	{ 0x46, 6, SITE_TYPE_ATCA },
	{ 0x47, 7, SITE_TYPE_ATCA },
	{ 0x48, 8, SITE_TYPE_ATCA },
	{ 0x49, 9, SITE_TYPE_ATCA },
	{ 0x4a, 10, SITE_TYPE_ATCA },
	{ 0x4b, 11, SITE_TYPE_ATCA },
	{ 0x4c, 12, SITE_TYPE_ATCA },
	{ 0x4d, 13, SITE_TYPE_ATCA },
	{ 0x4e, 14, SITE_TYPE_ATCA },
	{ 0x4f, 15, SITE_TYPE_ATCA },
	{ 0x50, 16, SITE_TYPE_ATCA }
}; 


/*
 *
Periodic verification
----------------------
The Shelf Manager periodically “pings” each Intelligent FRU to verify its presence. 
If the FRU does not respond, the Shelf Manager makes a note of that fact by generating
a FRU Hot Swap Event for the system event log (SEL) with a Current State of M7 and 
a Previous State of the last known state. Communications are considered
regained if the Shelf Manager eventually receives a response from its “pinging,” or it receives a
new State Change Event (or other message) from the lost FRU. If communication is regained, the
Shelf Manager can determine the current state of the FRU by issuing the “Set Event Receiver”
command. Once the Shelf Manager determines the current state it then asserts a FRU Hot Swap
Event Message with a Current State set to the FRU’s current state, and Previous State of M7.
The spec does not indicate what command to use for pings. We'll use "Get Device ID" 
for the time being.
*/

/* The Shelf Manager checks the existence and operational state of each FRU in the order
they are described in the Shelf FRU Information.*/

void
shm_process_work_list( void )
{
	FRU_ENTRY *fru_entry;

	switch( fru_entry->current_state ) {
		case FRU_STATE_M0_NOT_INSTALLED:
		case FRU_STATE_M1_INACTIVE:
			break;
		case FRU_STATE_M2_ACTIVATION_REQUEST:
			if( startup )
				
			break;
		case FRU_STATE_M3_ACTIVATION_IN_PROGRESS:
		case FRU_STATE_M4_ACTIVE:
		case FRU_STATE_M5_DEACTIVATION_REQUEST:
		case FRU_STATE_M6_DEACTIVATION_IN_PROGRESS:
		case FRU_STATE_M7_COMMUNICATION_LOST:
			break;
		default:
			break;
	}
}


void
activate_frus( void ) 
{
	uchar num_frus = sh_activation_power_record.number_fru_power_records;
	uchar hw_addr;
	FRU_ACTIVATION_POWER_RECORD fru_power_rec;
	FRU_ENTRY *fru_entry;

	/* step through the FRU Activation and Power Descriptors table */
	for( i = 0; i < num_frus; i++ ) {
		// get the hw id
		hw_addr = sh_activation_power_record.fru_power_rec[i].hw_addr;
		fru_entry = get_fru_entry_from_hw_addr( hw_addr );

		switch( fru_entry->fru_state ) {
			case FRU_STATE_M0_NOT_INSTALLED:
			case FRU_STATE_M1_INACTIVE:
				break;
			case FRU_STATE_M2_ACTIVATION_REQUEST:
				fru_power_rec = get_fru_activation_power_rec( hw_addr );
				if( fru_power_rec && ( fru_power_rec->sh_mgr_controlled_activation ) && !fru_entry->do_not_activate ) { 
					if( !fru_entry->activation_sent ) {
						// TODO send an activation command
						fru_entry->activation_sent = 1;
						fru_entry->activation_lbolt = lbolt;
						return;
					} else {
						// activation sent, check if it timed out
						if( lbolt > fru_entry->activation_lbolt/HZ + t_activation_readiness )
							fru_entry->do_not_activate = 1;
					}
			    	}
			    break;
			case FRU_STATE_M3_ACTIVATION_IN_PROGRESS:
			    if( !fru_entry->requested_fru_entry_device_locator_record ) {
				fru_entry->requested_fru_entry_device_locator_record = 1;
				// ask for SDR Type 11h - FRU Device Locator Record
				break;
			    }
			    
			    if( !fru_entry->requested_shm_device_locator_record ) {
				// ask for SDR Type 12h - Management Controller Device Locator Record
				fru_entry->requested_shm_device_locator_record = 1;
				break;
			    }
			    if( !fru_entry->sent_compute_power_properties ) {
				// Send Compute Power Properties command
				fru_entry->sent_compute_power_properties = 1;
				break;
			    }
			    /* shm determines the proper power allocation and
			       sends a “Set Power Level” command */
			    break;
			case FRU_STATE_M4_ACTIVE:
			case FRU_STATE_M5_DEACTIVATION_REQUEST:
			case FRU_STATE_M6_DEACTIVATION_IN_PROGRESS:
			case FRU_STATE_M7_COMMUNICATION_LOST:
				break;
			default:
				break;
		}
	
	// TODO: check sh_mgr_controlled_activation bit
	/* Go through the list of FRUs in the shelf_fru_info and check if they
	 * have sent us an M2 state transition event */
	for( i = 0; i < shelf_fru_info.address_table.addr_entry_count; i++ ) {
		hw_addr = shelf_fru_info.address_table.addr_entry[i].hw_address;
		fru_entry = get_fru_entry_from_hw_addr( hw_addr );
		if( ( fru_entry->fru_state == FRU_STATE_M2_ACTIVATION_REQUEST ) ||
			       ( !fru_entry->activation_sent ) ) {
			// Check if an FRU Activation and Power Record is provided,
			if( !( get_fru_activation_power_rec( hw_addr ) ) ) {
				/* If a FRU is found at a Hardware Address for which an
				 * FRU Activation and Power Descriptor is not provided,
				 * the Shelf Manager should not apply power to this FRU.
				 * The Shelf Manager may consider such a situation to be 
				 * a serious fault and assume that the Shelf FRU Information
				 * is corrupted or not up-to-date. */
				continue;
			}
			// TODO send an activation command to the first one we find
			fru_entry->activation_sent = 1;
			fru_entry->activation_lbolt = lbolt;

			// send "Get SDR" command to retrieve the "FRU Device Locator" 
			// & "Management Controller Device Locator" SDRs. This is the
			// minimum we should keep in the SHM SDR repository.
			// 
			// we either get an M3 from this fru or the t_activation_readiness period elapses
			// before activating the next one
			break;

/* the shm should builds partial SDR Repository entries for
  the FRU and begin periodic verification of the presence of the FRU */
		}
			
	}

	/* If the FRU is Shelf Manager controlled, send a “Set FRU Activation
	 * (Activate FRU)” command to the FRU. */

}

/*
SDR Repository is a single, centralized non-volatile storage area maintained by the Shelf Manager.

As FRUs are inserted and extracted from the Shelf, the Shelf Manager receives events describing
these operations. Since the Shelf Manager knows about these FRUs, it needs to collect this
information together and present it to the System Manager so it does not have to probe the IPMBs
all the time to determine what FRUs are present. The Shelf Manager does this by maintaining an
SDR Repository that, at the very least, has FRU and Management Controller Locator Records for
each FRU in the system. If the Shelf Manager wishes to optimize access to other Sensor Data
Records, it may elect to “import” all Device SDRs from each device into the centralized repository.

FRU Device Locator Record(s)

Management Controller Device Locator Records

At least the partial SDR Repository (with FRU and Management Controller Locator Records)
shall be held persistently such that the Shelf Manager can generate an event after power
restore of any IPM Controller or FRU that did not successfully power back up.

An application that retrieves records from the SDR Repository must first read them out sequentially. This is
accomplished by using the Get SDR command to retrieve the first SDR of the desired type. The response to this
command returns the requested record and the Record ID of the next SDR in sequence in the repository. Note that
Record IDs are not required to be sequential or consecutive. Applications should not assume that SDR Record
IDs will follow any particular numeric ordering.
The application retrieves succeeding records by issuing a Get SDR command using the ‘next’ Record ID that was
returned with the response of the previous Get SDR command. This is continued until the ‘End of Records’ ID is
encountered.

43.8 SDR Type 11h - FRU Device Locator Record
This record is used for locating FRU information that is on the IPMB, on a Private Bus behind or management
controller, or accessed via FRU commands to a management controller. This excludes any FRU Information that
is accessed via FRU commands at LUN 00b of a management controller. The presence or absence of that FRU
Information is indicated using the Management Device Locator record (see Table 37-7, Management Controller
Device Locator - SDR 12h, below).


*/
typedef struct fru_device_locator_record {
		/* The Record ID is used by the Sensor Data Repository device for record 
		   organization and access. This may not actually be stored, but may be 
		   calculated when records are accessed. */
	uchar	record_id_lsb;	/* Record ID, LS Byte */
	uchar	record_id_msb;	/* Record ID, MS Byte */
	uchar	version;	
		/* SDR Version - Version of the Sensor Model specification that this
		   record is compatible with. 51h for this specification. This is 
		   BCD encoded with bits 7:4 holding the Least Significant digit 
		   of the revision and bits 3:0 holding the Most Significant bits.
		   E.g. 51h corresponds to “1.5”. */
	uchar	record_type;	/* Record Type Number = 11h, FRU Device Locator */
	uchar	record_len;	/* Number of remaining record bytes following. */
	uchar	device_access_addr:7,	
		/* [7:1] Device Access Address  - Slave address of controller used
		   to access device. 0000000b if device is directly on IPMB. 
		   (This field indicates whether the device is on a private bus or not) */
		:1;		/* [0] - reserved */
	uchar	fru_dev_id;	
		/* FRU Device ID / Device Slave Address
		   For LOGICAL FRU DEVICE (accessed via FRU commands to mgmt. controller):
		   [7:0] - Number identifying FRU device within given IPM Controller. FFh = reserved.
		   The primary FRU device for a management controller is always device #0 at
		   LUN 00b. The primary FRU device is not reported via this FRU Device Locator
		   record - its presence is identified via the Device Capabilities field in the
		   Management Controller Device Locator record.
		   For non-intelligent FRU device:
		   [7:1] - 7-bit I2C Slave Address. This is relative to the bus the device is on. For
		   devices on the IPMB, this is the slave address of the device on the IPMB. For
		   devices on a private bus, this is the slave address of the device on the private
		   bus.
		   [0] - reserved */
	uchar	logical_device:1,	/* [7] - logical/physical FRU device
					   0b = device is not a logical FRU Device
					   1b = device is logical FRU Device 
					   (accessed via FRU commands to mgmt. controller) */
		:2,			/* [6:5] - reserved. */
		lun:2,			/* [4:3] - LUN for Master Write-Read command or FRU Command.
					    00b if device is nonintelligent device directly on IPMB. */
		private_bus_id:3;	/* [2:0] - Private bus ID if bus = Private. 
					   000b if device directly on IPMB, or device is a
					   logical FRU Device. */
	uchar	channel_number:4,	/* [7:4] - Channel number for management controller
					   used to access device. 000b if device directly on
					   the primary IPMB, or if controller is on the 
					   primary IPMB. Msbit for channel number is kept
					   in next byte. (For IPMI v1.5. This byte position
					   was reserved for IPMI v1.0.) */
		:4;			/* [3:0] - reserved */
	uchar	reserved;
	uchar	device_type;		/* Device Type. See Table 43-12, IPMB/I2C Device
					   Type Codes. 10h for Logical FRU Device. DTC_xx codes */
	uchar	device_type_modifier;	/* Device Type Modifier. See Table 43-12. DTCM_xx codes */
	uchar	fru_entity_id;		/* Entity ID for the device associated with this FRU information. */
	uchar	fru_entity_instance;	/* Instance number for entity. */
	uchar	oem;			/* Reserved for OEM use. */
	uchar	type:2,		/* Device ID String Type/Length code per Section 43.15, Type/Length Byte Format
				   [7:6] - Type :
				   00 = Unicode
				   01 = BCD plus
				   10 = 6-bit ASCII, packed
				   11 = 8-bit ASCII + Latin 1. At least two bytes of 
				   data must be present when this type is used. Therefore,
				   the length (number of data bytes) will be >1 if data is
				   present, 0 if data is not present. A length of 1 is
				   reserved. */
		:1,
		length:6;	/* [4:0] - Length of following data, in characters. 
				   00000b indicates ‘none following’. 11111b = reserved.*/
	uchar	device_str[16];	/* Short ‘ID’ string for the FRU Device. 16 bytes, maximum. */
} FRU_DEVICE_LOCATOR_RECORD;

/*
43.9 SDR Type 12h - Management Controller Device Locator Record
This information is used for identifying management controllers on the IPMB and other internal channels, and for
providing Entity and initialization information for all management controllers, including the BMC.
Table 43-8, Management Controller Device Locator - SDR 12h
*/ 

typedef struct management_controller_device_locator_record {
		/* The Record ID is used by the Sensor Data Repository device for record 
		   organization and access. This may not actually be stored, but may be 
		   calculated when records are accessed. */
	uchar	record_id_lsb;	/* Record ID, LS Byte */
	uchar	record_id_msb;	/* Record ID, MS Byte */
	uchar	version;	
		/* SDR Version - Version of the Sensor Model specification that this
		   record is compatible with. 51h for this specification. This is 
		   BCD encoded with bits 7:4 holding the Least Significant digit 
		   of the revision and bits 3:0 holding the Most Significant bits.
		   E.g. 51h corresponds to “1.5”. */
	uchar	record_type;	/* Record Type Number = 11h, FRU Device Locator */
	uchar	record_len;	/* Number of remaining record bytes following. */
	uchar	i2c_device_slave_addr:7,  /* [7:1] - 7-bit I2C Slave Address of device on channel. */
		:1;			/* [0] - reserved. */
	uchar	:4,			/* [7:4] - reserved */
		channel_number:4;	
			/* [3:0] - Channel number for the channel that the
			   management controller is on. Use 0h for the primary BMC.
			   (New byte for IPMI v1.5. Note this addition causes some
			   of the following byte offsets to be pushed down when
			   compared to the IPMI v1.0 version of this record.) */
			       
	/* Power State Notification & Global Initialization flags */
	uchar	acpi_system_power_notification_required:1,
		/* [7] - 1b = ACPI System Power State notification required (by system s/w)
		         0b = no ACPI System Power State notification required */
		acpi_device_power_notification_required:1,
		/* [6] - 1b = ACPI Device Power State notification required (by system s/w)
		         0b = no ACPI Device Power State notification required  */
		:1,	
		/* [5] - For backward compatibility, this bit does not apply to the BMC, 
		   and should be written as 0b. */
		
		:1,	/* [4] - reserved */
		log_mgmt_init_agent_errors:1,
		/* [3] - 1b = Controller logs Initialization Agent errors (only 
		   applicable to Management Controller that implements the 
		   initialization agent function. Set to 0b otherwise.) */
		log_init_agent_errors:1,
		/* [2] - 1b = Log Initialization Agent errors accessing this 
		   controller (this directs the initialization agent to log
		   any failures setting the Event Receiver) */
		initialization:2,
		/* [1:0] - 00b = Enable event message generation from controller 
				 (Init agent will set Event Receiver address into
				 controller)
		  	   01b = Disable event message generation from controller 
			         (Init agent will set Event Receiver to FFh). This
				 provides a temporary fix for a broken controller
				 that floods the system with events. It can also
				 be used for development / debug purposes.
			   10b = Do not initialize controller. This selection is
				 for development / debug support.
			   11b = reserved. */
			       
	/* Device Capability flags */
	uchar	chassis_device:1,	/* [7] - 1b = Chassis Device. (device functions as chassis device, per ICMB spec) */
		bridge:1,		/* [6] - 1b = Bridge (Controller responds to Bridge NetFn commands) */
		ipmb_evt_generator:1,	/* [5] - 1b = IPMB Event Generator (device generates event messages on IPMB) */
		ipmb_evt_receiver:1,	/* [4] - 1b = IPMB Event Receiver (device accepts event messages from IPMB) */
		fru_inventory_device:1	/* [3] - 1b = FRU Inventory Device (accepts FRU commands to FRU Device #0 at LUN 00b) */
		sel_device:1,		/* [2] - 1b = SEL Device (provides interface to SEL) */
		sdr_repository_device:1	/* [1] - 1b = SDR Repository Device (For BMC, indicates BMC provides interface to
					   1b = SDR Repository. For other controller, indicates controller accepts
					   Device SDR commands) */
		sensor_device:1;	/* [0] - 1b = Sensor Device (device accepts sensor commands) See Table 37-11, 
					   IPMB/I2C Device Type Codes */
	uchar	reserved1;
	uchar	reserved2;
	uchar	reserved3;
	uchar	entity_id;	/* Entity ID for the FRU associated with this
				   device. 00h if not specified. If device supports
				   FRU commands at LUN 00b, this Entity ID applies
				   to both the IPM device and the FRU information
				   accessed via LUN 00b. */
	uchar	entity_instance;	/* Instance number for entity. */
	uchar	oem;		/* Reserved for OEM use. */
	uchar	type:2,		/* Device ID String Type/Length code per Section 43.15, Type/Length Byte Format
				   [7:6] - Type :
				   00 = Unicode
				   01 = BCD plus
				   10 = 6-bit ASCII, packed
				   11 = 8-bit ASCII + Latin 1. At least two bytes of 
				   data must be present when this type is used. Therefore,
				   the length (number of data bytes) will be >1 if data is
				   present, 0 if data is not present. A length of 1 is
				   reserved. */
		:1,
		length:6;	/* [4:0] - Length of following data, in characters. 
				   00000b indicates ‘none following’. 11111b = reserved.*/
	uchar	device_str[16];	/* Short ‘ID’ string for the FRU Device. 16 bytes, maximum. */	
} MANAGEMENT_CONTROLLER_DEVICE_LOCATOR_RECORD;


/* Device Type Codes 
These codes are used to identify different types of devices on an IPMB, PCI Management Bus, or Private
Management Bus connection to an IPMI management controller.
Table 43-12, IPMB/I2C Device Type Codes */

#define DTCM_UNSPECIFIED		0x00

#define DTC_DS1624 			0x02	// DS1624 temperature sensor / EEPROM or equivalent
#define DTC_DS1621			0x03	// DS1621 temperature sensor or equivalent
#define DTC_LM75			0x04	// LM75 Temperature Sensor or equivalent
#define DTC_HECETA			0x05	// ‘Heceta’ ASIC or similar 

#define DTCM_HECETA_1			0x00	// Heceta 1 e.g. LM78

#define DTCM_HECETA_2			0x01	// Heceta 2 e.g. LM79
#define DTCM_HECETA_LM80		0x02	// LM80
#define DTCM_HECETA_3			0x03	// Heceta 3 e.g. LM81/ ADM9240 / DS1780
#define DTCM_HECETA_4			0x04	// Heceta 4
#define DTCM_HECETA_5			0x05	// Heceta 5

#define DTC_EEPROM_24C01		0x08	// EEPROM, 24C01 or equivalent
#define DTCM_EEPROM_UNSPECIFIED		0x00	// unspecified
#define DTCM_EEPROM_DIMM_MEMORY_ID	0x01	// DIMM Memory ID
#define DTCM_EEPROM_FRU_INVENTORY	0x02	// IPMI FRU Inventory
#define DTCM_EEPROM_SYSTEM_PROCESSOR	0x03	// System Processor Cartridge FRU / PIROM (processor information ROM)

// DTCM_ modifiers for the following EEPROM devices same as for code 08h
#define DTC_EEPROM_24C02		0x09	// EEPROM, 24C02 or equivalent 
#define DTC_EEPROM_24C04		0x0A	// EEPROM, 24C04 or equivalent
#define DTC_EEPROM_24C08		0x0B	// EEPROM, 24C08 or equivalent
#define DTC_EEPROM_24C16		0x0C	// EEPROM, 24C16 or equivalent
#define DTC_EEPROM_24C16		0x0D	// EEPROM, 24C17 or equivalent
#define DTC_EEPROM_24C32		0x0E	// EEPROM, 24C32 or equivalent
#define DTC_EEPROM_24C64		0x0F	// EEPROM, 24C64 or equivalent

#define DTC_FRU_INVENTORY_DEVICE	0x10	/* FRU Inventory Device behind management controller
						   (accessed using Read/Write FRU commands at LUN other than 00b) */

#define DTCM_FRU_INVENTORY		0x00	// IPMI FRU Inventory
#define DTCM_FRU_DIMM_MEMORY_ID		0x01	// DIMM Memory ID
#define DTCM_FRU_INVENTORY2		0x02	// IPMI FRU Inventory
#define DTCM_FRU_SYSTEM_PROCESSOR	0x03	// System Processor Cartridge FRU / PIROM (processor information ROM)
#define DTCM_FRU_UNSPECIFIED		0xFF	// unspecified
/* Note that either DTCM_FRU_INVENTORY or DTCM_FRU_INVENTORY2 can be used. 
   The 00h Device Type Modifier is present for backward compatibility. 
   The remaining modifiers line up with those for the 08h-0Fh Device Types. */

#define DTC_PCF8570			0x14	// PCF 8570 256 byte RAM or equivalent
#define DTC_PCF8573			0x15	// PCF 8573 clock/calendar or equivalent
#define DTC_PCF8574A			0x16	// PCF 8574A ‘i/o port’ or equivalent
#define DTC_PCF8583			0x17	// PCF 8583 clock/calendar or equivalent
#define DTC_PCF8593			0x18	// PCF 8593 clock/calendar or equivalent
#define DTC_CLOCK_CALENDAR		0x19	// Clock calendar, type not specified
#define DTC_PCF8591			0x1A	// PCF 8591 A/D, D/A Converter or equivalent
#define DTC_IO_PORT			0x1B	// i/o port, specific device not specified
#define DTC_ADC				0x1C	// A/D Converter, specific device not specified
#define DTC_DAC				0x1D	// D/A Converter, specific device not specified
#define DTC_ADDAC			0x1E	// A/D, D/A Converter, specific device not specified
#define DTC_LCD				0x1F	// LCD controller / Driver, specific device not specified
#define DTC_CORE_LOGIC			0x20	// Core Logic (Chip set) Device, specific device not specified
#define DTC_LMC6874			0x21	// LMC6874 Intelligent Battery controller, or equivalent
#define DTC_BATTERY_CONTROLLER		0x22	// Intelligent Battery controller, specific device not specified
#define DTC_COMBO_MGMT_ASIC		0x23	// Combo Management ASIC, specific device not specified
#define DTC_MAXIM1617			0x24	// Maxim 1617 Temperature Sensor
#define DTC_OTHER			0xBF	// Other / unspecified device
/* codes C0h - FFh OEM specified device all other reserved */




typedef struct fru_hot_swap_event_msg_req {
	uchar	command;
	uchar	evt_msg_rev;		/* Event Message Rev = 04h (IPMI v1.5) */
	uchar	sensor_type;		/* Sensor Type = F0h (Hot Swap Event)  */
	uchar	sensor_number;		/* Sensor Number = 0xxh (Implementation specific) */
	uchar	evt_direction;		/* Event Direction (bit7) = 0b (Assertion)
					   Event Type [6:0] = 6Fh (Generic Availability) */
	uchar	code_type:4,		/* Event Data 1 - [7:4] = Ah (OEM code in Event Data 2,
					   OEM code in Event Data 3) */
		current_state:4;	/* [3:0] = Current State = FRU_STATE_xxx */
	uchar	state_change_cause:4,	/* Event Data 2 - [7:4] = Cause of state change. 
					   See Table 3-20, “Cause of State Change values,” 
					   for values. */
		previous_state:4;	/* [3:0] = Previous State = FRU_STATE_xxx */
	uchar	fru_device_id;		/* Event Data 3 - [7:0] = FRU Device ID */
} FRU_HOT_SWAP_EVENT_MSG_REQ;


/* AMC - Table 3-8 Module Hot Swap event message */
// make the following changes to event.c/ipmi_event_handler()
typedef struct module_hot_swap_event_msg_req {
	uchar	command;
	uchar	evt_msg_rev;		/* Event Message Rev = 04h (IPMI v1.5) */
	uchar	sensor_type;		/* Sensor Type = F2h (Module Hot Swap Event)  */
	uchar	sensor_number;		/* Sensor Number = 0xxh (Implementation specific) */
	uchar	evt_direction;		/* Event Direction (bit7) = 0b (Assertion)
					   Event Type [6:0] = 6Fh (Generic Availability) */
	uchar	code_type:4,		/* Event Data 1 - [7:4] = 0 (Unspecified Event Data 2,
					   Event Data 3) */
		current_state:4;	/* [3:0] = Current State = MODULE_STATE_xxx */
	uchar	evt_data2;		/* Event Data 2 - FFh - unspecified */
	uchar	evt_data3;		/* Event Data 3 - FFh - unspecified */
} MODULE_HOT_SWAP_EVENT_MSG_REQ;

#define MODULE_STATE_HANDLE_CLOSED		0
#define MODULE_STATE_HANDLE_OPENED		1
#define MODULE_STATE_QUIESCED			2
#define MODULE_STATE_BACKEND_POWER_FAILURE	3
#define MODULE_STATE_BACKEND_POWER_SHUTDOWN	4


// the following code is used by both SHM and an AMC carrier

// hot swap events need special attention
if(evt_msg->sensor_type == IPMI_SENSOR_HOT_SWAP )
{
	FRU_HOT_SWAP_EVENT_MSG_REQ *hot_swap_msg = evt_msg;
	FRU_ENTRY *fru_entry;
	
	hot_swap_msg->code_type;

	hot_swap_msg->state_change_cause;
	hot_swap_msg->previous_state;
	hot_swap_msg->fru_device_id;

	fru_entry = get_fru_entry( hot_swap_msg->fru_device_id );
	// TODO check for lookup failure, we might be seeing this fru for the first time.
	
	// Update fru entry
	fru_entry->state = hot_swap_msg->current_state;
	fru_entry->state_change_cause = hot_swap_msg->state_change_cause;
	
}

#define SHM_FRU_TABLE_ENTRIES	32

typedef struct fru_entry {
	uchar	hw_id;
	uchar	state;
	uchar	activation_sent:1,
		:7;
	FRU_DEVICE_LOCATOR_RECORD fru_device_locator_record;
	MANAGEMENT_CONTROLLER_DEVICE_LOCATOR_RECORD shm_device_locator_record;
} FRU_ENTRY;

FRU_ENTRY shm_fru_table[SHM_FRU_TABLE_ENTRIES];

FRU_ENTRY *
get_fru_entry( uchar hw_id )
{
	int i;

	for( i = 0 ; i < SHM_FRU_TABLE_ENTRIES ; i ++ ) {
		if( shm_fru_table[i].hw_id == hw_id ) 
			return( &( shm_fru_table[i] ) );
	}
	
	return( 0 );
}
/*
Renegotiation of Power Level
Whenever a FRU that is in state M3, M4, or M5 wishes to change its power levels, the IPM
Controller sends a “Renegotiate Power” command (see Table 3-66, “Renegotiate Power
command”) to the Shelf Manager for the FRU. This command causes the Shelf Manager to begin a
negotiation with the specified FRU or all FRUs under the IPM Controller. At the end of the
renegotiation the Shelf Manager sends a Set Power Level command to the IPM Controller so it
knows what the conclusion was. In the worst case, the FRU will get a Power Level = FFh and a Set
Present Levels to Desired Levels of “Do not change present levels” indicating that the renegotiation
failed.
*/



/*

when an IPM Controller
detects that a monitored temperature has exceeded one or more upper thresholds. At this point, the
IPM Controller raises a standard IPMI Temperature Event Message and sends it to the Event
Receiver (typically the Shelf Manager). The Shelf Manager uses this event to adjust the fan speeds,
power levels of FRUs, or, in the worst case, begins shutting down FRUs in an attempt to return the
Shelf to the Normal Operation Stage.

In IPMI, thresholds are classified as Non-Critical, Critical, or Non-Recoverable.
AdvancedTCA® uses three levels, but classifies them as minor, major, and critical. This choice of
names avoids the implication that the upper-most level cannot be recovered from and also aligns
with the naming of telco alarm levels

If attempts to increase cooling of the Shelf are
unsuccessful, the Shelf Manager shall attempt to reduce the power consumption of the
distressed FRU (or FRUs, if the condition is not focused on an individual FRU). The “Set
Power Level (Power Level)” command is sent to the FRU or FRUs involved, and those that
are capable of setting lower power levels will reduce their power level in an attempt to
alleviate the over-temperature condition.

When the alert condition reaches critical, the Shelf Manager shall request deactivation of the
FRU via the “Set FRU Activation (Deactivate FRU)” command. The FRU gracefully
completes operations and shuts down. When the condition is not FRU focused, the Shelf
Manager should begin powering down FRUs in the reverse order of the FRU Activation and
Power Descriptors (see Table 3-59, “FRU Activation and Power Descriptor”) until the
condition returns to normal.
• When the Shelf returns to a Normal Operating Condition, the Shelf Manager should return the
cooling to a normal level.
• When the Shelf cooling is increased in response to a distressed FRU that is subsequently
removed from the Shelf, the Shelf Manager should return the cooling for the FRU to a normal
level.

• When the Shelf returns to a Normal Operating Condition from a major temperature event, the
Shelf Manager may restore the power to any FRUs that were stepped down. Care should be
taken that any restoration of power to the FRUs does not cause the over-temperature condition
to occur repeatedly.
• When the Shelf returns to a Normal Operating Condition from a Critical Temperature Event,
the Shelf Manager may power back up any FRUs that were powered off. Care should be taken
that any restoration of power to the FRUs does not cause the over-temperature condition to
occur repeatedly.

*/

/* 
Every IPM Controller must contain at least one temperature sensor and an appropriate Sensor Data
Record (SDR) to describe the sensor.
*/



/*SDR Repository A single, centralized non-volatile storage area maintained by the Shelf Manager.*/

/* Shall monitor and control the Shelf IPMB interconnects */


/*
SDRs
IPM Controllers are required to maintain Device Sensor Data Records for the sensors and objects
they manage. Access methods for the Device SDR entries are described in the IPMI specification,
Section 29, “Sensor Device Commands.” After a FRU is inserted, the System Manager, using the
Shelf Manager, may gather the various SDRs from the FRU’s IPM Controller to learn of the
various objects and how to use them. The System Manager would use the “Sensor Device
Commands” to gather this information. Thus, commands, such as “Get Device SDR Info” and “Get
Device SDR”, which are optional in the IPMI specification, are mandatory in AdvancedTCA®
systems.
The implementer may choose to have the Shelf Manager gather the individual Device Sensor Data
Records into a centralized SDR Repository. This SDR Repository may exist in either the Shelf
Manager or System Manager. If the Shelf Manager implements the SDR Repository on-board, it
shall also respond to “SDR Repository” commands.

As FRUs are inserted and extracted from the Shelf, the Shelf Manager receives events describing
these operations. Since the Shelf Manager knows about these FRUs, it needs to collect this
information together and present it to the System Manager so it does not have to probe the IPMBs
all the time to determine what FRUs are present. The Shelf Manager does this by maintaining an
SDR Repository that, at the very least, has FRU and Management Controller Locator Records for
each FRU in the system. If the Shelf Manager wishes to optimize access to other Sensor Data
Records, it may elect to “import” all Device SDRs from each device into the centralized repository.

*/ 


ipmi_send_cmd( IPMI_PKT *)
{

}
/* AMC additions */

/* add to ipmi.h
 * 
 * Table 3-1 Get PICMG Properties command for MMCs
 * 3 - PICMG Extension Version. Indicates the version of PICMG extensions implemented 
 * by the MMC.[7:4] = BCD encoded minor version[3:0] = BCD encoded major version
 * This specification defines version 4.1 of the PICMG extensions for MMCs. 
 * MMCs implementing the extensions defined by this specification must report a 
 * value of 14h. The value of 00h is reserved.
 * 4 - Max FRU Device ID. MMCs must report 0.
 * 5 - FRU Device ID for MMC. MMCs must report 0.
 */

/* geographic addressing */
/*
The IPMB-L address of a Module can be calculated
as (70h + Site Number x 2). For example, a Module with GA address GGU (Site Number 1)
is accessed at IPMB-L address 72h.
*/

/* Table 3-3 Carrier Information Table - this is maintained on the carrier */
typedef struct carrier_information_table {
	uchar	record_type_id;	/* Record Type ID. For all records defined
				   in this specification a value of C0h (OEM)
				   shall be used. */
	uchar 	eol:1,		/* [7:7] End of list. Set to one for the last record */
	      	reserved:3,	/* [6:4] Reserved, write as 0h.*/
		version:4;	/* [3:0] record format version (=2h for this definition) */
	uchar	record_len;	/* Record Length. */
	uchar	record_cksum;	/* Record Checksum. Holds the zero checksum of the record. */
	uchar	header_cksum;	/* Header Checksum. Holds the zero checksum of the header. */
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


/* Review Table 3-5 Get Address Info command extensions for Carrier IPMCs */

/* AMC activation - From 3.6.1 Typical Module insertion
This is very similar to ATCA IPMC _SHM interaction except, there are
different power and connection records for the AMC.

The Carrier IPMC reads the Module’s Module Current Requirements record and 
AdvancedMC Point-to-Point Connectivity record. The Carrier IPMC checks that
the power requested by a Module can be delivered by the Carrier. This involves
checking that the Maximum Module Current is greater than Current Draw and that
Maximum Internal Current is greater than Current Draw of all Modules that have
been allocated power plus the Current Draw for the Module being negotiated. 
See Table 3-10, “Module Current Requirements record” and Table 3-16, “AdvancedMC
Point-to-Point Connectivity record.”

Note: New "Module Hot Swap event" used.
Each MMC contains one Module Hot Swap sensor. This sensor proactively generate events
(Module Handle Closed, Module Handle Opened, Quiesced, Backend Power Shut Down,
and Backend Power Failure) to enable the Carrier IPMC to perform Hot Swap management
for the Modules it represents.

There is a FRU Hot Swap sensor (a distinct object from the Module Hot Swap sensor) for
each FRU that a Carrier IPMC manages (including the Module FRUs). Since the Carrier
IPMC is responsible for maintaining the Hot Swap states for its Module FRUs, the current
state of a Module can be determined by sending a “Get Sensor Reading (FRU Hot Swap
sensor)” command to the Carrier IPMC with the appropriate Hot Swap sensor number. The
AdvancedTCA specification Section 3.2.4.3.2 “Reading the FRU Hot Swap Sensor”
provides more details on this subject.

*/

/* AMC Table 3-10 Module Current Requirements record */
typedef struct module_current_requirements_record {
	uchar	record_type_id;	/* Record Type ID. For all records defined
				   in this specification a value of C0h (OEM)
				   shall be used. */
	uchar 	eol:1,		/* [7:7] End of list. Set to one for the last record */
	      	reserved:3,	/* [6:4] Reserved, write as 0h.*/
		version:4;	/* [3:0] record format version (=2h for this definition) */
	uchar	record_len;	/* Record Length. */
	uchar	record_cksum;	/* Record Checksum. Holds the zero checksum of the record. */
	uchar	header_cksum;	/* Header Checksum. Holds the zero checksum of the header. */
	uchar	manuf_id[3];	/* Manufacturer ID. LS Byte first. Write as the
				   three byte ID assigned to PICMG®. For this
				   specification, the value 12634 (00315Ah) shall
				   be used. */
	uchar	picmg_rec_id;	/* PICMG Record ID. For the Shelf Power 
				   Distribution Record, the value 11h shall be
				   used. */
	uchar	rec_fmt_ver;	/* Record Format Version. For this specification,
				   the value 0h shall be used. */
	uchar	current_draw;	/* Current Draw. This field holds the Payload Power
				   (PWR) requirement of the Module given as current
				   requirement in units of 0.1A at 12V. (This equals
				   the value of the power in W divided by 1.2.) */
} MODULE_CURRENT_REQUIREMENTS_RECORD;

/* AMC Table 3-11 Carrier Activation and Current Management record */

typedef struct carrier_activation_current_mgmt_record {
	uchar	record_type_id;	/* Record Type ID. For all records defined
				   in this specification a value of C0h (OEM)
				   shall be used. */
	uchar 	eol:1,		/* [7:7] End of list. Set to one for the last record */
	      	reserved:3,	/* [6:4] Reserved, write as 0h.*/
		version:4;	/* [3:0] record format version (=2h for this definition) */
	uchar	record_len;	/* Record Length. */
	uchar	record_cksum;	/* Record Checksum. Holds the zero checksum of the record. */
	uchar	header_cksum;	/* Header Checksum. Holds the zero checksum of the header. */
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

/* AMC Table 3-13 Carrier Point-to-Point Connectivity record  */

typedef struct carrier_p2p_conn_record {
	uchar	record_type_id;	/* Record Type ID. For all records defined
				   in this specification a value of C0h (OEM)
				   shall be used. */
	uchar 	eol:1,		/* [7:7] End of list. Set to one for the last record */
	      	reserved:3,	/* [6:4] Reserved, write as 0h.*/
		version:4;	/* [3:0] record format version (=2h for this definition) */
	uchar	record_len;	/* Record Length. */
	uchar	record_cksum;	/* Record Checksum. Holds the zero checksum of the record. */
	uchar	header_cksum;	/* Header Checksum. Holds the zero checksum of the header. */
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

/* AMC Table 3-14 Point-to-Point AdvancedMC Resource Descriptor */
typedef struct p2p_amc_resource_descr {
	uchar	resource_id;	/* Resource ID. Indicates the AMC Slot ID or 
				   on-Carrier device. */
	uchar	resource_type:1, /* [7] Resource Type. 1 AMC, 0 indicates on-Carrier device ID */
		:2,		/* [5:4] Reserved; write 0h */
		dev_id:4;	/* [3:0] On-Carrier device ID or AMC Site Number */
	uchar	p2p_port_count;	/* Point-to-Point Port Count. Indicates the number
				   of point-to-point Ports associated with this Resource. */
	P2P_PORT_DESCRIPTOR	p2p_port[8];
				/* 3*n Point-to-Point Port Descriptors. An array 
				   of n Point-to-Point Port Descriptors (each with
				   Least significant byte first) (see Table 3-15)
				   where n is specified in the Point-to-Point Port
				   Count byte. TODO check size of array */
} P2P_AMC_RESOURCE_DESCR;

/* AMC Table 3-15 Point-to-Point Port Descriptor */
typedef struct p2p_port_descriptor {
	uint_16_t 	:6,	/* [23:18] Reserved. Must be 0. */
		local_port:5,	/* [17:13] Local Port. Indicates the Port number
				   within the local AMC Slot or on-Carrier device. */
		remote_port:5;	/* [12:8]  Remote Port. Indicates the Port number
				   within the remote AMC Slot or on-Carrier device
				   ID to which this point-to-point connection is routed. */
				/* [7:0] Remote Resource ID. In AMC.0 systems: */
	uchar	resource_type:1,/* [7] Resource Type. 1 AMC, 0 indicates on-Carrier device */
		:3,		/* [6:4] Reserved; write 0h */
		amc_site_num:1;	/* [3:0] On-Carrier device ID or AMC Site Number */
} P2P_PORT_DESCRIPTOR;

/* AMC Table 3-16 AdvancedMC Point-to-Point Connectivity record */
typedef struct amc_p2p_conn_record {
	uchar	record_type_id;	/* Record Type ID. For all records defined
				   in this specification a value of C0h (OEM)
				   shall be used. */
	uchar 	eol:1,		/* [7:7] End of list. Set to one for the last record */
	      	reserved:3,	/* [6:4] Reserved, write as 0h.*/
		version:4;	/* [3:0] record format version (=2h for this definition) */
	uchar	record_len;	/* Record Length. */
	uchar	record_cksum;	/* Record Checksum. Holds the zero checksum of the record. */
	uchar	header_cksum;	/* Header Checksum. Holds the zero checksum of the header. */
	uchar	manuf_id[3];	/* Manufacturer ID. LS Byte first. Write as the
				   three byte ID assigned to PICMG®. For this
				   specification, the value 12634 (00315Ah) shall
				   be used. */
	uchar	picmg_rec_id;	/* PICMG Record ID. For the Shelf Power 
				   Distribution Record, the value 11h shall be
				   used. */
	uchar	rec_fmt_ver;	/* Record Format Version. For this specification,
				   the value 0h shall be used. */
	uchar	oem_guid_count;	/* OEM GUID Count. The number, n, of OEM GUIDs
				   defined in this record. */
	OEM_GUID oem_guid_list[n];
				/* A list 16*n bytes of OEM GUIDs. */
	uchar	record_type:1,	/* [7] Record Type – 1 AMC Module, 0 On-Carrier device */
		:3,		/* [6:4] Reserved; write as 0h */
		conn_dev_id:4;	/* [3:0] Connected-device ID if Record Type = 0, Reserved, otherwise. */
	uchar	ch_descr_count;	/* AMC Channel Descriptor Count. The number, m, 
				   of AMC Channel Descriptors defined in this record. */
	AMC_CH_DESCR ch_descr[m]; 
				/* AMC Channel Descriptors. A variable length 
				   list of m three-byte AMC Channel Descriptors,
				   each defining the Ports that make up an AMC
				   Channel (least significant byte first).*/
	AMC_LINK_DESCR link_desrc[p];
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

/* AMC Table 3-17 AMC Channel Descriptor */
/*
23:20 Reserved. Must be 1111b.
19:15 Lane 3 Port Number. The Port within this AMC resource that functions as Lane 3 of this
AMC Channel.
14:10 Lane 2 Port Number. The Port within this AMC resource that functions as Lane 2 of this
AMC Channel.
9:5 Lane 1 Port Number. The Port within this AMC resource that functions as Lane 1 of this
AMC Channel.
4:0 Lane 0 Port Number. The Port within this AMC resource that functions as Lane 0 of this
AMC Channel.
*/

/* AMC Table 3-19 AMC Link Descriptor */
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
	uint16_t link_type_ext:4,
				/* [23:20] AMC Link Type Extension. Identifies the
				   subset of a subsidiary specification that is
				   implemented and is defined entirely by the 
				   subsidiary specification identified in the Link
				   Type field. */
		link_type:8,	/* [19:12] AMC Link Type. Identifies the AMC.x
				   subsidiary specification that governs this
				   description or identifies the description as
				   proprietary; see Table 3-21, “AMC Link Type.” */
		link_designator;
				/* [11:0] AMC Link Designator. Identifies the AMC
				   Channel and the Ports within the AMC Channel
				   that are being described; see Table 3-20,
				   “AMC Link Designator.” */
} AMC_LINK_DESCR;

/* AMC Table 3-20 AMC Link Designator */
/*
11 Lane 3 Bit Flag (1 = Lane Included; 0 = Lane Excluded)
10 Lane 2 Bit Flag (1 = Lane Included; 0 = Lane Excluded)
9 Lane 1 Bit Flag (1 = Lane Included; 0 = Lane Excluded)
8 Lane 0 Bit Flag (1 = Lane Included; 0 = Lane Excluded)
7:0 AMC Channel ID. Identifies an AMC Channel Descriptor defined in an AMC Point-to-
Point Connectivity record.
*/

/* Table 3-21 AMC Link Type */
/*
0x00	Reserved
0x01	Reserved
0x02	AMC.1 PCI Express 
0x03, 0x04	AMC.1 PCI Express Advanced Switching
0x05	AMC.2 Ethernet
0x06	AMC.4 Serial RapidIO
0x07	AMC.3 Storage
0x08...0xEF	Reserved
0xF0...0xFE	E-Keying OEM GUID DefinitionFFhReserved
*/

/* Table 3-27 Set AMC Port State command */
/* Byte 7 can be present in AMC - amend ipmi.h Set Port State Command accordingly 
(7)Present if AMC Channel ID is associated with an on-Carrier device, absent otherwise.[7:4] Reserved; write as 0h[3:0] On-Carrier device ID. Identifies the on-Carrier device to which the described AMC Channel is connected.

Set AMC Port State command
Byte	Data field	
Request Data
1PICMG Identifier. Indicates that this is a PICMG®-defined group extension command. A value of 00h must be used.
2–5Link Info. Least significant byte first. Describes the Link that should be enabled or disabled.[31:24] – Link Grouping ID[23:20] – Link Type Extension[19:12] – Link Type[11] – Lane 3 Bit Flag[10] – Lane 2 Bit Flag[9] – Lane 1 Bit Flag[8] – Lane 0 Bit Flag[7:0] – AMC Channel ID6State. Indicates the desired state of the Link as described by Link Info.00h = Disable01h = EnableAll other values reserved.(7)Present if AMC Channel ID is associated with an on-Carrier device, absent otherwise.[7:4] Reserved; write as 0h[3:0] On-Carrier device ID. Identifies the on-Carrier device to which the described AMC Channel is connected.Response Data1Completion Code2PICMG Identifier. Indicates that this is a PICMG®-defined group extension command. A value of 00h must be used.
*/

/*
MicroTCA Carrier Hub
The MicroTCA™ Carrier Hub (MCH) is
defined in the PICMG specification
MicroTCA.0 as a fundamental component
of a MicroTCA enclosure, or “carrier.” The
specification defines common facilities
required of an MCH and shared by the
modules in the payload slots of the carrier.
These common facilities include basic
management of power, cooling, module
control, and clock distribution, in addition to
system status indicators, remote interfaces,
and gigabit Ethernet connectivity to each
payload device. In most cases, the “base”
Ethernet connectivity for the carrier is
provided by an unmanaged Layer-2 switch.
*/

