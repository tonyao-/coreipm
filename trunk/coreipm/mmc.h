/*
-------------------------------------------------------------------------------
coreIPM/mmc.h

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

#define MAX_FRU_DEV_ID  0
void mmc_set_port_state( IPMI_PKT *pkt );
void mmc_get_port_state( IPMI_PKT *pkt );
void mmc_set_clock_state( IPMI_PKT *pkt );
void mmc_get_clock_state( IPMI_PKT *pkt );
void mmc_get_fru_control_capabilities( IPMI_PKT *pkt );
