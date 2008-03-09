/*
-------------------------------------------------------------------------------
coreIPM/lpc21nn.h

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
#ifndef lpc210x_h
#define lpc210x_h

#define REG32 (volatile unsigned int*)

#define VICIRQStatus    (*(REG32 (0xFFFFF000)))
#define VICFIQStatus    (*(REG32 (0xFFFFF004)))
#define VICRawIntr      (*(REG32 (0xFFFFF008)))
#define VICIntSelect    (*(REG32 (0xFFFFF00C)))
#define VICIntEnable    (*(REG32 (0xFFFFF010)))
#define VICIntEnClear   (*(REG32 (0xFFFFF014)))
#define VICIntEnClr     VICIntEnClear
#define VICSoftInt      (*(REG32 (0xFFFFF018)))
#define VICSoftIntClear (*(REG32 (0xFFFFF01C)))
#define VICSoftIntClr   VICSoftIntClear
#define VICProtection   (*(REG32 (0xFFFFF020)))
#define VICVectAddr     (*(REG32 (0xFFFFF030)))
#define VICDefVectAddr  (*(REG32 (0xFFFFF034)))

#define VICVectAddr0    (*(REG32 (0xFFFFF100)))
#define VICVectAddr1    (*(REG32 (0xFFFFF104)))
#define VICVectAddr2    (*(REG32 (0xFFFFF108)))
#define VICVectAddr3    (*(REG32 (0xFFFFF10C)))
#define VICVectAddr4    (*(REG32 (0xFFFFF110)))
#define VICVectAddr5    (*(REG32 (0xFFFFF114)))
#define VICVectAddr6    (*(REG32 (0xFFFFF118)))
#define VICVectAddr7    (*(REG32 (0xFFFFF11C)))
#define VICVectAddr8    (*(REG32 (0xFFFFF120)))
#define VICVectAddr9    (*(REG32 (0xFFFFF124)))
#define VICVectAddr10   (*(REG32 (0xFFFFF128)))
#define VICVectAddr11   (*(REG32 (0xFFFFF12C)))
#define VICVectAddr12   (*(REG32 (0xFFFFF130)))
#define VICVectAddr13   (*(REG32 (0xFFFFF134)))
#define VICVectAddr14   (*(REG32 (0xFFFFF138)))
#define VICVectAddr15   (*(REG32 (0xFFFFF13C)))

#define VICVectCntl0    (*(REG32 (0xFFFFF200)))
#define VICVectCntl1    (*(REG32 (0xFFFFF204)))
#define VICVectCntl2    (*(REG32 (0xFFFFF208)))
#define VICVectCntl3    (*(REG32 (0xFFFFF20C)))
#define VICVectCntl4    (*(REG32 (0xFFFFF210)))
#define VICVectCntl5    (*(REG32 (0xFFFFF214)))
#define VICVectCntl6    (*(REG32 (0xFFFFF218)))
#define VICVectCntl7    (*(REG32 (0xFFFFF21C)))
#define VICVectCntl8    (*(REG32 (0xFFFFF220)))
#define VICVectCntl9    (*(REG32 (0xFFFFF224)))
#define VICVectCntl10   (*(REG32 (0xFFFFF228)))
#define VICVectCntl11   (*(REG32 (0xFFFFF22C)))
#define VICVectCntl12   (*(REG32 (0xFFFFF230)))
#define VICVectCntl13   (*(REG32 (0xFFFFF234)))
#define VICVectCntl14   (*(REG32 (0xFFFFF238)))
#define VICVectCntl15   (*(REG32 (0xFFFFF23C)))

// VIC test registers
#define VICITCR         (*(REG32 (0xFFFFF300)))
#define VICITIP1        (*(REG32 (0xFFFFF304)))
#define VICITIP2        (*(REG32 (0xFFFFF308)))
#define VICITOP1        (*(REG32 (0xFFFFF30C)))
#define VICITOP2        (*(REG32 (0xFFFFF310)))

// VIC peripheral identification registers
#define VICPeriphID0    (*(REG32 (0xFFFFFFE0)))
#define VICPeriphID1    (*(REG32 (0xFFFFFFE4)))
#define VICPeriphID2    (*(REG32 (0xFFFFFFE8)))
#define VICPeriphID3    (*(REG32 (0xFFFFFFEC)))

/*##############################################################################
## PCB - Pin Connect Block
##############################################################################*/

#define PCB_PINSEL0     (*(REG32 (0xE002C000)))
#define PCB_PINSEL1     (*(REG32 (0xE002C004)))
#define PCB_PINSEL2     (*(REG32 (0xE002C014)))

#define PINSEL0         PCB_PINSEL0
#define PINSEL1         PCB_PINSEL1
#define PINSEL2         PCB_PINSEL2


/*##############################################################################
## GPIO - General Purpose I/O
##############################################################################*/

#define GPIO_IOPIN      (*(REG32 (0xE0028000))) /* ALTERNATE NAME GPIO = GPIO0 */
#define GPIO_IOSET      (*(REG32 (0xE0028004)))
#define GPIO_IODIR      (*(REG32 (0xE0028008)))
#define GPIO_IOCLR      (*(REG32 (0xE002800C)))

