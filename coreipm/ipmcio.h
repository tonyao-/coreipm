/*
-------------------------------------------------------------------------------
coreIPM/ipmcio.h

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
// JTAG
// ----
#define JTAG_TRST		P1_31	// I	JTAG TRST		20	P1.31
#define JTAG_TMS		P1_30	// I	JTAG TMS		52	P1.30
#define JTAG_TCK		P1_29	// I	JTAG TCK		56	P1.29
#define JTAG_TDI		P1_28	// I	JTAG TDI		60	P1.28
#define JTAG_TDO		P1_27	// O	JTAG TDO		64	P1.27
#define JTAG_RTCK		P1_26	// I	JTAG RTCK		24	P1.26
#define JTAG_EXTIN0		P1_25	// I	JTAG EXTIN		28	P1.25
#define JTAG_TRACECLK		P1_24	// I	JTAG TRACECLK		32	P1.24

// INTERRUPT GENERATING INPUTS
// ---------------------------
#define EINT_HOT_SWAP_HANDLE	P0_16  // EINT0
#define EINT_SPI		P0_20  // EINT3

// SPI
// ---
#define SCK_0	P0_4	// 27
#define MISO_0	P0_5	// 29
#define MOSI_0	P0_6	// 30

// ADC/GPIO
// --------
#define ADC_CH_0  P0_28 // 13
#define ADC_CH_1  P0_29 // 14
#define ADC_CH_2  P0_30 // 15
#define ADC_CH_3  P0_25 //  9
#define ADC_CH_4  P0_10 // 35
#define ADC_CH_5  P0_15 // 45
#define ADC_CH_6  P0_21 //  1
#define ADC_CH_7  P0_22 //  2

// Alternate uses for ADC_CH_4 - 7 pins
#define SPI_SELECT_0	P0_10
#define SPI_SELECT_1	P0_15
#define SPI_SELECT_2	P0_21
#define SPI_SELECT_3	P0_22



/* Backplane addressing GPIO IN */
#define	BP_0	P1_16  // 16
#define	BP_1	P1_17  // 12
#define	BP_2	P1_18  //  8
#define	BP_3	P1_19  //  4
#define	BP_4	P1_20  // 48
#define	BP_5	P1_21  // 44
#define	BP_6	P1_22  // 40
#define	BP_7	P1_23  // 36

#define UART_0_TX	P0_0	// O	RS232 Transmit		19	P0.0
#define UART_0_RX	P0_1	// I	RS232 Receive		21	P0.1


#define I2C_0_SCL	P0_2	// I	I2C clock		22	P0.2
#define I2C_0_SDA	P0_3	// I/O	I2C data		26	P0.3


#define I2C_1_SCL	P0_11	// I	I2C clock		37	P0.2
#define I2C_1_SDA	P0_14	// I/O	I2C data		41	P0.3

#define UART_1_TX	P0_8	// O	RS232 Transmit		33	P0.8
#define UART_1_RX	P0_9	// I	RS232 Receive		34	P0.9

// General IO
#define	GPIO_0		P0_18  // 53
#define	GPIO_1		P0_23  // 58
#define GPIO_2		P0_12  // 38
#define GPIO_3		P0_13  // 39
#define LED_1		P0_17  // 47
#define BLUE_LED	P0_31  // 17
#define LED_0		BLUE_LED
#define GPIO_LED_0	LED_0
#define GPIO_LED_1	LED_1

// TACH-PWM / GPIO
#define TACH_IN_0	P0_19  // 54
#define PWM_OUT_0	P0_7   // 31

#define PAYLOAD_POWER P4_29
