/*
-------------------------------------------------------------------------------
coreIPM/strings.c

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

#include "ipmi.h"

STR_LST app_str[] = {
	{ IPMI_CMD_GET_DEVICE_ID, IPMI_CMD_GET_DEVICE_ID_STR },
	{ IPMI_CMD_GET_SELF_TEST_RESULTS, IPMI_CMD_GET_SELF_TEST_RESULTS_STR },
	{ IPMI_CMD_RESET_WATCHDOG_TIMER, IPMI_CMD_RESET_WATCHDOG_TIMER_STR },
	{ IPMI_CMD_SET_WATCHDOG_TIMER,IPMI_CMD_SET_WATCHDOG_TIMER_STR },
	{ IPMI_CMD_GET_WATCHDOG_TIMER, IPMI_CMD_GET_WATCHDOG_TIMER_STR },
	{ IPMI_CMD_SEND_MESSAGE, IPMI_CMD_SEND_MESSAGE_STR },
	{ -1, 0	}
};

STR_LST atca_str[] = {
	{ ATCA_CMD_GET_PICMG_PROPERTIES, ATCA_CMD_GET_PICMG_PROPERTIES_STR },
	{ ATCA_CMD_GET_ADDRESS_INFO, ATCA_CMD_GET_ADDRESS_INFO_STR },
	{ ATCA_CMD_GET_SHELF_ADDRESS_INFO, ATCA_CMD_GET_SHELF_ADDRESS_INFO_STR },
	{ ATCA_CMD_SET_SHELF_ADDRESS_INFO, ATCA_CMD_SET_SHELF_ADDRESS_INFO_STR },
	{ ATCA_CMD_FRU_CONTROL, ATCA_CMD_FRU_CONTROL_STR },
	{ ATCA_CMD_GET_FRU_LED_PROPERTIES, ATCA_CMD_GET_FRU_LED_PROPERTIES_STR },
	{ ATCA_CMD_GET_LED_COLOR, ATCA_CMD_GET_LED_COLOR_STR },
	{ ATCA_CMD_SET_FRU_LED_STATE, ATCA_CMD_SET_FRU_LED_STATE_STR },
	{ ATCA_CMD_GET_FRU_LED_STATE, ATCA_CMD_GET_FRU_LED_STATE_STR },
	{ ATCA_CMD_SET_IPMB_STATE, ATCA_CMD_SET_IPMB_STATE_STR },
	{ ATCA_CMD_SET_FRU_ACTIVATION_POLICY, ATCA_CMD_SET_FRU_ACTIVATION_POLICY_STR },
	{ ATCA_CMD_GET_FRU_ACTIVATION_POLICY, ATCA_CMD_GET_FRU_ACTIVATION_POLICY_STR },
	{ ATCA_CMD_SET_FRU_ACTIVATION, ATCA_CMD_SET_FRU_ACTIVATION_STR },
	{ ATCA_CMD_GET_DEVICE_LOCATOR_REC_ID, ATCA_CMD_GET_DEVICE_LOCATOR_REC_ID_STR },
	{ ATCA_CMD_SET_PORT_STATE, ATCA_CMD_SET_PORT_STATE_STR },
	{ ATCA_CMD_GET_PORT_STATE, ATCA_CMD_GET_PORT_STATE_STR },
	{ ATCA_CMD_COMPUTE_POWER_PROPERTIES, ATCA_CMD_COMPUTE_POWER_PROPERTIES_STR },
	{ ATCA_CMD_SET_POWER_LEVEL, ATCA_CMD_SET_POWER_LEVEL_STR },
	{ ATCA_CMD_GET_POWER_LEVEL, ATCA_CMD_GET_POWER_LEVEL_STR },
	{ ATCA_CMD_RENEGOTIATE_POWER, ATCA_CMD_RENEGOTIATE_POWER_STR },
	{ ATCA_CMD_GET_FAN_SPEED_PROPERTIES, ATCA_CMD_GET_FAN_SPEED_PROPERTIES_STR },
	{ ATCA_CMD_SET_FAN_LEVEL, ATCA_CMD_SET_FAN_LEVEL_STR },
	{ ATCA_CMD_GET_FAN_LEVEL, ATCA_CMD_GET_FAN_LEVEL_STR },
	{ ATCA_CMD_BUSED_RESOURCE_CONTROL, ATCA_CMD_BUSED_RESOURCE_STR },
	{ ATCA_CMD_GET_IPMB_LINK_INFO, ATCA_CMD_GET_IPMB_LINK_INFO_STR },
	{ -1, 0	}
}; 

STR_LST media_specific_str[] = {
	{ -1, 0 }
};

STR_LST nvstore_str[] = {
	{ -1, 0 }
};

STR_LST firmware_str[] = {
	{ -1, 0 }
};

STR_LST event_str[] = {
	{ -1, 0	}
};

STR_LST chassis_str[] = {
	{ -1, 0 }
};

STR_LST bridge_str[] = {
	{ -1, 0 }
};

STR_LST group_extension_str[] = {
	{ -1, 0 }
};

STR_LST oem_str[] = {
	{ -1, 0	}
};

char *string_find( STR_LST *ptr, int id )
{
	int i = 0;

	while( ptr[i].id != -1 ) {
		if( ptr[i].id == id )
			return ptr[i].str;
		i++;
	}

	return 0;
}