#define GPIO0_IOPIN     (*(REG32 (0xE0028000))) /* ALTERNATE NAME GPIO = GPIO0 */
#define GPIO0_IOSET     (*(REG32 (0xE0028004)))
#define GPIO0_IODIR     (*(REG32 (0xE0028008)))
#define GPIO0_IOCLR     (*(REG32 (0xE002800C)))

#define IOPIN0          (*(REG32 (0xE0028000)))
#define IOSET0          (*(REG32 (0xE0028004)))
#define IODIR0          (*(REG32 (0xE0028008)))
#define IOCLR0          (*(REG32 (0xE002800C)))
#define IOPIN1          (*(REG32 (0xE0028010)))
#define IOSET1          (*(REG32 (0xE0028014)))
#define IODIR1          (*(REG32 (0xE0028018)))
#define IOCLR1          (*(REG32 (0xE002801C)))
#define IO0PIN          (*(REG32 (0xE0028000)))
#define IO0SET          (*(REG32 (0xE0028004)))
#define IO0DIR          (*(REG32 (0xE0028008)))
#define IO0CLR          (*(REG32 (0xE002800C)))
#define IO1PIN          (*(REG32 (0xE0028010)))
#define IO1SET          (*(REG32 (0xE0028014)))
#define IO1DIR          (*(REG32 (0xE0028018)))
#define IO1CLR          (*(REG32 (0xE002801C)))
#define FIO0DIR         (*(REG32 (0x3FFFC000)))
#define FIO0MASK        (*(REG32 (0x3FFFC010)))
#define FIO0PIN         (*(REG32 (0x3FFFC014)))
#define FIO0SET         (*(REG32 (0x3FFFC018)))
#define FIO0CLR         (*(REG32 (0x3FFFC01C)))
#define FIO1DIR         (*(REG32 (0x3FFFC020)))
#define FIO1MASK        (*(REG32 (0x3FFFC030)))
#define FIO1PIN         (*(REG32 (0x3FFFC034)))
#define FIO1SET         (*(REG32 (0x3FFFC038)))
#define FIO1CLR         (*(REG32 (0x3FFFC03C)))

// Memory Acceleration Module (MAM) Registers
#define MAMCR           (*(REG32 (0xE01FC000)))
#define MAMTIM          (*(REG32 (0xE01FC004)))
#define MEMMAP          (*(REG32 (0xE01FC040)))

/*##############################################################################
## UART0 / UART1
##############################################################################*/

/* ---- UART 0 --------------------------------------------- */
#define UART0_RBR       (*(REG32 (0xE000C000)))
#define UART0_THR       (*(REG32 (0xE000C000)))
#define UART0_IER       (*(REG32 (0xE000C004)))
#define UART0_IIR       (*(REG32 (0xE000C008)))
#define UART0_FCR       (*(REG32 (0xE000C008)))
#define UART0_LCR       (*(REG32 (0xE000C00C)))
#define UART0_MCR       (*(REG32 (0xE000C010)))
#define UART0_LSR       (*(REG32 (0xE000C014)))
#define UART0_MSR       (*(REG32 (0xE000C018)))
#define UART0_SCR       (*(REG32 (0xE000C01C)))
#define UART0_DLL       (*(REG32 (0xE000C000)))
#define UART0_DLM       (*(REG32 (0xE000C004)))
#define UART0_ACR       (*(REG32 (0xE000C020)))
#define UART0_FDR       (*(REG32 (0xE000C028)))
#define UART0_TER       (*(REG32 (0xE000C030)))

/* ---- UART 1 --------------------------------------------- */
#define UART1_RBR       (*(REG32 (0xE0010000)))
#define UART1_THR       (*(REG32 (0xE0010000)))
#define UART1_IER       (*(REG32 (0xE0010004)))
#define UART1_IIR       (*(REG32 (0xE0010008)))
#define UART1_FCR       (*(REG32 (0xE0010008)))
#define UART1_LCR       (*(REG32 (0xE001000C)))
#define UART1_MCR       (*(REG32 (0xE0010010)))
#define UART1_LSR       (*(REG32 (0xE0010014)))
#define UART1_MSR       (*(REG32 (0xE0010018)))
#define UART1_SCR       (*(REG32 (0xE001001C)))
#define UART1_DLL       (*(REG32 (0xE0010000)))
#define UART1_DLM       (*(REG32 (0xE0010004)))
#define UART1_ACR       (*(REG32 (0xE0010020)))
#define UART1_FDR       (*(REG32 (0xE0010028)))
#define UART1_TER       (*(REG32 (0xE0010030)))


/* Universal Asynchronous Receiver Transmitter 0 (UART0) */
#define U0RBR           UART0_RBR
#define U0THR           UART0_THR
#define U0IER           UART0_IER
#define U0IIR           UART0_IIR
#define U0FCR           UART0_FCR
#define U0LCR           UART0_LCR
#define U0MCR           UART0_MCR
#define U0LSR           UART0_LSR
#define U0MSR           UART0_MSR
#define U0SCR           UART0_SCR
#define U0DLL           UART0_DLL
#define U0DLM           UART0_DLM
#define U0ACR           UART0_ACR
#define U0FDR           UART0_FDR
#define U0TER           UART0_TER

