/*
 * $QNXLicenseC:
 * Copyright 2007, QNX Software Systems. All Rights Reserved.
 * 
 * You must obtain a written license from and pay applicable license fees to QNX 
 * Software Systems before you may reproduce, modify or distribute this software, 
 * or any work that includes all or part of this software.   Free development 
 * licenses are available for evaluation and non-commercial purposes.  For more 
 * information visit http://licensing.qnx.com or email licensing@qnx.com.
 *  
 * This file may contain contributions from others.  Please review this entire 
 * file for other proprietary rights or license notices, as well as the QNX 
 * Development Suite License Guide at http://licensing.qnx.com/license-guide/ 
 * for other information.
 * $
 */



#ifndef __MIPS_AU1000CPU_H_INCLUDED
#define __MIPS_AU1000CPU_H_INCLUDED
/*
 *  mips/au1000cpu.h
 *

 */

#ifdef __ASM__

	#include <mips/cpu.h>

#else

	#ifndef __PLATFORM_H_INCLUDED
	#include <sys/platform.h>
	#endif

	#include _NTO_HDR_(mips/cpu.h)

#endif

/*
** System Control 
*/
#define AU1000_SYS_BASE		0x11900000

#define AU1000_SYS_FREQCTRL0	0x0020
#define AU1000_SYS_FREQCTRL1	0x0024
#define AU1000_SYS_CLKSRC		0x0028
#define AU1000_SYS_PINFUNC		0x002C
#define	AU1000_SYS_POWERCTRL	0x003C
#define AU1000_SYS_CPUPLL		0x0060
#define AU1000_SYS_AUXPLL		0x0064

#define	AU1000_SYS_BUS_DIV_MASK _BITFIELD32L(0,0x3)

/*
** Interrupt Control 0/1 Registers
*/
#define	AU1000_ICR0_BASE		0x10400000
#define	AU1000_ICR1_BASE		0x11800000

#define	AU1000_ICR_CFG0RD		0x0040 
#define	AU1000_ICR_CFG0SET		0x0040
#define	AU1000_ICR_CFG0CLR		0x0044
#define	AU1000_ICR_CFG1RD		0x0048 
#define	AU1000_ICR_CFG1SET		0x0048
#define	AU1000_ICR_CFG1CLR		0x004C
#define	AU1000_ICR_CFG2RD		0x0050 
#define	AU1000_ICR_CFG2SET		0x0050
#define	AU1000_ICR_CFG2CLR		0x0054
#define	AU1000_ICR_REQ0INT		0x0054
#define	AU1000_ICR_SRCRD		0x0058
#define	AU1000_ICR_SRCSET		0x0058
#define	AU1000_ICR_SRCCLR		0x005C
#define	AU1000_ICR_REQ1INT		0x005C
#define	AU1000_ICR_ASSIGNRD		0x0060
#define	AU1000_ICR_ASSIGNSET	0x0060
#define	AU1000_ICR_ASSIGNCLR	0x0064
#define	AU1000_ICR_WAKERD		0x0068
#define	AU1000_ICR_WAKESET		0x0068
#define	AU1000_ICR_WAKECLR		0x006C
#define	AU1000_ICR_MASKRD		0x0070
#define	AU1000_ICR_MASKSET		0x0070
#define	AU1000_ICR_MASKCLR		0x0074
#define	AU1000_ICR_RISINGRD		0x0078
#define	AU1000_ICR_RISINGCLR	0x0078
#define	AU1000_ICR_FALLINGRD	0x007C
#define	AU1000_ICR_FALLINGCLR	0x007C
#define	AU1000_ICR_TESTBIT		0x0080


/*
** Ethernet MAC Controller
*/

/* MAC Base Address */
#define	AU1000_MAC0_BASE	0x10500000
#define	AU1000_MAC1_BASE	0x10510000
/* MAC Registers (Offset from base register) */
#define AU1000_MAC_CONTROL		0x0000
#define AU1000_MAC_ADDRHIGH		0x0004
#define AU1000_MAC_ADDRLOW		0x0008
#define AU1000_MAC_HASHHIGH		0x000c
#define AU1000_MAC_HASHLOW		0x0010
#define AU1000_MAC_MIICTRL		0x0014
#define AU1000_MAC_MIIDATA		0x0018
#define AU1000_MAC_FLOWCTRL		0x001c
#define AU1000_MAC_VLAN1		0x0020
#define AU1000_MAC_VLAN2		0x0024

