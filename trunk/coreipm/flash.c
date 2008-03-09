/*
-------------------------------------------------------------------------------
coreIPM/flash.c

Author: Gokhan Sozmen
-------------------------------------------------------------------------------
Copyright (C) 200a-2008 Gokhan Sozmen
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
#include <string.h> // for memcpy
#include "flash.h"
#include "arch.h"
#include "debug.h"


#define IAP_LOCATION	0x7ffffff1
#define PRC_CLK         60000            /* 60 MHz in kHz */

/* IAP commands */
#define IAP_CMD_PREPARE_SECTORS		50
#define IAP_CMD_COPY_RAM_TO_FLASH	51
#define IAP_CMD_ERASE_SECTORS		52
#define IAP_CMD_BLANK_CHECK		53
#define IAP_CMD_READ_PART_ID		54     
#define IAP_CMD_BOOT_CODE_VER		55
#define IAP_CMD_COMPARE			56 
#define IAP_CMD_REINVOKE_ISP		57


/* IAP Status Codes */
#define IAP_STAT_CMD_SUCCESS		0  /* command is executed successfully    */
#define IAP_STAT_INVALID_COMMAND	1     
#define IAP_STAT_SRC_ADDR_ERROR		2  /* source address is not on a word boundary */
#define IAP_STAT_DST_ADDR_ERROR		3  /* destination address is not on a 
					      correct boundary */
#define IAP_STAT_SRC_ADDR_NOT_MAPPED	4  /* source address is not mapped in the
					      memory map. count value is taken
					      into consideration where applicable */
#define IAP_STAT_DST_ADDR_NOT_MAPPED	5  /* destination address is not mapped in
					      the memory map.  count value is taken
					      into consideration where applicable */
#define IAP_STAT_COUNT_ERROR		6  /* byte count is not multiple of 4 or 
					      is not a permitted value */
#define IAP_STAT_INVALID_SECTOR		7  /* sector number is invalid */
#define IAP_STAT_SECTOR_NOT_BLANK	8     
#define IAP_STAT_SECTOR_NOT_RDY		9  /* sector not ready for write operation. */
#define IAP_STAT_COMPARE_ERROR		10 /* source and destination data is not same */
#define IAP_STAT_BUSY			11 /* flash programming hardware interface is busy */

#define FLASH_PAGE_SIZE			4096	/* 2^FLASH_PAGE_SHIFT */ 
#define FLASH_PAGE_SHIFT		12	/* these two _PAGE_ values are co-dependent */

typedef void ( * IAP ) ( unsigned int [], unsigned int [] );

IAP iap_entry_addr = ( IAP )IAP_LOCATION;

typedef struct flash_sector_table {
	int size;
	int num;
} FLASH_SECTOR_TABLE;

FLASH_SECTOR_TABLE lpc2148_flash_sectors[] =
	{ {4096,8}, {32768,14}, {4096,5}, {0,0} };

unsigned char flash_buffer[ FLASH_PAGE_SIZE ];

unsigned int iap_cmd( unsigned int cmd, unsigned int p0, unsigned int p1, 
	unsigned int p2, unsigned int p3, unsigned int *r0 );

int 
flash_get_sector_number( unsigned int in_addr )
{
	FLASH_SECTOR_TABLE * table_ptr;
	unsigned int end_addr;
	unsigned int start_addr = 0;
	int n_sector = 0;
	int i;

	table_ptr = ( FLASH_SECTOR_TABLE * )&lpc2148_flash_sectors[0];

	while( table_ptr->num != 0 )
	{
		for( i = 0; i < table_ptr->num; i++ ) {
			end_addr = start_addr + table_ptr->size;
			if( in_addr >= start_addr && in_addr < end_addr ) {
				return n_sector;
			}
			n_sector++;
			start_addr = end_addr;
		}
		table_ptr++;
	}

	return ( -1 );
}

/* read the part identification number */
int
flash_read_part_id( void )
{
	unsigned int part_id = 0, rc = 0;
	
	rc = iap_cmd( IAP_CMD_READ_PART_ID, 0, 0, 0, 0, &part_id);
	if( rc == IAP_STAT_CMD_SUCCESS ) {
		switch( part_id ) {
			case PART_ID_LPC2141:
				putstr( "2141\n" );			
				break;
			case PART_ID_LPC2142:
				putstr( "2142\n" );			
				break;
			case PART_ID_LPC2144:
				putstr( "2144\n" );			
				break;
			case PART_ID_LPC2146:
				putstr( "2146\n" );			
				break;
			case PART_ID_LPC2148:
				putstr( "2148\n" );			
				break;
			default:
				putstr( "Unkonwn part number\n" );
				break;
		}
		return( part_id );
	}
	return( 0 );
}