/* Universal Asynchronous Receiver Transmitter 1 (UART1) */
#define U1RBR           UART1_RBR
#define U1THR           UART1_THR
#define U1IER           UART1_IER
#define U1IIR           UART1_IIR
#define U1FCR           UART1_FCR
#define U1LCR           UART1_LCR
#define U1MCR           UART1_MCR
#define U1LSR           UART1_LSR
#define U1MSR           UART1_MSR
#define U1SCR           UART1_SCR
#define U1DLL           UART1_DLL
#define U1DLM           UART1_DLM
#define U1ACR           UART1_ACR
#define U1FDR           UART1_FDR
#define U1TER           UART1_TER

/*##############################################################################
## I2C
##############################################################################*/

#define I2C_I2CONSET    (*(REG32 (0xE001C000)))
#define I2C_I2STAT      (*(REG32 (0xE001C004)))
#define I2C_I2DAT       (*(REG32 (0xE001C008)))
#define I2C_I2ADR       (*(REG32 (0xE001C00C)))
#define I2C_I2SCLH      (*(REG32 (0xE001C010)))
#define I2C_I2SCLL      (*(REG32 (0xE001C014)))
#define I2C_I2CONCLR    (*(REG32 (0xE001C018)))

#define I2C0_I2CONSET    I2C_I2CONSET
#define I2C0_I2STAT      I2C_I2STAT
#define I2C0_I2DAT       I2C_I2DAT
#define I2C0_I2ADR       I2C_I2ADR
#define I2C0_I2SCLH      I2C_I2SCLH
#define I2C0_I2SCLL      I2C_I2SCLL
#define I2C0_I2CONCLR    I2C_I2CONCLR

#define I2C1_I2CONSET    (*(REG32 (0xE005C000)))
#define I2C1_I2STAT      (*(REG32 (0xE005C004)))
#define I2C1_I2DAT       (*(REG32 (0xE005C008)))
#define I2C1_I2ADR       (*(REG32 (0xE005C00C)))
#define I2C1_I2SCLH      (*(REG32 (0xE005C010)))
#define I2C1_I2SCLL      (*(REG32 (0xE005C014)))
#define I2C1_I2CONCLR    (*(REG32 (0xE005C018)))

/* I2C Interface 0 */
#define I2C0CONSET      I2C_I2CONSET
#define I2C0STAT        I2C_I2STAT
#define I2C0DAT         I2C_I2DAT
#define I2C0ADR         I2C_I2ADR
#define I2C0SCLH        I2C_I2SCLH
#define I2C0SCLL        I2C_I2SCLL
#define I2C0CONCLR      I2C_I2CONCLR

/* I2C Interface 1 */
#define I2C1CONSET      I2C1_I2CONSET
#define I2C1STAT        I2C1_I2STAT
#define I2C1DAT         I2C1_I2DAT
#define I2C1ADR         I2C1_I2ADR
#define I2C1SCLH        I2C1_I2SCLH
#define I2C1SCLL        I2C1_I2SCLL
#define I2C1CONCLR      I2C1_I2CONCLR


/*##############################################################################
## SPI - Serial Peripheral Interface
##############################################################################*/

/* SPI0 (Serial Peripheral Interface 0) */
#define SPI_SPCR        (*(REG32 (0xE0020000)))
#define SPI_SPSR        (*(REG32 (0xE0020004)))
#define SPI_SPDR        (*(REG32 (0xE0020008)))
#define SPI_SPCCR       (*(REG32 (0xE002000C)))
#define SPI_SPTCR       (*(REG32 (0xE0020010)))
#define SPI_SPTSR       (*(REG32 (0xE0020014)))
#define SPI_SPTOR       (*(REG32 (0xE0020018)))
#define SPI_SPINT       (*(REG32 (0xE002001C)))

#define S0SPCR          SPI_SPCR	// SPI0 Control Register
#define S0SPSR          SPI_SPSR	// SPI0 Status Register
#define S0SPDR          SPI_SPDR	// SPI Data Register
#define S0SPCCR         SPI_SPCCR	// SPI Clock Counter Register
#define S0SPTCR         SPI_SPTCR
#define S0SPTSR         SPI_SPTSR
#define S0SPTOR         SPI_SPTOR
#define S0SPINT         SPI_SPINT	// SPI Interrupt Flag

/* SSP Controller (SPI1) */
#define SSPCR0          (*(REG32 (0xE0068000)))	// Control Register 0
#define SSPCR1          (*(REG32 (0xE0068004)))	// Control Register 1
#define SSPDR           (*(REG32 (0xE0068008)))	// Data Register
#define SSPSR           (*(REG32 (0xE006800C)))	// Status Register
#define SSPCPSR         (*(REG32 (0xE0068010)))	// Clock Prescale Register
#define SSPIMSC         (*(REG32 (0xE0068014)))	// Interrupt Mask Set and Clear Register
#define SSPRIS          (*(REG32 (0xE0068018)))	// Raw Interrupt Status Register
#define SSPMIS          (*(REG32 (0xE006801C)))	// Masked Interrupt Status Register
#define SSPICR          (*(REG32 (0xE0068020)))	// SSPICR Interrupt Clear Register

/*##############################################################################
## Timer 0 and Timer 1
##############################################################################*/