/* MAC Enable Base Address */
#define	AU1000_MAC_ENABLE_BASE		0x10520000
/* MAC Enable Registers */
#define AU1000_MAC_ENABLE_MAC0		(AU1000_MAC_ENABLE_BASE+0x0000)
#define AU1000_MAC_ENABLE_MAC1		(AU1000_MAC_ENABLE_BASE+0x0004)

/* MAC DMA Base Address */
#define	AU1000_MAC_DMA0_BASE		0x14004000
#define	AU1000_MAC_DMA1_BASE		0x14004200
/* MAC DMA Entry (Offset from base register) */
#define	AU1000_MAC_TX_DMA			0x0000
#define	AU1000_MAC_RX_DMA			0x0100
/* MAC DMA Transmit Entry Registers (Offset from TX entry base) */
#define	AU1000_MAC_TX_DMA_STATUS	0x0000
#define	AU1000_MAC_TX_DMA_ADDRESS	0x0004
#define	AU1000_MAC_TX_DMA_LENGTH	0x0008
/* MAC DMA Receive Entry Registers (Offset from RX entry base) */
#define	AU1000_MAC_RX_DMA_STATUS	0x0000
#define	AU1000_MAC_RX_DMA_ADDRESS	0x0004

/*
** Uart Interface
*/

#define	AU1000_UART0_BASE	0x11100000
#define	AU1000_UART1_BASE	0x11200000
#define	AU1000_UART2_BASE	0x11300000
#define	AU1000_UART3_BASE	0x11400000

#define	AU1000_UART_RXDATA		0x0000
#define	AU1000_UART_TXDATA		0x0004
#define	AU1000_UART_INTEN		0x0008
#define AU1000_UART_INTCAUSE	0x000C
#define	AU1000_UART_FIFOCTRL	0x0010
#define	AU1000_UART_LINECTRL	0x0014
#define	AU1000_UART_MDMCTRL		0x0018
#define AU1000_UART_LINESTAT	0x001C
#define AU1000_UART_MDMSTAT		0x0020
#define AU1000_UART_CLKDIV	 	0x0028
#define AU1000_UART_ENABLE		0x0100
#define AU1000_UART_SIZE		0x0104

#define	AU1000_UART_CE			_BITFIELD32L(0,0x1)
#define	AU1000_UART_E			_BITFIELD32L(0,0x2)

/* Bit definitions for interrupt identification (INTCAUSE) */
#define AU1000_IIR_NOINTR		0x01 /* No interrupt pending */
#define AU1000_IIR_MS			0x00 /* Modem Status */
#define AU1000_IIR_TX			0x02 /* Transmit Buffer */
#define AU1000_IIR_RX			0x04 /* Receive Buffer */
#define AU1000_IIR_LS			0x06 /* Line Status */
#define AU1000_IIR_CTO			0x0C /* Character Time Out */

/* Bit definitions for fifo control (FIFOCTRL) */
#define AU1000_FCR_FE			0x01 /* FIFO Enable */
#define AU1000_FCR_RR			0x02 /* Receiver Reset */
#define AU1000_FCR_TR			0x04 /* Transmitter Reset */
#define AU1000_FCR_MS			0x08 /* Mode Select */
#define AU1000_FCR_TFT_MASK		0x30 /* Transmit FIFO Threshold */
#define AU1000_FCR_RFT_MASK		0xC0 /* Receiver FIFO Threshold */

