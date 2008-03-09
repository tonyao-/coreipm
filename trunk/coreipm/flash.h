
/* Flash write error  codes */
#define FWERR_NOERR		0
#define FWERR_IAP		-1
#define FWERR_SECTOR_NUM	-2
#define FWERR_SECTOR_COMPARE	-3
#define FWERR_DATA_COMPARE	-4

/* LPC21xx Part Identification numbers */

#define PART_ID_LPC2104 0xFFF0FF12
#define PART_ID_LPC2105 0xFFF0FF22
#define PART_ID_LPC2106 0xFFF0FF32

#define PART_ID_LPC2114 0x0101FF12
#define PART_ID_LPC2119 0x0201FF12
#define PART_ID_LPC2124 0x0101FF13
#define PART_ID_LPC2129 0x0201FF13

#define PART_ID_LPC2131 0x0002FF01
#define PART_ID_LPC2132 0x0002FF11
#define PART_ID_LPC2134 0x0002FF12
#define PART_ID_LPC2136 0x0002FF23
#define PART_ID_LPC2138 0x0002FF25

#define PART_ID_LPC2141 0x0402FF01
#define PART_ID_LPC2142 0x0402FF11
#define PART_ID_LPC2144 0x0402FF12
#define PART_ID_LPC2146 0x0402FF23
#define PART_ID_LPC2148 0x0402FF25

#define PART_ID_LPC2194 0x0301FF13
int flash_erase_sectors( unsigned int start_addr, int len );
int flash_write( unsigned int flash_addr, unsigned int ram_addr, int len );
int flash_block_write( unsigned int flash_addr, unsigned int ram_addr, int len );
int flash_read_part_id( void );