/* ---- Timer 0 -------------------------------------------- */
#define T0_IR           (*(REG32 (0xE0004000)))
#define T0_TCR          (*(REG32 (0xE0004004)))
#define T0_TC           (*(REG32 (0xE0004008)))
#define T0_PR           (*(REG32 (0xE000400C)))
#define T0_PC           (*(REG32 (0xE0004010)))
#define T0_MCR          (*(REG32 (0xE0004014)))
#define T0_MR0          (*(REG32 (0xE0004018)))
#define T0_MR1          (*(REG32 (0xE000401C)))
#define T0_MR2          (*(REG32 (0xE0004020)))
#define T0_MR3          (*(REG32 (0xE0004024)))
#define T0_CCR          (*(REG32 (0xE0004028)))
#define T0_CR0          (*(REG32 (0xE000402C)))
#define T0_CR1          (*(REG32 (0xE0004030)))
#define T0_CR2          (*(REG32 (0xE0004034)))
#define T0_CR3          (*(REG32 (0xE0004038)))
#define T0_EMR          (*(REG32 (0xE000403C)))
#define T0_CTCR         (*(REG32 (0xE0004070)))

/* ---- Timer 1 -------------------------------------------- */
#define T1_IR           (*(REG32 (0xE0008000)))
#define T1_TCR          (*(REG32 (0xE0008004)))
#define T1_TC           (*(REG32 (0xE0008008)))
#define T1_PR           (*(REG32 (0xE000800C)))
#define T1_PC           (*(REG32 (0xE0008010)))
#define T1_MCR          (*(REG32 (0xE0008014)))
#define T1_MR0          (*(REG32 (0xE0008018)))
#define T1_MR1          (*(REG32 (0xE000801C)))
#define T1_MR2          (*(REG32 (0xE0008020)))
#define T1_MR3          (*(REG32 (0xE0008024)))
#define T1_CCR          (*(REG32 (0xE0008028)))
#define T1_CR0          (*(REG32 (0xE000802C)))
#define T1_CR1          (*(REG32 (0xE0008030)))
#define T1_CR2          (*(REG32 (0xE0008034)))
#define T1_CR3          (*(REG32 (0xE0008038)))
#define T1_EMR          (*(REG32 (0xE000803C)))
#define T1_CTCR         (*(REG32 (0xE0008070)))

/* Timer 0 */
#define T0IR            T0_IR
#define T0TCR           T0_TCR
#define T0TC            T0_TC
#define T0PR            T0_PR
#define T0PC            T0_PC
#define T0MCR           T0_MCR
#define T0MR0           T0_MR0
#define T0MR1           T0_MR1
#define T0MR2           T0_MR2
#define T0MR3           T0_MR3
#define T0CCR           T0_CCR
#define T0CR0           T0_CR0
#define T0CR1           T0_CR1
#define T0CR2           T0_CR2
#define T0CR3           T0_CR3
#define T0EMR           T0_EMR
#define T0CTCR          T0_CTCR

/* Timer 1 */
#define T1IR            T1_IR
#define T1TCR           T1_TCR
#define T1TC            T1_TC
#define T1PR            T1_PR
#define T1PC            T1_PC
#define T1MCR           T1_MCR
#define T1MR0           T1_MR0
#define T1MR1           T1_MR1
#define T1MR2           T1_MR2
#define T1MR3           T1_MR3
#define T1CCR           T1_CCR
#define T1CR0           T1_CR0
#define T1CR1           T1_CR1
#define T1CR2           T1_CR2
#define T1CR3           T1_CR3
#define T1EMR           T1_EMR
#define T1CTCR          T1_CTCR


/*##############################################################################
## PWM
##############################################################################*/

#define PWM_IR          (*(REG32 (0xE0014000)))
#define PWM_TCR         (*(REG32 (0xE0014004)))
#define PWM_TC          (*(REG32 (0xE0014008)))
#define PWM_PR          (*(REG32 (0xE001400C)))
#define PWM_PC          (*(REG32 (0xE0014010)))
#define PWM_MCR         (*(REG32 (0xE0014014)))
#define PWM_MR0         (*(REG32 (0xE0014018)))
#define PWM_MR1         (*(REG32 (0xE001401C)))
#define PWM_MR2         (*(REG32 (0xE0014020)))
#define PWM_MR3         (*(REG32 (0xE0014024)))
#define PWM_MR4         (*(REG32 (0xE0014040)))
#define PWM_MR5         (*(REG32 (0xE0014044)))
#define PWM_MR6         (*(REG32 (0xE0014048)))
#define PWM_EMR         (*(REG32 (0xE001403C)))
#define PWM_PCR         (*(REG32 (0xE001404C)))
#define PWM_LER         (*(REG32 (0xE0014050)))

#define PWM_CCR         (*(REG32 (0xE0014028)))
#define PWM_CR0         (*(REG32 (0xE001402C)))
#define PWM_CR1         (*(REG32 (0xE0014030)))
#define PWM_CR2         (*(REG32 (0xE0014034)))
#define PWM_CR3         (*(REG32 (0xE0014038)))

#define PWMIR           PWM_IR
#define PWMTCR          PWM_TCR
#define PWMTC           PWM_TC
#define PWMPR           PWM_PR
#define PWMPC           PWM_PC
#define PWMMCR          PWM_MCR
#define PWMMR0          PWM_MR0
#define PWMMR1          PWM_MR1
#define PWMMR2          PWM_MR2
#define PWMMR3          PWM_MR3
#define PWMMR4          PWM_MR4
#define PWMMR5          PWM_MR5
#define PWMMR6          PWM_MR6
#define PWMPCR          PWM_PCR
#define PWMLER          PWM_LER

