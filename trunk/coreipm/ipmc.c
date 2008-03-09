/*
-------------------------------------------------------------------------------
coreIPM/ipmc.c

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

There is a FRU Hot Swap sensor (a distinct object from the Module Hot Swap sensor) for
each FRU that a Carrier IPMC manages (including the Module FRUs). Since the Carrier
IPMC is responsible for maintaining the Hot Swap states for its Module FRUs, the current
state of a Module can be determined by sending a “Get Sensor Reading (FRU Hot Swap
sensor)” command to the Carrier IPMC with the appropriate Hot Swap sensor number. The
AdvancedTCA specification Section 3.2.4.3.2 “Reading the FRU Hot Swap Sensor”
provides more details on this subject.

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
