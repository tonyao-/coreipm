
#include <stdio.h>
#include <rt_misc.h>

// #pragma import(__use_no_semihosting_swi)


extern int  sendchar(int ch);  /* in serial.c */


struct __FILE { 
	int handle;
};

FILE __stdout;


int
fputc( 	int ch, 
	FILE *f ) 
{
	return( sendchar( ch ) );
}


int 
ferror( FILE *f ) 
{
	return EOF;
}


void 
_ttywrch( int ch ) 
{
	sendchar(ch);
}


void 
_sys_exit( int return_code ) 
{
	label:  goto label;  /* endless loop */
}