/*##############################################################################
## RTC
##############################################################################*/

/* ---- RTC: Miscellaneous Register Group ------------------ */
#define RTC_ILR         (*(REG32 (0xE0024000)))
#define RTC_CTC         (*(REG32 (0xE0024004)))
#define RTC_CCR         (*(REG32 (0xE0024008)))  
#define RTC_CIIR        (*(REG32 (0xE002400C)))
#define RTC_AMR         (*(REG32 (0xE0024010)))
#define RTC_CTIME0      (*(REG32 (0xE0024014)))
#define RTC_CTIME1      (*(REG32 (0xE0024018)))
#define RTC_CTIME2      (*(REG32 (0xE002401C)))

/* ---- RTC: Timer Control Group --------------------------- */
#define RTC_SEC         (*(REG32 (0xE0024020)))
#define RTC_MIN         (*(REG32 (0xE0024024)))
#define RTC_HOUR        (*(REG32 (0xE0024028)))
#define RTC_DOM         (*(REG32 (0xE002402C)))
#define RTC_DOW         (*(REG32 (0xE0024030)))
#define RTC_DOY         (*(REG32 (0xE0024034)))
#define RTC_MONTH       (*(REG32 (0xE0024038)))
#define RTC_YEAR        (*(REG32 (0xE002403C)))

/* ---- RTC: Alarm Control Group --------------------------- */
#define RTC_ALSEC       (*(REG32 (0xE0024060)))
#define RTC_ALMIN       (*(REG32 (0xE0024064)))
#define RTC_ALHOUR      (*(REG32 (0xE0024068)))
#define RTC_ALDOM       (*(REG32 (0xE002406C)))
#define RTC_ALDOW       (*(REG32 (0xE0024070)))
#define RTC_ALDOY       (*(REG32 (0xE0024074)))
#define RTC_ALMON       (*(REG32 (0xE0024078)))
#define RTC_ALYEAR      (*(REG32 (0xE002407C)))

/* ---- RTC: Reference Clock Divider Group ----------------- */
#define RTC_PREINT      (*(REG32 (0xE0024080)))
#define RTC_PREFRAC     (*(REG32 (0xE0024084)))

/* Real Time Clock */
#define ILR             RTC_ILR
#define CTC             RTC_CTC
#define CCR             RTC_CCR
#define CIIR            RTC_CIIR
#define AMR             RTC_AMR
#define CTIME0          RTC_CTIME0
#define CTIME1          RTC_CTIME1
#define CTIME2          RTC_CTIME2

#define SEC             RTC_SEC 
#define MIN             RTC_MIN
#define HOUR            RTC_HOUR
#define DOM             RTC_DOM
#define DOW             RTC_DOW
#define DOY             RTC_DOY
#define MONTH           RTC_MONTH
#define YEAR            RTC_YEAR

#define ALSEC           RTC_ALSEC
#define ALMIN           RTC_ALMIN
#define ALHOUR          RTC_ALHOUR
#define ALDOM           RTC_ALDOM
#define ALDOW           RTC_ALDOW
#define ALDOY           RTC_ALDOY
#define ALMON           RTC_ALMON
#define ALYEAR          RTC_ALYEAR

#define PREINT          RTC_PREINT
#define PREFRAC         RTC_PREFRAC

/* A/D Converter 0 (AD0) */
#define AD0CR           (*(REG32 (0xE0034000)))
#define AD0GDR          (*(REG32 (0xE0034004)))
#define AD0STAT         (*(REG32 (0xE0034030)))
#define AD0INTEN        (*(REG32 (0xE003400C)))
#define AD0DR0          (*(REG32 (0xE0034010)))
#define AD0DR1          (*(REG32 (0xE0034014)))
#define AD0DR2          (*(REG32 (0xE0034018)))
#define AD0DR3          (*(REG32 (0xE003401C)))
#define AD0DR4          (*(REG32 (0xE0034020)))
#define AD0DR5          (*(REG32 (0xE0034024)))
#define AD0DR6          (*(REG32 (0xE0034028)))
#define AD0DR7          (*(REG32 (0xE003402C)))

/* A/D Converter 1 (AD1) */
#define AD1CR           (*(REG32 (0xE0060000)))
#define AD1GDR          (*(REG32 (0xE0060004)))
#define AD1STAT         (*(REG32 (0xE0060030)))
#define AD1INTEN        (*(REG32 (0xE006000C)))
#define AD1DR0          (*(REG32 (0xE0060010)))
#define AD1DR1          (*(REG32 (0xE0060014)))
#define AD1DR2          (*(REG32 (0xE0060018)))
#define AD1DR3          (*(REG32 (0xE006001C)))
#define AD1DR4          (*(REG32 (0xE0060020)))
#define AD1DR5          (*(REG32 (0xE0060024)))
#define AD1DR6          (*(REG32 (0xE0060028)))
#define AD1DR7          (*(REG32 (0xE006002C)))

/* A/D Converter Global */
#define ADGSR           (*(REG32 (0xE0034008)))

/* D/A Converter */
#define DACR            (*(REG32 (0xE006C000)))

/*##############################################################################
## WD - Watchdog
##############################################################################*/