/* FIFO Trigger Depths (FCR_TFT_MASK/FCR_RFT_MASK) */
#define AU1000_FCR_TFT0			_BITFIELD32L(4,0x00) /* Transmit FIFO Depth 0 */
#define AU1000_FCR_TFT4			_BITFIELD32L(4,0x01) /* Transmit FIFO Depth 4 */
#define AU1000_FCR_TFT8			_BITFIELD32L(4,0x02) /* Transmit FIFO Depth 8 */
#define AU1000_FCR_TFT12		_BITFIELD32L(4,0x03) /* Transmit FIFO Depth 12 */
#define AU1000_FCR_RFT1			_BITFIELD32L(6,0x00) /* Transmit FIFO Depth 1 */
#define AU1000_FCR_RFT4			_BITFIELD32L(6,0x01) /* Transmit FIFO Depth 4 */
#define AU1000_FCR_RFT8			_BITFIELD32L(6,0x02) /* Transmit FIFO Depth 8 */
#define AU1000_FCR_RFT14		_BITFIELD32L(6,0x03) /* Transmit FIFO Depth 14 */

/* Bit definitions for line control (LINECTRL) */
#define AU1000_LCR_WLS_MASK		0x03 /* Word Length Select */
#define AU1000_LCR_ST			0x04 /* Stop Bits */
#define AU1000_LCR_PEN			0x08 /* Parity Enable */
#define AU1000_LCR_EPS			0x10 /* Parity Select */
#define AU1000_LCR_SPS			0x20 /* Stick Parity */
#define AU1000_LCR_SB			0x40 /* Send Break */

/* Bit definitions for modem control (MDMCTRL) */
#define AU1000_MCR_DT			0x01 /* DTR */
#define AU1000_MCR_RT			0x02 /* RTS */
#define AU1000_MCR_I0			0x04 /* OUT1 */
#define AU1000_MCR_I1			0x08 /* OUT2 */
#define AU1000_MCR_LP			0x10 /* Loopback */

/* Bit definitions for line status (LINESTAT) */
#define AU1000_LSR_DR			0x01 /* Data Ready */
#define AU1000_LSR_OE			0x02 /* Overrun Error */
#define AU1000_LSR_PE			0x04 /* Parrity Error */
#define AU1000_LSR_FE			0x08 /* Framming Error */
#define AU1000_LSR_BI			0x10 /* Break Indication */
#define AU1000_LSR_TT			0x20 /* Transmit Threshold */
#define AU1000_LSR_TE			0x40 /* Transmit Shift Register Empty */
#define AU1000_LSR_RF			0x80 /* Receiver FIFO Contains Error */

/* Bit definitions for modem status (MDMSTAT) */
#define AU1000_MSR_DC			0x01 /* Delta CTS */
#define AU1000_MSR_DR			0x02 /* Delta DSR */
#define AU1000_MSR_TRI			0x04 /* Terminate Ring Condition */
#define AU1000_MSR_DD			0x08 /* Delta DCD */
#define AU1000_MSR_CT			0x10 /* CTS */
#define AU1000_MSR_DS			0x20 /* DSR */
#define AU1000_MSR_RI			0x40 /* Ring Indication */
#define AU1000_MSR_CD			0x80 /* Data Carrier Detect */


/*
** IRDA Interface
*/

/* Irda base and size */
#define AU1000_IRDA_BASE 			0x10300000
#define AU1000_IRDA_SIZE 			0x40

/* Register Offsets */
#define AU1000_IRDA_RNGPTRSTAT 		0x00
#define AU1000_IRDA_RNGBSADRH 		0x04
#define AU1000_IRDA_RNGBSADRL 		0x08
#define AU1000_IRDA_RINGSIZE 		0x0C
#define AU1000_IRDA_RNGPROMPT 		0x10
#define AU1000_IRDA_RNGADRCMP 		0x14
#define AU1000_IRDA_INTCLEAR 		0x18
#define AU1000_IRDA_CONFIG1 		0x20
#define AU1000_IRDA_SIRFLAGS 		0x24
#define AU1000_IRDA_STATUSEN 		0x28
#define AU1000_IRDA_RDPHYCFG 		0x2C
#define AU1000_IRDA_WRPHYCFG 		0x30
#define AU1000_IRDA_MAXPKTLEN 		0x34
#define AU1000_IRDA_RXBYTECNT 		0x38
#define AU1000_IRDA_CONFIG2 		0x3C
#define AU1000_IRDA_ENABLE 			0x40

