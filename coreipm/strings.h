/*
-------------------------------------------------------------------------------
coreIPM/strings.h

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

extern STR_LST app_str[];
extern STR_LST atca_str[];
extern STR_LST media_specific_str[];
extern STR_LST nvstore_str[];
extern STR_LST firmware_str[];
extern STR_LST event_str[];
extern STR_LST chassis_str[];
extern STR_LST bridge_str[];
extern STR_LST group_extension_str[];
extern STR_LST oem_str[];
extern char *string_find( STR_LST *str_lst, int id );