#define WD_WDMOD        (*(REG32 (0xE0000000)))
#define WD_WDTC         (*(REG32 (0xE0000004)))
#define WD_WDFEED       (*(REG32 (0xE0000008)))
#define WD_WDTV         (*(REG32 (0xE000000C)))

#define WDMOD           WD_WDMOD
#define WDTC            WD_WDTC
#define WDFEED          WD_WDFEED
#define WDTV            WD_WDTV

/* USB Controller */
#define USBIntSt        (*(REG32 (0xE01FC1C0)))
#define USBDevIntSt     (*(REG32 (0xE0090000)))
#define USBDevIntEn     (*(REG32 (0xE0090004)))
#define USBDevIntClr    (*(REG32 (0xE0090008)))
#define USBDevIntSet    (*(REG32 (0xE009000C)))
#define USBDevIntPri    (*(REG32 (0xE009002C)))
#define USBEpIntSt      (*(REG32 (0xE0090030)))
#define USBEpIntEn      (*(REG32 (0xE0090034)))
#define USBEpIntClr     (*(REG32 (0xE0090038)))
#define USBEpIntSet     (*(REG32 (0xE009003C)))
#define USBEpIntPri     (*(REG32 (0xE0090040)))
#define USBReEp         (*(REG32 (0xE0090044)))
#define USBEpInd        (*(REG32 (0xE0090048)))
#define USBMaxPSize     (*(REG32 (0xE009004C)))
#define USBRxData       (*(REG32 (0xE0090018)))
#define USBRxPLen       (*(REG32 (0xE0090020)))
#define USBTxData       (*(REG32 (0xE009001C)))
#define USBTxPLen       (*(REG32 (0xE0090024)))
#define USBCtrl         (*(REG32 (0xE0090028)))
#define USBCmdCode      (*(REG32 (0xE0090010)))
#define USBCmdData      (*(REG32 (0xE0090014)))
#define USBDMARSt       (*(REG32 (0xE0090050)))
#define USBDMARClr      (*(REG32 (0xE0090054)))
#define USBDMARSet      (*(REG32 (0xE0090058)))
#define USBUDCAH        (*(REG32 (0xE0090080)))
#define USBEpDMASt      (*(REG32 (0xE0090084)))
#define USBEpDMAEn      (*(REG32 (0xE0090088)))
#define USBEpDMADis     (*(REG32 (0xE009008C)))
#define USBDMAIntSt     (*(REG32 (0xE0090090)))
#define USBDMAIntEn     (*(REG32 (0xE0090094)))
#define USBEoTIntSt     (*(REG32 (0xE00900A0)))
#define USBEoTIntClr    (*(REG32 (0xE00900A4)))
#define USBEoTIntSet    (*(REG32 (0xE00900A8)))
#define USBNDDRIntSt    (*(REG32 (0xE00900AC)))
#define USBNDDRIntClr   (*(REG32 (0xE00900B0)))
#define USBNDDRIntSet   (*(REG32 (0xE00900B4)))
#define USBSysErrIntSt  (*(REG32 (0xE00900B8)))
#define USBSysErrIntClr (*(REG32 (0xE00900BC)))
#define USBSysErrIntSet (*(REG32 (0xE00900C0)))

/*##############################################################################
## System Control Block
##############################################################################*/

#define SCB_EXTINT      (*(REG32 (0xE01FC140)))
#define SCB_EXTWAKE     (*(REG32 (0xE01FC144)))
#define SCB_MEMMAP      (*(REG32 (0xE01FC040)))
#define SCB_PLLCON      (*(REG32 (0xE01FC080)))
#define SCB_PLLCFG      (*(REG32 (0xE01FC084)))
#define SCB_PLLSTAT     (*(REG32 (0xE01FC088)))
#define SCB_PLLFEED     (*(REG32 (0xE01FC08C)))
#define SCB_PCON        (*(REG32 (0xE01FC0C0)))
#define SCB_PCONP       (*(REG32 (0xE01FC0C4)))
#define SCB_VPBDIV      (*(REG32 (0xE01FC100)))

/* Phase Locked Loop 0 (PLL0) */
#define PLL0CON         (*(REG32 (0xE01FC080)))
#define PLL0CFG         (*(REG32 (0xE01FC084)))
#define PLL0STAT        (*(REG32 (0xE01FC088)))
#define PLL0FEED        (*(REG32 (0xE01FC08C)))

/* Phase Locked Loop 1 (PLL1) */
#define PLL1CON         (*(REG32 (0xE01FC0A0)))
#define PLL1CFG         (*(REG32 (0xE01FC0A4)))
#define PLL1STAT        (*(REG32 (0xE01FC0A8)))
#define PLL1FEED        (*(REG32 (0xE01FC0AC)))

#define VPBDIV          SCB_VPBDIV

#define PCON            SCB_PCON
#define PCONP           SCB_PCONP

#define EXTINT          SCB_EXTINT
#define INTWAKE         SCB_EXTWAKE
#define EXTMODE         (*(REG32 (0xE01FC148)))
#define EXTPOLAR        (*(REG32 (0xE01FC14C)))

/* Reset */
#define RSID            (*(REG32 (0xE01FC180)))

/* Code Security / Debugging */
#define CSPR            (*(REG32 (0xE01FC184)))

