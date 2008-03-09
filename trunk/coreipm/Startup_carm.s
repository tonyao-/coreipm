
/*
-------------------------------------------------------------------------------
coreIPM/Startup_carm.s

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

        PROCESSOR_MODE_USER        EQU      0x10
        PROCESSOR_MODE_FIQ         EQU      0x11
        PROCESSOR_MODE_IRQ         EQU      0x12
        PROCESSOR_MODE_SUPERVISOR  EQU      0x13
        PROCESSOR_MODE_ABORT       EQU      0x17
        PROCESSOR_MODE_UNDEFINED   EQU      0x1B
        PROCESSOR_MODE_SYSTEM      EQU      0x1F

        STACK_SIZE_USR         EQU     0x00000400
        STACK_SIZE_FIQ         EQU     0x00000004
        STACK_SIZE_IRQ         EQU     0x00000100
        STACK_SIZE_SUPERVISOR  EQU     0x00000004
        STACK_SIZE_ABORT       EQU     0x00000004
        STACK_SIZE_UNDEFINED   EQU     0x00000004

AREA   STACK, DATA, READWRITE, ALIGN = 2
        DS   ( STACK_SIZE_USR + 3 ) & ~3 
        DS   ( STACK_SIZE_SUPERVISOR + 3 ) & ~3
        DS   ( STACK_SIZE_IRQ + 3 ) & ~3
        DS   ( STACK_SIZE_FIQ + 3 ) & ~3
        DS   ( STACK_SIZE_ABORT + 3 ) & ~3
        DS   ( STACK_SIZE_UNDEFINED + 3 ) & ~3
Top_Stack:

        PLL_BASE        EQU     0xE01FC080  /* PLL Base Address */
        PLLCON_OFS      EQU     0x00        /* PLL Control Offset*/
        PLLCFG_OFS      EQU     0x04        /* PLL Configuration Offset */
        PLLSTAT_OFS     EQU     0x08        /* PLL Status Offset */
        PLLFEED_OFS     EQU     0x0C        /* PLL Feed Offset */
        PLLCON_PLLE     EQU     ( 1<<0 )    /* PLL Enable */
        PLLCON_PLLC     EQU     ( 1<<1 )    /* PLL Connect */
        PLLCFG_MSEL     EQU     ( 0x1F<<0 ) /* PLL Multiplier */
        PLLCFG_PSEL     EQU     ( 0x03<<5 ) /* PLL Divider */
        PLLSTAT_PLOCK   EQU     ( 1<<10 )   /* PLL Lock Status */

        PLLCFG_Val      EQU     0x00000024

        MAM_BASE        EQU     0xE01FC000  /* MAM Base Address */
        MAMCR_OFS       EQU     0x00        /* MAM Control Offset*/
        MAMTIM_OFS      EQU     0x04        /* MAM Timing Offset */


        MAMCR_Val       EQU     0x00000002
        MAMTIM_Val      EQU     0x00000004

        CODE_BASE       EQU     0x00000000

AREA   STARTUPCODE, CODE, AT CODE_BASE 
       PUBLIC  __startup

       EXTERN  CODE32 (?C?INIT)

__startup       PROC    CODE32

EXTERN CODE32 (Undef_Handler?A)
EXTERN CODE32 (SWI_Handler?A)
EXTERN CODE32 (PAbt_Handler?A)
EXTERN CODE32 (DAbt_Handler?A)
EXTERN CODE32 (IRQ_Handler?A)
EXTERN CODE32 (FIQ_Handler?A)


Vectors:        LDR     PC,Reset_Addr         
                LDR     PC,Undef_Addr
                LDR     PC,SWI_Addr
                LDR     PC,PAbt_Addr
                LDR     PC,DAbt_Addr
                NOP                      
                LDR     PC,[PC, #-0x0FF0] 
                LDR     PC,FIQ_Addr

Reset_Addr:     DD      Reset_Handler
Undef_Addr:     DD      Undef_Handler?A
SWI_Addr:       DD      SWI_Handler?A
PAbt_Addr:      DD      PAbt_Handler?A
DAbt_Addr:      DD      DAbt_Handler?A
                DD      0               
IRQ_Addr:       DD      IRQ_Handler?A
FIQ_Addr:       DD      FIQ_Handler?A


Reset_Handler:  
                LDR     R0, =PLL_BASE
                MOV     R1, #0xAA
                MOV     R2, #0x55

	/* PLL configuration */
                MOV     R3, #PLLCFG_Val
                STR     R3, [R0, #PLLCFG_OFS] 
                MOV     R3, #PLLCON_PLLE
                STR     R3, [R0, #PLLCON_OFS]
                STR     R1, [R0, #PLLFEED_OFS]
                STR     R2, [R0, #PLLFEED_OFS]
pll_loop:       LDR     R3, [R0, #PLLSTAT_OFS]
                ANDS    R3, R3, #PLLSTAT_PLOCK
                BEQ     pll_loop
                MOV     R3, #(PLLCON_PLLE | PLLCON_PLLC)
                STR     R3, [R0, #PLLCON_OFS]
                STR     R1, [R0, #PLLFEED_OFS]
                STR     R2, [R0, #PLLFEED_OFS]

                LDR     R0, =MAM_BASE
                MOV     R1, #MAMTIM_Val
                STR     R1, [R0, #MAMTIM_OFS] 
                MOV     R1, #MAMCR_Val
                STR     R1, [R0, #MAMCR_OFS] 

	/* Enter each mode and set up it's stack pointer */
                LDR     R0, =Top_Stack
                MSR     CPSR_c, #PROCESSOR_MODE_UNDEFINED|0x80|0x40
                MOV     SP, R0
                SUB     R0, R0, #STACK_SIZE_UNDEFINED
                MSR     CPSR_c, #PROCESSOR_MODE_ABORT|0x80|0x40
                MOV     SP, R0
                SUB     R0, R0, #STACK_SIZE_ABORT
                MSR     CPSR_c, #PROCESSOR_MODE_FIQ|0x80|0x40
                MOV     SP, R0
                SUB     R0, R0, #STACK_SIZE_FIQ
                MSR     CPSR_c, #PROCESSOR_MODE_IRQ|0x80|0x40
                MOV     SP, R0
                SUB     R0, R0, #STACK_SIZE_IRQ
                MSR     CPSR_c, #PROCESSOR_MODE_SUPERVISOR|0x80|0x40
                MOV     SP, R0
                SUB     R0, R0, #STACK_SIZE_SUPERVISOR
                MSR     CPSR_c, #PROCESSOR_MODE_USER
                MOV     SP, R0

	/* enter C code */
                LDR     R0,=?C?INIT
                TST     R0,#1       // if bit 0 is set we're running in Thumb mode
                LDREQ   LR,=exit?A  // ARM Mode
                LDRNE   LR,=exit?T  // Thumb Mode
                BX      R0
                ENDP

PUBLIC exit?A
exit?A          PROC    CODE32
                B       exit?A
                ENDP

PUBLIC exit?T
exit?T          PROC    CODE16
exit:           B       exit?T
                ENDP


                END
