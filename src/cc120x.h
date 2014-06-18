/*
 * @file cc120x.h - register map and definitions for cc1201 and cc1200
 *
 * Copyright (c) 2014 Alexey Anisimov <zolkko@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with program.  If not, see <http://www.gnu.org/licenses/>
 */

#ifndef __cc120x_h__
#define __cc120x_h__


#ifdef __cplusplus
extern "C" {
#endif

#define CCx_RW_BIT_bm						(0x80)
#define CCx_BURST_BIT_bm					(0x40)

/*
 * Configuration registers
 */
#define CCx_REG_BEGIN						(0x00)
#define CCx_IOCFG2							(0x00)	/* GDO2 output pin configuration */
#define CCx_IOCFG1							(0x01)	/* GDO1 output pin configuration */
#define CCx_IOCFG0							(0x02)	/* GDO0 output pin configuration */
#define CCx_FIFOTHR							(0x03)	/* RX FIFO and TX FIFO thresholds */
#define CCx_SYNC1							(0x04)	/* Sync word, high unsigned char */
#define CCx_SYNC0							(0x05)	/* Sync word, low unsigned char */
#define CCx_PKTLEN							(0x06)	/* Packet length */
#define CCx_PKTCTRL1						(0x07)	/* Packet automation control */
#define CCx_PKTCTRL0						(0x08)	/* Packet automation control */
#define CCx_ADDR							(0x09)	/* Device address */
#define CCx_CHANNR							(0x0A)	/* Channel number */
#define CCx_FSCTRL1							(0x0B)	/* Frequency synthesizer control */
#define CCx_FSCTRL0							(0x0C)	/* Frequency synthesizer control */
#define CCx_FREQ2							(0x0D)	/* Frequency control word, high unsigned char */
#define CCx_FREQ1							(0x0E)	/* Frequency control word, middle unsigned char */
#define CCx_FREQ0							(0x0F)	/* Frequency control word, low unsigned char */
#define CCx_MDMCFG4							(0x10)	/* Modem configuration */
#define CCx_MDMCFG3							(0x11)	/* Modem configuration */
#define CCx_MDMCFG2							(0x12)	/* Modem configuration */
#define CCx_MDMCFG1							(0x13)	/* Modem configuration */
#define CCx_MDMCFG0							(0x14)	/* Modem configuration */
#define CCx_DEVIATN							(0x15)	/* Modem deviation setting */
#define CCx_MCSM2							(0x16)	/* Main Radio Control State Machine configuration */
#define CCx_MCSM1							(0x17)	/* Main Radio Control State Machine configuration */
#define CCx_MCSM0							(0x18)	/* Main Radio Control State Machine configuration */
#define CCx_FOCCFG							(0x19)	/* Frequency Offset Compensation configuration */
#define CCx_BSCFG							(0x1A)	/* Bit Synchronization configuration */
#define CCx_AGCCTRL2						(0x1B)	/* AGC control */
#define CCx_AGCCTRL1						(0x1C)	/* AGC control */
#define CCx_AGCCTRL0						(0x1D)	/* AGC control */
#define CCx_WOREVT1							(0x1E)	/* High unsigned char Event 0 timeout */
#define CCx_WOREVT0							(0x1F)	/* Low unsigned char Event 0 timeout */
#define CCx_WORCTRL							(0x20)	/* Wake On Radio control */
#define CCx_FREND1							(0x21)	/* Front end RX configuration */
#define CCx_FREND0							(0x22)	/* Front end TX configuration */
#define CCx_FSCAL3							(0x23)	/* Frequency synthesizer calibration */
#define CCx_FSCAL2							(0x24)	/* Frequency synthesizer calibration */
#define CCx_FSCAL1							(0x25)	/* Frequency synthesizer calibration */
#define CCx_FSCAL0							(0x26)	/* Frequency synthesizer calibration */
#define CCx_RCCTRL1							(0x27)	/* RC oscillator configuration */
#define CCx_RCCTRL0							(0x28)	/* RC oscillator configuration */
#define CCx_FSTEST							(0x29)	/* Frequency synthesizer calibration control */
#define CCx_PTEST							(0x2A)	/* Production test */
#define CCx_AGCTEST							(0x2B)	/* AGC test */
#define CCx_TEST2							(0x2C)	/* Various test settings */
#define CCx_TEST1							(0x2D)	/* Various test settings */
#define CCx_TEST0							(0x2E)	/* Various test settings */

/*
 * @defgroup CCx_EXT_REG Extended register space for CC120x
 *
 * In order to access these registers the MCU has to
 * send [RW_BIT | BURST_BIT | 0x2f] [extender address] [data] sequence.
 *
 * @{
 */
/* Extended register base address */
#define CCx_EXTENDED_REGISTER		(0x2f)

#define CCx_IF_MIX_CFG				(0x00)
#define CCx_FREQOFF_CFG				(0x01)
#define CCx_TOC_CFG					(0x02)
#define CCx_MARC_SPARE				(0x03)
#define CCx_ECG_CFG					(0x04)
#define CCx_MDMCFG2					(0x05)
#define CCx_EXT_CTRL				(0x06)
#define CCx_RCCAL_FINE				(0x07)
#define CCx_RCCAL_COARSE			(0x08)
#define CCx_RCCAL_OFFSET			(0x09)
#define CCx_FREQOFF1				(0x0a)
#define CCx_FREQOFF0				(0x0b)
#define CCx_FREQ2					(0x0c)
#define CCx_FREQ1					(0x0d)
#define CCx_FREQ0					(0x0e)
#define CCx_IF_ADC2					(0x0f)
#define CCx_IF_ADC1					(0x10)
#define CCx_IF_ADC0					(0x11)
#define CCx_FS_DIG1					(0x12)
#define CCx_FS_DIG0					(0x13)
#define CCx_FS_CAL3					(0x14)
#define CCx_FS_CAL2					(0x15)
#define CCx_FS_CAL1					(0x16)
#define CCx_FS_CAL0					(0x17)
#define CCx_FS_CHP					(0x18)
#define CCx_FS_DIVTWO				(0x19)
#define CCx_FS_DSM1					(0x1a)
#define CCx_FS_DSM0					(0x1b)
#define CCx_FS_DVC1					(0x1c)
#define CCx_FS_DVC0					(0x1d)
#define CCx_FS_LBI					(0x1e)
#define CCx_FS_PFD					(0x1f)
#define CCx_FS_PRE					(0x20)
#define CCx_FS_REG_DIV_CML			(0x21)
#define CCx_FS_SPARE				(0x22)
#define CCx_FS_VCO4					(0x23)
#define CCx_FS_VCO3					(0x24)
#define CCx_FS_VCO2					(0x25)
#define CCx_FS_VCO1					(0x26)
#define CCx_FS_VCO0					(0x27)
#define CCx_GBIAS6					(0x28)
#define CCx_GBIAS5					(0x29)
#define CCx_GBIAS4					(0x2a)
#define CCx_GBIAS3					(0x2b)
#define CCx_GBIAS2					(0x2c)
#define CCx_GBIAS1					(0x2d)
#define CCx_GBIAS0					(0x2e)
#define CCx_IFAMP					(0x2f)
#define CCx_LNA						(0x30)
#define CCx_RXMIX					(0x31)
#define CCx_XOSC5					(0x32)
#define CCx_XOSC4					(0x33)
#define CCx_XOSC3					(0x34)
#define CCx_XOSC2					(0x35)
#define CCx_XOSC1					(0x36)
#define CCx_XOSC0					(0x37)
#define CCx_ANALOG_SPARE			(0x38)
#define CCx_PA_CFG3					(0x39)

#define CCx_WOR_TIME1				(0x64)
#define CCx_WOR_TIME0				(0x65)
#define CCx_WOR_CAPTURE1			(0x66)
#define CCx_WOR_CAPTURE0			(0x67)
#define CCx_BIST					(0x68)
#define CCx_DCFILTOFFSET_I1			(0x69)
#define CCx_DCFILTOFFSET_I0			(0x6a)
#define CCx_DCFILTOFFSET_Q1			(0x6b)
#define CCx_DCFILTOFFSET_Q0			(0x6c)
#define CCx_IQIE_I1					(0x6d)
#define CCx_IQIE_I0					(0x6e)
#define CCx_IQIE_Q1					(0x6f)
#define CCx_IQIE_Q0					(0x70)
#define CCx_RSSI1					(0x71)
#define CCx_RSSI0					(0x72)
#define CCx_MARCSTATE				(0x73)
#define CCx_LQI_VAL					(0x74)
#define CCx_PQT_SYNC_ERR			(0x75)
#define CCx_DEM_STATUS				(0x76)
#define CCx_FREQOFF_EST1			(0x77)
#define CCx_FREQOFF_EST0			(0x78)
#define CCx_AGC_GAIN3				(0x79)
#define CCx_AGC_GAIN2				(0x7a)
#define CCx_AGC_GAIN1				(0x7b)
#define CCx_AGC_GAIN0				(0x7c)
#define CCx_CFM_RX_DATA_OUT			(0x7d)
#define CCx_CFM_TX_DATA_IN			(0x7e)
#define CCx_ASK_SOFT_RX_DATA		(0x7f)
#define CCx_RNDGEN					(0x80)
#define CCx_MAGN2					(0x81)
#define CCx_MAGN1					(0x82)
#define CCx_MAGN0					(0x83)
#define CCx_ANG1					(0x84)
#define CCx_ANG0					(0x85)
#define CCx_CHFILT_I2				(0x86)
#define CCx_CHFILT_I1				(0x87)
#define CCx_CHFILT_I0				(0x88)
#define CCx_CHFILT_Q2				(0x89)
#define CCx_CHFILT_Q1				(0x8a)
#define CCx_CHFILT_Q0				(0x8b)
#define CCx_GPIO_STATUS				(0x8c)
#define CCx_FSCAL_CTRL				(0x8d)
#define CCx_PHASE_ADJUST			(0x8e)
#define CCx_PARTNUMBER				(0x8f)
#define CCx_PARTVERSION				(0x90)
#define CCx_SERIAL_STATUS			(0x91)
#define CCx_MODEM_STATUS1			(0x92)
#define CCx_MODEM_STATUS0			(0x93)
#define CCx_MARC_STATUS1			(0x94)
#define CCx_MARC_STATUS0			(0x95)
#define CCx_PA_IFAMP_TEST			(0x96)
#define CCx_FSRF_TEST				(0x97)
#define CCx_PRE_TEST				(0x98)
#define CCx_PRE_OVR					(0x99)
#define CCx_ADC_TEST				(0x9a)
#define CCx_DVC_TEST				(0x9b)
#define CCx_ATEST					(0x9c)
#define CCx_ATEST_LVDS				(0x9d)
#define CCx_ATEST_MODE				(0x9e)
#define CCx_XOSC_TEST1				(0x9f)
#define CCx_XOSC_TEST0				(0xa0)
#define CCx_AES						(0xa1)
#define CCx_MDM_TEST				(0xa2)
#define CCx_RXFIRST					(0xd2)
#define CCx_TXFIRST					(0xd3)
#define CCx_RXLAST					(0xd4)
#define CCx_TXLAST					(0xd5)
#define CCx_NUM_TXBYTES				(0xd6)
#define CCx_NYN_RXBYTES				(0xd7)
#define CCx_FIFO_NUM_TXBYTES		(0xd8)
#define CCx_FIFO_NUM_RXBYTES		(0xd9)
#define CCx_RXFIFO_PRE_BUF			(0xda)
#define CCx_AES_START				(0xe0)
#define CCx_AES_END					(0xff)
/* @} */

/*
 * @defgroup CHIP_ID IC Known values of CCx_PARTNUMBER register
 *
 * @{
 */
#define CCx_CHIP_CC1200						(0x20)
#define CCx_CHIP_CC1201						(0x21)
/* @} */

/*
 * @defgroup STROBES Strobe commands
 *
 * @{
 */
#define CCx_SRES							(0x30)	/* Reset chip. */
/*
 * Enable and calibrate frequency synthesizer (if MCSM0.FS_AUTOCAL=1).
 * If in RX/TX: Go to a wait state where only the synthesizer is
 * running (for quick RX / TX turn around).
 */
#define CCx_SFSTXON							(0x31)
#define CCx_SXOFF							(0x32)	/* Turn off crystal oscillator. */
#define CCx_SCAL							(0x33)	/* Calibrate frequency synthesizer and turn it off (enables quick start). */
/*
 * Enable RX. Perform calibration first if coming from IDLE and
 * MCSM0.FS_AUTOCAL=1.
 */
#define CCx_SRX								(0x34)
/*
 * In IDLE state: Enable TX. Perform calibration first if
 * MCSM0.FS_AUTOCAL=1. If in RX state and CCA is enabled:
 */
#define CCx_STX								(0x35)
/*
 * Exit RX / TX, turn off frequency synthesizer and exit
 * Wake-On-Radio mode if applicable.
 */
#define CCx_SIDLE							(0x36)
#define CCx_SAFC							(0x37)	/* Perform AFC adjustment of the frequency synthesizer */
#define CCx_SWOR							(0x38)	/* Start automatic RX polling sequence (Wake-on-Radio) */
#define CCx_SPWD							(0x39)	/* Enter power down mode when CSn goes high. */
#define CCx_SFRX							(0x3A)	/* Flush the RX FIFO buffer. */
#define CCx_SFTX							(0x3B)	/* Flush the TX FIFO buffer. */
#define CCx_SWORRST							(0x3C)	/* Reset real time clock. */
/* No operation. May be used to pad strobe commands to two bytes for simpler software. */
#define CCx_SNOP							(0x3D)
/* @} */


/* Status registers (read & burst) */
#define CCx_PARTNUM							(0x30 | 0xc0)
#define CCx_VERSION							(0x31 | 0xc0)
#define CCx_FREQEST							(0x32 | 0xc0)
#define CCx_LQI								(0x33 | 0xc0)
#define CCx_RSSI							(0x34 | 0xc0)
#define CCx_MARCSTATE						(0x35 | 0xc0)
#define CCx_WORTIME1						(0x36 | 0xc0)
#define CCx_WORTIME0						(0x37 | 0xc0)
#define CCx_PKTSTATUS						(0x38 | 0xc0)
#define CCx_VCO_VC_DAC						(0x39 | 0xc0)
#define CCx_TXBYTES							(0x3A | 0xc0)
#define CCx_RXBYTES							(0x3B | 0xc0)

#define CCx_PATABLE							(0x3E)
#define CCx_TXFIFO							(0x3F)
#define CCx_RXFIFO							(0x3F)

#define CCx_FIFO_SIZE						(0x80)	/* 64 bytes for cc1201 */
#define CCx_PACKT_LEN						(CCx_FIFO_SIZE - 3)

/*
 * Packet Control 0 register values
 */
#define CCx_PKTCTRL0_LENGTH_FIXED_bm		(0x00)
#define CCx_PKTCTRL0_LENGTH_VARIABLE_bm		(0x01)
#define CCx_PKTCTRL0_LENGTH_INFINITE_bm		(0x02)

#define CCx_PKTCTRL0_CRC_EN_bm				(0x04)

#define CCx_PKTCTRL0_FMT_NORMAL_bm			(0x00)
#define CCx_PKTCTRL0_FMT_SYNC_SERIAL_bm		(0x10)
#define CCx_PKTCTRL0_FMT_RANDOM_bm			(0x20)
#define CCx_PKTCTRL0_FMT_ASYNC_SERIAL_bm	(0x30)

#define CCx_PKTCTRL0_WHITE_DATA_bm			(0x40)


/*
 * GDOx configuration registers
 */
#define GDOx_CFG_RX_THR_RX_THR_gc			(0x00)
#define GDOx_CFG_RX_THR_RX_EMPTY_gc			(0x01)

/*
 * Associated to the TX FIFO: asserts when TX FIFO is filled above TXFIFO_THR.
 * De-asserts when TX FIFO is drained below TX_FIFOR_THR
 */
#define GDOx_CFG_TX_THR_TX_THR_gc			(0x02)

/*
 * Assets when sync word has been sent / received, and de-asserts at the end of the packet. In Rx the pin will
 * also de-assert when a packet is discarded due to address or maximum length filtering or when
 * the radio enters RXFIFO_OVERFLOW state.
 */
#define GDOx_CFG_SYNC_WORD_gc				(0x06)
#define GDOx_CFG_PKT_RECEIVED_gc			(0x07)
#define GDOx_CFG_PREAMBLE_QUALITY_gc		(0x08)
#define GDOx_CFG_CCA_gc						(0x09)

#define GDOx_CFG_CHIP_RDYn					(0x29)
#define GDOx_CFG_XOSC_STABLE				(0x2b)
#define GDOx_CFG_HI_Z						(0x2e)
#define GDOx_CFG_HW_0						(0x2f)

#define GDOx_CFG_CLK_XOSC1_gc				(0x30)
#define GDOx_CFG_CLK_XOSC1_5_gc				(0x31)
#define GDOx_CFG_CLK_XOSC2_gc				(0x32)
#define GDOx_CFG_CLK_XOSC3_gc				(0x33)
#define GDOx_CFG_CLK_XOSC4_gc				(0x34)
#define GDOx_CFG_CLK_XOSC6_gc				(0x35)
#define GDOx_CFG_CLK_XOSC8_gc				(0x36)
#define GDOx_CFG_CLK_XOSC12_gc				(0x37)
#define GDOx_CFG_CLK_XOSC16_gc				(0x38)
#define GDOx_CFG_CLK_XOSC24_gc				(0x39)
#define GDOx_CFG_CLK_XOSC32_gc				(0x3a)
#define GDOx_CFG_CLK_XOSC48_gc				(0x3b)
#define GDOx_CFG_CLK_XOSC64_gc				(0x3c)
#define GDOx_CFG_CLK_XOSC96_gc				(0x3d)
#define GDOx_CFG_CLK_XOSC128_gc				(0x3e)
#define GDOx_CFG_CLK_XOSC192_gc				(0x3f)

#define GDOx_CFG_INV_bm						(0x40)

/*
 * Status byte bit-masks
 */
#define CCx_STATUS_STATE_bm					(0x70)
#define CCx_STATUS_FIFO_BYTES_bm			(0x0f)
#define CCx_STATUS_CHIP_RDYn_bm				(0x80)

#define CCx_STATUS_STATE_IDLE_bm			(0x00)
#define CCx_STATUS_STATE_RX_bm				(0x10)
#define CCx_STATUS_STATE_TX_bm				(0x20)
#define CCx_STATUS_STATE_FSTXON_bm			(0x30)
#define CCx_STATUS_STATE_CALIBRATE_bm		(0x40)
#define CCx_STATUS_STATE_SETTLING_bm		(0x50)
#define CCx_STATUS_STATE_OVERFLOW_bm		(0x60)
#define CCx_STATUS_STATE_UNDERFLOW_bm		(0x70)

/*
 * Main Radio Control State Machine
 */
#define CCx_MARC_bm							(0x1f)
#define CCx_MARC_SLEEP_gc					(0x00)
#define CCx_MARC_IDLE_gc					(0x01)
#define CCx_MARC_XOFF_gc					(0x02)
#define CCx_MARC_VCOON_MC_gc				(0x03)
#define CCx_MARC_REGON_MC_gc				(0x04)
#define CCx_MARC_MANCAL_gc					(0x05)
#define CCx_MARC_VCOON_gc					(0x06)
#define CCx_MARC_REGON_gc					(0x07)
#define CCx_MARC_STARTCAL_gc				(0x08)
#define CCx_MARC_BWBOOST_gc					(0x09)
#define CCx_MARC_FS_LOCK_gc					(0x0a)
#define CCx_MARC_IFADCON_gc					(0x0b)
#define CCx_MARC_ENDCAL_gc					(0x0c)
#define CCx_MARC_RX_gc						(0x0d)
#define CCx_MARC_RX_END_gc					(0x0e)
#define CCx_MARC_RX_RST_gc					(0x0f)
#define CCx_MARC_TXRX_SWITCH_gc				(0x10)
#define CCx_MARC_RXFIFO_OVERFLOW_gc			(0x11)
#define CCx_MARC_FSTXON_gc					(0x12)
#define CCx_MARC_TX_gc						(0x13)
#define CCx_MARC_TX_END_gc					(0x14)
#define CCx_MARC_RXTX_SWITCH_gc				(0x15)
#define CCx_MARC_TXFIFO_UNDERFLOW_gc		(0x16)

#ifdef __cplusplus
}
#endif

#endif /* __cc120x_h__ */