/* System Control Miscellaneous */
#define SCS             (*(REG32 (0xE01FC1A0)))
#endif /* lpc210x_h */


/* PINSEL0 register setting flags */

#define PS0_P0_0_GPIO		0x00000000
#define PS0_P0_0_TXD_UART_0	0x00000001
#define PS0_P0_0_PWM_1		0x00000002

#define PS0_P0_1_GPIO		0x00000000
#define PS0_P0_1_RXD_UART_0	0x00000004
#define PS0_P0_1_PWM_3		0x00000008
#define PS0_P0_1_EINT_0		0x0000000c

#define PS0_P0_2_GPIO		0x00000000
#define PS0_P0_2_SCL_I2C_0	0x00000010
#define PS0_P0_2_CAPTURE_00	0x00000020

#define PS0_P0_3_GPIO		0x00000000
#define PS0_P0_3_SDA_I2C_0	0x00000040
#define PS0_P0_3_MATCH_0_0	0x00000080
#define PS0_P0_3_EINT_1		0x000000c0

#define PS0_P0_4_GPIO		0x00000000
#define PS0_P0_4_SCK_0		0x00000100
#define PS0_P0_4_CAPTURE_0_1	0x00000200
#define PS0_P0_4_AD_06		0x00000300

#define PS0_P0_5_GPIO		0x00000000
#define PS0_P0_5_MISO_0		0x00000400
#define PS0_P0_5_MATCH_0_1	0x00000800
#define PS0_P0_5_AD_0_7		0x00000c00

#define PS0_P0_6_GPIO		0x00000000
#define PS0_P0_6_MOSI_0		0x00001000
#define PS0_P0_6_CAPTURE_0_2	0x00002000
#define PS0_P0_6_AD_1_0		0x00003000

#define PS0_P0_7_GPIO		0x00000000
#define PS0_P0_7_SSEL_0		0x00004000
#define PS0_P0_7_PWM_2		0x00008000
#define PS0_P0_7_EINT_2		0x0000c000

#define PS0_P0_8_GPIO		0x00000000
#define PS0_P0_8_TDX_UART_1	0x00010000
#define PS0_P0_8_PWM_4		0x00020000
#define PS0_P0_8_AD_1_1		0x00030000

#define PS0_P0_9_GPIO		0x00000000
#define PS0_P0_9_RDX_UART_1	0x00040000
#define PS0_P0_9_PWM_6		0x00080000
#define PS0_P0_9_EINT_3		0x000c0000

#define PS0_P0_10_GPIO		0x00000000
#define PS0_P0_10_RTS_UART_1	0x00100000
#define PS0_P0_10_CAPTURE_1_0	0x00200000
#define PS0_P0_10_AD_1_2	0x00300000

#define PS0_P0_11_GPIO		0x00000000
#define PS0_P0_11_CTS_UART_1	0x00400000
#define PS0_P0_11_CAPTURE_1_1	0x00800000
#define PS0_P0_11_SCL_I2C_1	0x00c00000

#define PS0_P0_12_GPIO		0x00000000
#define PS0_P0_12_DSR_UART_1	0x01000000
#define PS0_P0_12_MATCH_1_0	0x02000000
#define PS0_P0_12_AD_1_3	0x03000000

#define PS0_P0_13_GPIO		0x00000000
#define PS0_P0_13_DTR_UART_1	0x04000000
#define PS0_P0_13_MATCH_1_1	0x08000000
#define PS0_P0_13_AD_1_4	0x0c000000

#define PS0_P0_14_GPIO		0x00000000
#define PS0_P0_14_DCD_UART_1	0x10000000
#define PS0_P0_14_EINT_1	0x20000000
#define PS0_P0_14_SDA_I2C_1	0x30000000

#define PS0_P0_15_GPIO		0x00000000
#define PS0_P0_15_RI_UART_1	0x40000000
#define PS0_P0_15_EINT_2	0x80000000
#define PS0_P0_15_AD_1_5	0xc0000000


/* PINSEL1 register setting flags */

#define PS1_P0_16_GPIO		0x00000000
#define PS1_P0_16_EINT_0	0x00000001
#define PS1_P0_16_MATCH_0_2	0x00000002
#define PS1_P0_16_CAPTURE_0_2	0x00000003

#define PS1_P0_17_GPIO		0x00000000
#define PS1_P0_17_CAPTURE_1_2	0x00000004
#define PS1_P0_17_SCK_1		0x00000008
#define PS1_P0_17_MATCH_1_2	0x0000000c

#define PS1_P0_18_GPIO		0x00000000
#define PS1_P0_18_CAPTURE_1_3	0x00000010
#define PS1_P0_18_MISO_1	0x00000020
#define PS1_P0_18_MATCH_1_3	0x00000030

#define PS1_P0_19_GPIO		0x00000000
#define PS1_P0_19_MATCH_1_2	0x00000040
#define PS1_P0_19_MOSI_1	0x00000080
#define PS1_P0_19_CAPTURE_1_2	0x000000c0

#define PS1_P0_20_GPIO		0x00000000
#define PS1_P0_20_MATCH_1_3	0x00000100
#define PS1_P0_20_SSEL_1	0x00000200
#define PS1_P0_20_EINT_3	0x00000300

