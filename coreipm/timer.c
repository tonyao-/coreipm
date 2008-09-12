/*
-------------------------------------------------------------------------------
coreIPM/timer.c

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

#include "arch.h"
#include "timer.h"
#include "error.h"

#define CQ_ARRAY_SIZE 32

unsigned long lbolt;

typedef struct cqe_struct {
	unsigned state;
	unsigned long tick;
	void *handle;
	void ( *func )( unsigned char * );
	unsigned char *arg;
} CQE;

CQE	cq_array[CQ_ARRAY_SIZE];

/*==============================================================*/
/* Function Prototypes						*/
/*==============================================================*/

void timer_process_callout_queue( void );
void cq_init( void );
CQE *cq_alloc( void );
void cq_free( CQE *cqe );
CQE *cq_get_expired_elem( unsigned long current_tick );
void cq_set_cqe_state( CQE *cqe, unsigned state );
#if defined (__CA__) || defined (__CC_ARM)
void hardclock( void ) __irq;
#elif defined (__GNUC__)
void hardclock( void ) __attribute__ ((interrupt));
#endif

/*==============================================================
 * hardclock()
 *==============================================================*/
/* Timer Counter 0 Interrupt executes each 100ms */
#if defined (__CA__) || defined (__CC_ARM)
void hardclock( void ) __irq  
#elif defined (__GNUC__)
void hardclock( void )
#endif
{
	lbolt++;
	T0IR = 1;		/* Clear interrupt flag */
	VICVectAddr = 0;	/* Acknowledge Interrupt */
}

/*==============================================================
 * timer_initialize()
 *==============================================================*/
/* Setup the Timer Counter 0 Interrupt */
void
timer_initialize( void ) 
{
//	T0MR0 = 5999999;				/* 60MHz clk - 100mSec = 6,000,000-1 counts */
//	T0MR0 = 1199999;				/* 12MHz clk - 100mSec = 1,200,000-1 counts */
	T0MR0 = PCLK/10 - 1;
	T0MCR = 3;					/* Interrupt and Reset on MR0 */
	T0TCR = 1;					/* Timer0 Enable */
	VICVectAddr3 = (unsigned long)hardclock;	/* set interrupt vector in 3 */
	VICVectCntl3 = 0x20 | 4;			/* use it for Timer 0 Interrupt */
	VICIntEnable = IER_TIMER0;			/* enable Timer0 interrupt */

	cq_init();
}

/*==============================================================
 * timer_add_callout_queue()
 *==============================================================*/
int
timer_add_callout_queue( 
	void *handle,
	unsigned long ticks, 
	void(*func)(unsigned char *), 
	unsigned char *arg )
{
	CQE *cqe;

	cqe = cq_alloc();
	if( cqe ) {
		cqe->func = func;
		cqe->arg = arg;
		cqe->tick = ticks + lbolt;
		cqe->handle = handle;
		return( ESUCCESS );
	} else
		return( ENOMEM );
}

/*==============================================================
 * timer_remove_callout_queue()
 * 	Search the callout queue using the handle and remove the 
 * 	entry if found.
 *==============================================================*/
void
timer_remove_callout_queue(
	void *handle )
{
	CQE *ptr;
	unsigned i;
	unsigned int interrupt_mask = CURRENT_INTERRUPT_MASK;	

	DISABLE_INTERRUPTS;
	for ( i = 0; i < CQ_ARRAY_SIZE; i++ )
	{
		ptr = &cq_array[i];
		if( ( ptr->state == CQE_ACTIVE ) && ( ptr->handle == handle ) ) {
			cq_free( ptr );
			break;
		}
	}
	ENABLE_INTERRUPTS( interrupt_mask );
}

/*==============================================================
 * timer_reset_callout_queue() 
 * 	Search the callout queue using the handle and reset the
 * 	timeout value with the new value passed in ticks.
 *==============================================================*/
