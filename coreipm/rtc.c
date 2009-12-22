/*
-------------------------------------------------------------------------------
coreIPM/rtc.c

Author: Gokhan Sozmen
-------------------------------------------------------------------------------
Copyright (C) 2007 Gokhan Sozmen
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
#include "arch.h"
#include "rtc.h"


/*
Time is an unsigned 32-bit value representing the local time as the number of
seconds from 00:00:00, January 1, 1970. This format is sufficient to maintain
timestamping with 1-second resolution past the year 2100. 
The timestamps used for SDR and SEL records are assumed to be specified in 
relative local time. 

0xFFFFFFFF indicates an invalid or unspecified time value.

0x00000000 through 0x20000000 are used for timestamping events that occur after
the initialization of the System Event Log device up to the time that the 
timestamp is set with the system time value. Thus, these timestamp values are
relative to the completion of the SEL device’s initialization, not 1/1/1970.
*/

typedef struct tm {
	int    tm_sec;   	// seconds [0,61]
	int    tm_min;  	// minutes [0,59]
	int    tm_hour;  	// hour [0,23]
	int    tm_mday;  	// day of month [1,31]
	int    tm_mon;  	// month of year [0,11]
	int    tm_year;  	// years since 1900
	int    tm_wday;  	// day of week [0,6] (Sunday = 0)
	int    tm_yday;  	// day of year [0,365]
	int    tm_isdst; 	// daylight savings flag
} TM;

unsigned mktime( TM *tptr );

void
rtc_init( void )
{
	/* Set the prescaler. The prescaler divides the peripheral clock (PCLK)
	 * by a value which contains both an integer portion and a fractional 
	 * portion. */ 
	RTC_PREINT = ( PCLK/32768 ) - 1;			// 13 bits valid
	RTC_PREFRAC = PCLK - ( (PREINT + 1) * 32768 );	// 15 bits valid
}
	
unsigned int
rtc_get_timestamp( void )
{
	TM t;

	t.tm_yday = RTC_DOY;
	t.tm_wday = RTC_DOW;
	t.tm_year = RTC_YEAR;
	t.tm_mon = RTC_MONTH;
	t.tm_mday = RTC_DOM;
	t.tm_hour = RTC_HOUR;
	t.tm_min = RTC_MIN;
	t.tm_sec = RTC_SEC;

	return( mktime( &t ) );
}

void
rtc_set_clock( TM *tptr )
{
	RTC_DOY = tptr->tm_yday;
	RTC_DOW = tptr->tm_wday;
	RTC_YEAR = tptr->tm_year;
	RTC_MONTH = tptr->tm_mon;
	RTC_DOM = tptr->tm_mday;
	RTC_HOUR = tptr->tm_hour;
	RTC_MIN = tptr->tm_min;
	RTC_SEC = tptr->tm_sec;
}

void
rtc_get_clock( TM *tptr )
{
	tptr->tm_yday = RTC_DOY;
	tptr->tm_wday = RTC_DOW;
	tptr->tm_year = RTC_YEAR;
	tptr->tm_mon = RTC_MONTH;
	tptr->tm_mday = RTC_DOM;
	tptr->tm_hour = RTC_HOUR;
	tptr->tm_min = RTC_MIN;
	tptr->tm_sec = RTC_SEC;
}
/*
The mktime() function converts the broken-down time, expressed as local time,
in the structure pointed to by timeptr, into a time since the Epoch value with
the same encoding as that of the values returned by time().
*/
unsigned mktime( TM *tptr )
{
	unsigned result;
	register short	month, year;
	static int	m_to_d[12] =
		{0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};

	month = tptr->tm_mon;
	year = tptr->tm_year + month / 12 + 1900;
	month %= 12;
	if ( month < 0 ) {
		year -= 1;
		month += 12;
	}
	result = ( year - 1970 ) * 365 + ( year - 1969 ) / 4 + m_to_d[month];
	result = ( year - 1970 ) * 365 + m_to_d[month];
	if ( month <= 1 )
		year -= 1;
	result += ( year - 1968 ) / 4;
	result -= ( year - 1900 ) / 100;
	result += ( year - 1600 ) / 400;
	result += tptr->tm_mday;
	result -= 1;
	result *= 24;
	result += tptr->tm_hour;
	result *= 60;
	result += tptr->tm_min;
	result *= 60;
	result += tptr->tm_sec;
	
	if( tptr->tm_isdst > 0 )
		result -= 3600;
	return(result);
}