#define PS1_P0_21_GPIO		0x00000000
#define PS1_P0_21_PWM_5		0x00000400
#define PS1_P0_21_AD_1_6	0x00000800
#define PS1_P0_21_CAPTURE_1_3	0x00000c00

#define PS1_P0_22_GPIO		0x00000000
#define PS1_P0_22_AD_1_7	0x00001000
#define PS1_P0_22_CAPTURE_0_0	0x00002000
#define PS1_P0_22_MATCH_0_0	0x00003000

#define PS1_P0_23_GPIO		0x00000000
#define PS1_P0_23_V_BUS		0x00004000

#define PS1_P0_25_GPIO		0x00000000
#define PS1_P0_25_AD_0_4	0x00040000
#define PS1_P0_25_DAC_A_OUT 	0x00080000

#define PS1_P0_28_GPIO		0x00000000
#define PS1_P0_28_AD_0_1	0x01000000
#define PS1_P0_28_CAPTURE_0_2	0x02000000
#define PS1_P0_28_MATCH_0_2	0x03000000

#define PS1_P0_29_GPIO		0x00000000
#define PS1_P0_29_AD_0_2	0x04000000
#define PS1_P0_29_CAPTURE_0_3	0x08000000
#define PS1_P0_29_MATCH_0_3	0x0c000000

#define PS1_P0_30_GPIO		0x00000000
#define PS1_P0_30_AD_0_3	0x10000000
#define PS1_P0_30_EINT_3	0x20000000
#define PS1_P0_30_CAPTURE_0_0	0x30000000

#define PS1_P0_31_GPIO		0x00000000
#define PS1_P0_31_UP_LED	0x40000000
#define PS1_P0_31_CONNECT	0x80000000
#define PS1_P0_31_AD_1_5	0xc0000000

/* PINSEL2 register setting flags */
#define PS2_P1_26_36_GPIO	0x00000000
#define PS2_P1_26_36_DEBUG	0x00000004
#define PS2_P1_16_25_GPIO	0x00000000
#define PS2_P1_16_25_TRACE	0x00000008



#define P0_0	0x00000001	// P0.0
#define P0_1	0x00000002	// P0.1
#define P0_2	0x00000004	// P0.2
#define P0_3	0x00000008	// P0.3
#define P0_4	0x00000010	// P0.4
#define P0_5	0x00000020	// P0.5
#define P0_6	0x00000040	// P0.6
#define P0_7	0x00000080	// P0.7
#define P0_8	0x00000100	// P0.8
#define P0_9	0x00000200	// P0.9
#define P0_10	0x00000400	// P0.10
#define P0_11	0x00000800	// P0.11
#define P0_12	0x00001000	// P0.12
#define P0_13	0x00002000	// P0.13
#define P0_14	0x00004000	// P0.14
#define P0_15	0x00008000	// P0.15
#define P0_16	0x00010000	// P0.16
#define P0_17	0x00020000	// P0.17
#define P0_18	0x00040000	// P0.18
#define P0_19	0x00080000	// P0.19
#define P0_20	0x00100000	// P0.20
#define P0_21	0x00200000	// P0.21
#define P0_22	0x00400000	// P0.22
#define P0_23	0x00800000	// P0.23
#define P0_24	0x01000000	// P0.24
#define P0_25	0x02000000	// P0.25
#define P0_26	0x04000000	// P0.26
#define P0_27	0x08000000	// P0.27
#define P0_28	0x10000000	// P0.28
#define P0_29	0x20000000	// P0.29
#define P0_30	0x40000000	// P0.30
#define P0_31	0x80000000	// P0.31

#define P1_0	0x0000000100000000	// P1.0
#define P1_1	0x0000000200000000	// P1.1
#define P1_2	0x0000000400000000	// P1.2
#define P1_3	0x0000000800000000	// P1.3
#define P1_4	0x0000001000000000	// P1.4
#define P1_5	0x0000002000000000	// P1.5
#define P1_6	0x0000004000000000	// P1.6
#define P1_7	0x0000008000000000	// P1.7
#define P1_8	0x0000010000000000	// P1.8
#define P1_9	0x0000020000000000	// P1.9
#define P1_10	0x0000040000000000	// P1.10
#define P1_11	0x0000080000000000	// P1.11
#define P1_12	0x0000100000000000	// P1.12
#define P1_13	0x0000200000000000	// P1.13
#define P1_14	0x0000400000000000	// P1.14
#define P1_15	0x0000800000000000	// P1.15
#define P1_16	0x0001000000000000	// P1.16
#define P1_17	0x0002000000000000	// P1.17
#define P1_18	0x0004000000000000	// P1.18
#define P1_19	0x0008000000000000	// P1.19
#define P1_20	0x0010000000000000	// P1.20
#define P1_21	0x0020000000000000	// P1.21
#define P1_22	0x0040000000000000	// P1.22
#define P1_23	0x0080000000000000	// P1.23
#define P1_24	0x0100000000000000	// P1.24
#define P1_25	0x0200000000000000	// P1.25
#define P1_26	0x0400000000000000	// P1.26
#define P1_27	0x0800000000000000	// P1.27
#define P1_28	0x1000000000000000	// P1.28
#define P1_29	0x2000000000000000	// P1.29
#define P1_30	0x4000000000000000	// P1.30
#define P1_31	0x8000000000000000	// P1.31