void
timer_reset_callout_queue(
	void *handle,
	unsigned long ticks )
{
	CQE *cqe;
	unsigned i;
	unsigned int interrupt_mask = CURRENT_INTERRUPT_MASK;	

	DISABLE_INTERRUPTS;	
	for ( i = 0; i < CQ_ARRAY_SIZE; i++ )
	{
		cqe = &cq_array[i];
		if( ( cqe->state == CQE_ACTIVE ) && ( cqe->handle == handle ) ) {
			cqe->tick = ticks + lbolt;
			break;
		}
	}
	ENABLE_INTERRUPTS( interrupt_mask );
}


/*==============================================================
 * timer_get_expiration_time() 
 * 	Get ticks to expiration.
 * 
 *==============================================================*/
unsigned long
timer_get_expiration_time(
	void *handle ) 
{
	CQE *cqe;
	unsigned i;
	unsigned long ticks = 0;
	
	for ( i = 0; i < CQ_ARRAY_SIZE; i++ )
	{
		cqe = &cq_array[i];
		if( ( cqe->state == CQE_ACTIVE ) && ( cqe->handle == handle ) ) {
			if( cqe->tick > lbolt )
				return( cqe->tick - lbolt );
			break;
		}
	}
	return( 0 );
}
		

/*==============================================================
 * timer_process_callout_queue()
 * 	Get first expired entry, invoke it's callback and remove
 * 	from queue.
 *==============================================================*/
void
timer_process_callout_queue( void )
{
	CQE *cqe;
	
	if( ( cqe = cq_get_expired_elem( lbolt ) ) ) {
		cq_set_cqe_state( cqe, CQE_PENDING );
		(*cqe->func)( cqe->arg );
		cq_free( cqe );
	}
}

/*======================================================================*
 * CALLOUT QUEUE MANAGEMENT
 */

/*==============================================================
 * cq_init()
 *==============================================================*/
/* initialize cq structures */
void 
cq_init( void )
{
	unsigned i;
	
	for ( i = 0; i < CQ_ARRAY_SIZE; i++ )
	{
		cq_array[i].state = CQE_FREE;
	}

}

/*==============================================================
 * cq_alloc()
 *==============================================================*/
/* get a free callout queue entry (CQE) */
CQE *
cq_alloc( void )
{
	CQE *cqe = 0;
	CQE *ptr = cq_array;
	unsigned i;
	unsigned int interrupt_mask = CURRENT_INTERRUPT_MASK;	

	DISABLE_INTERRUPTS;
	for ( i = 0; i < CQ_ARRAY_SIZE; i++ )
	{
		ptr = &cq_array[i];
		if( ptr->state == CQE_FREE ) {
			ptr->tick = 0;
			ptr->state = CQE_ACTIVE;
			cqe = ptr;
			break;
		}
	}
	ENABLE_INTERRUPTS( interrupt_mask );
	return cqe;
}

/*==============================================================
 * cq_free()
 *==============================================================*/
/* set cqe state to free */
void 
cq_free( CQE *cqe )
{
	cqe->state = CQE_FREE;
}

/*==============================================================
 * cq_get_expired_elem()
 *==============================================================*/
CQE *
cq_get_expired_elem( unsigned long current_tick )
{
	CQE *cqe = 0;
	CQE *ptr = cq_array;
	unsigned i;
	unsigned int interrupt_mask = CURRENT_INTERRUPT_MASK;	

	DISABLE_INTERRUPTS;
	for ( i = 0; i < CQ_ARRAY_SIZE; i++ )
	{
		ptr = &cq_array[i];
		if( ( ptr->state == CQE_ACTIVE ) && ( ptr->tick )
				&& ( current_tick >= ptr->tick ) ) {
			cqe = ptr;
			break;
		}
	}
	ENABLE_INTERRUPTS( interrupt_mask );
	return cqe;
}

/*==============================================================
 * cq_set_cqe_state()
 *==============================================================*/
void
cq_set_cqe_state( CQE *cqe, unsigned state )
{
	cqe->state = state;
}