/* Bits Definitions for Enable Register */
#define AU1000_IRDA_ENABLE_E 		0x1
#define AU1000_IRDA_ENABLE_C 		0x2
#define AU1000_IRDA_ENABLE_CE 		0x4
#define AU1000_IRDA_ENABLE_HC 		0x8

/* Bits Definitions for STATUSEN Register */
#define AU1000_IRDA_STATUSEN_CS 	(1<<8)
#define AU1000_IRDA_STATUSEN_RS 	(1<<9)
#define AU1000_IRDA_STATUSEN_TS 	(1<<10)
#define AU1000_IRDA_STATUSEN_SV 	(1<<11)
#define AU1000_IRDA_STATUSEN_MV 	(1<<12)
#define AU1000_IRDA_STATUSEN_FV 	(1<<13)
#define AU1000_IRDA_STATUSEN_CE 	(1<<14)
#define AU1000_IRDA_STATUSEN_E 		(1<<15)

/* Bits Definitions for CONFIG1 Register */
#define AU1000_IRDA_CONFG1_RI 		(1<<0)
#define AU1000_IRDA_CONFG1_TI 		(1<<1)
#define AU1000_IRDA_CONFG1_ST 		(1<<2)
#define AU1000_IRDA_CONFG1_SF 		(1<<3)
#define AU1000_IRDA_CONFG1_SI 		(1<<4)
#define AU1000_IRDA_CONFG1_MI 		(1<<5)
#define AU1000_IRDA_CONFG1_FI 		(1<<6)
#define AU1000_IRDA_CONFG1_CM 		(1<<7)
#define AU1000_IRDA_CONFG1_TD 		(1<<8)
#define AU1000_IRDA_CONFG1_RA 		(1<<9)
#define AU1000_IRDA_CONFG1_ME 		(1<<10)
#define AU1000_IRDA_CONFG1_RE 		(1<<11)
#define AU1000_IRDA_CONFG1_TE 		(1<<12)
#define AU1000_IRDA_CONFG1_IL 		(1<<14)
#define AU1000_IRDA_SIR_MODE 		(AU1000_IRDA_CONFG1_SI | AU1000_IRDA_CONFG1_ME | AU1000_IRDA_CONFG1_RA | AU1000_IRDA_CONFG1_RE | AU1000_IRDA_CONFG1_SF | AU1000_IRDA_CONFG1_CM)

/* Bits Definitions for CONFIG2 Register */
#define AU1000_IRDA_CONFG2_MI  		(1<<0)
#define AU1000_IRDA_CONFG2_P   		(1<<1)
#define AU1000_IRDA_CONFG2_CS  		_BITFIELD32L(2,0x03)
#define AU1000_IRDA_CONFG2_DP  		(1<<4)
#define AU1000_IRDA_CONFG2_DA  		(1<<5)
#define AU1000_IRDA_CONFG2_FS  		_BITFIELD32L(6,0x03)
#define AU1000_IRDA_CONFG2_IE  		(1<<8)

/* Bits Definitions for WRPHYCFG Register */
#define AU1000_IRDA_BAUD_2400 		47
#define AU1000_IRDA_BAUD_9600 		11
#define AU1000_IRDA_BAUD_19200 		5
#define AU1000_IRDA_BAUD_38400 		2
#define AU1000_IRDA_BAUD_57600 		1
#define AU1000_IRDA_BAUD_115200 	0
#define AU1000_IRDA_PHY_PW12 		0x0180 


#define _RAU1000UNIT(unit,reg,size)	(*(volatile uint##size##_t *)((MIPS_R4K_K1BASE+AU1000_##unit##_BASE + AU1000_##reg)))
#define RAU1000UNIT(unit,reg)			_RAU1000UNIT(unit, reg, 32)

#endif /* __MIPS_AU1000CPU_H_INCLUDED */

/* __SRCVERSION("au1000cpu.h $Rev: 153052 $"); */