// TODO make sure we disable the watchdog timer so it does not expire during flash programming
unsigned int
iap_cmd (
	unsigned int cmd,
	unsigned int p0,
	unsigned int p1,
	unsigned int p2,
	unsigned int p3,
	unsigned int *r0 )
{
	static IAP iap_entry_addr = (IAP)IAP_LOCATION;
	unsigned int command[5];
	unsigned int result[2];
	unsigned int VIC_enable_state;

	VIC_enable_state = VICIntEnable;	// save interrupt enable state
	VICIntEnClr = 0xFFFFFFFF;		// disable all interrupts handled by VIC

	command[0] = cmd;
	command[1] = p0;
	command[2] = p1;
	command[3] = p2;
	command[4] = p3;

	iap_entry_addr( command, result );

	VICIntEnable = VIC_enable_state;	// re-enable interrupts

	if(r0)
		*r0 = result[1];
	
	return result[0];
}

int 
flash_block_write(
	unsigned int flash_addr,
	unsigned int ram_addr,
	int len )     /* 256,512, etc up to min sector size (4096) */
{
	int n_sector;
	unsigned int rc = 0;

	n_sector = flash_get_sector_number( flash_addr );

	if( n_sector < 0 ) {
	      return FWERR_SECTOR_NUM;
	}
	
	/* write prep */
 	rc = iap_cmd( IAP_CMD_PREPARE_SECTORS, n_sector, n_sector, 0, 0, 0 );

	if( rc == IAP_STAT_CMD_SUCCESS )
	{
		while( 1 ) {
			rc = iap_cmd( IAP_CMD_COPY_RAM_TO_FLASH, flash_addr, 
					ram_addr, len, PRC_CLK, 0 );
			if( rc != IAP_STAT_BUSY ) {
				break;
			}
		}
	}
	
	if( rc == IAP_STAT_CMD_SUCCESS ) 
		rc = FWERR_NOERR;
	else
		rc = FWERR_IAP;

	return rc;
}

/*
 * flash_write()
 *
 * 	RETURN VALUE
 * 		FWERR_NOERR	: Upon successful completion
 * 		FWERR_xx	: one or more page writes failed
 */

int 
flash_write(
	unsigned int flash_addr,
	unsigned int ram_addr,
	int len )
{
	int num_pages, i, len_remaining = len;
	int offset, copy_size, ret = 0;
	unsigned int current_page_base, page_aligned_start, page_aligned_end, source;

	page_aligned_start = flash_addr & ~( FLASH_PAGE_SIZE - 1 );
	page_aligned_end = ( ( flash_addr + len ) & ~( FLASH_PAGE_SIZE - 1 ) ) + FLASH_PAGE_SIZE;

	num_pages = ( page_aligned_end - page_aligned_start ) >> FLASH_PAGE_SHIFT;

	current_page_base = page_aligned_start;
	offset = flash_addr & ( FLASH_PAGE_SIZE - 1 );
	source = ram_addr;
	
	for( i = 0; i < num_pages; i++ ) {
		copy_size = ( ( len_remaining + offset ) > FLASH_PAGE_SIZE ) ? ( FLASH_PAGE_SIZE - offset ) : len_remaining;
		// read the area we're going to overwrite
		memcpy( ( unsigned char * )flash_buffer, ( unsigned char * )current_page_base, FLASH_PAGE_SIZE );
		// overlay our changes
		memcpy( ( unsigned char * )( flash_buffer + offset ), ( unsigned char * )source, copy_size ); 
		flash_erase_sectors( current_page_base, FLASH_PAGE_SIZE );
		if( ret =  flash_block_write( current_page_base, ( unsigned int )flash_buffer, FLASH_PAGE_SIZE ) ) { // write it back
			return( ret );
		}
		// check page write
		for( i = 0; i < FLASH_PAGE_SIZE; i++ ) {
			if( ((unsigned char *)current_page_base)[i] != ((unsigned char *)flash_buffer)[i] ) {
				return( FWERR_SECTOR_COMPARE );
			}
		}

		current_page_base += FLASH_PAGE_SIZE;
		offset = 0;
		source = ram_addr + copy_size;
		len_remaining -= copy_size;
	}

	if( !ret ) { 
		for( i = 0; i < len; i++ ) {
			if( ((unsigned char *)flash_addr)[i] != ((unsigned char *)ram_addr)[i] ) {
				ret = FWERR_DATA_COMPARE;
				break;
			}
		}
	}

	return( ret );
}


int 
flash_erase_sectors(
	unsigned int start_addr,
	int len )
{
	int start_sector;
	int end_sector;
	unsigned int rc = 0;

	start_sector = flash_get_sector_number( start_addr );
	end_sector   = flash_get_sector_number( start_addr + len - 1 );

	if( ( start_sector != -1 ) && ( end_sector != -1 ) ) {
		rc = iap_cmd( IAP_CMD_PREPARE_SECTORS, start_sector, end_sector, 0, 0, 0 );
		if( rc == IAP_STAT_CMD_SUCCESS ) {
			while( 1 ) {
				rc =  iap_cmd(IAP_CMD_ERASE_SECTORS, 
						start_sector, end_sector, PRC_CLK, 0, 0 );
				if(rc != IAP_STAT_BUSY)
					break;
			}
		}
	}

	return 0;
}


