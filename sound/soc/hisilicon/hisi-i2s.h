/*
 * linux/sound/soc/hisilicon/hisi-i2s.h
 *
 * Copyright (C) 2015 Linaro, Ltd
 * Author: Andy Green <andy.green@linaro.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef _HISI_I2S_H
#define _HISI_I2S_H

  enum hisi_bits {
	HII2S_BITS_16,
	HII2S_BITS_18,
	HII2S_BITS_20,
	HII2S_BITS_24,
};

enum hisi_i2s_rates {
	HII2S_FS_RATE_8KHZ = 0,
	HII2S_FS_RATE_16KHZ = 1,
	HII2S_FS_RATE_32KHZ = 2,
	HII2S_FS_RATE_48KHZ = 4,
	HII2S_FS_RATE_96KHZ = 5,
	HII2S_FS_RATE_192KHZ = 6,
};

#define HI_ASP_CFG_R_RST_CTRLEN_REG		0x0
#define HI_ASP_CFG_R_RST_CTRLDIS_REG		0x4
#define HI_ASP_CFG_R_GATE_EN_REG			0xC
#define HI_ASP_CFG_R_GATE_DIS_REG			0x10
#define HI_ASP_CFG_R_GATE_CLKEN_REG 		0x14
#define HI_ASP_CFG_R_GATE_CLKSTAT_REG 		0x18
#define HI_ASP_CFG_R_GATE_CLKDIV_EN_REG	0x1C
#define HI_ASP_CFG_R_CLK1_DIV_REG			0x20
#define HI_ASP_CFG_R_CLK2_DIV_REG			0x24
#define HI_ASP_CFG_R_CLK3_DIV_REG			0x28
#define HI_ASP_CFG_R_CLK4_DIV_REG 			0x2C
#define HI_ASP_CFG_R_CLK5_DIV_REG			0x30
#define HI_ASP_CFG_R_CLK6_DIV_REG			0x34
#define HI_ASP_CFG_R_CLK_SEL_REG 			0x38
#define HI_ASP_CFG_R_SEC_REG 				0x100


#define HI_ASP_SIO_VERSION_REG			(0x3C)
#define HI_ASP_SIO_MODE_REG			(0x40)
#define HI_ASP_SIO_INTSTATUS_REG		(0x44)
#define HI_ASP_SIO_INTCLR_REG			(0x48)
#define HI_ASP_SIO_I2S_LEFT_XD_REG		(0x4C)
#define HI_ASP_SIO_I2S_RIGHT_XD_REG	(0x50)
#define HI_ASP_SIO_I2S_LEFT_RD_REG		(0x54)
#define HI_ASP_SIO_I2S_RIGHT_RD_REG 	(0x58)
#define HI_ASP_SIO_CT_SET_REG			(0x5C)
#define HI_ASP_SIO_CT_CLR_REG			(0x60)
#define HI_ASP_SIO_RX_STA_REG			(0x68)
#define HI_ASP_SIO_TX_STA_REG			(0x6C)
#define HI_ASP_SIO_DATA_WIDTH_SET_REG	(0x78)
#define HI_ASP_SIO_I2S_START_POS_REG	(0x7C)
#define HI_ASP_SIO_I2S_POS_FLAG_REG	(0x80)
#define HI_ASP_SIO_SIGNED_EXT_REG		(0x84)
#define HI_ASP_SIO_I2S_POS_MERGE_EN_REG	(0x88)
#define HI_ASP_SIO_INTMASK_REG				(0x8C)
#define HI_ASP_SIO_I2S_DUAL_RX_CHN_REG	(0xA0)
#define HI_ASP_SIO_I2S_DUAL_TX_CHN_REG		(0xC0)


#define HI_ASP_CFG_R_CLK_SEL_EN				BIT(2)
#define HI_ASP_CFG_R_CLK_SEL				0x140010
#define HI_ASP_CFG_R_CLK1_DIV_SEL			0xbcdc9a
#define HI_ASP_CFG_R_CLK4_DIV_SEL			0x00ff000f
#define HI_ASP_CFG_R_CLK6_DIV_SEL			0x00ff003f
#define HI_ASP_CFG_SIO_MODE				0
#define HI_ASP_SIO_MODE_SEL_EN				BIT(0)
#define HI_ASP_MASK							0xffffffff

#define HI_ASP_SIO_RX_ENABLE				BIT(13)
#define HI_ASP_SIO_TX_ENABLE				BIT(12)
#define HI_ASP_SIO_RX_FIFO_DISABLE			BIT(11)
#define HI_ASP_SIO_TX_FIFO_DISABLE			BIT(10)
#define HI_ASP_SIO_RX_DATA_MERGE			BIT(9)
#define HI_ASP_SIO_TX_DATA_MERGE			BIT(8)
#define HI_ASP_SIO_RX_FIFO_THRESHOLD       	(0x5 << 4)
#define HI_ASP_SIO_TX_FIFO_THRESHOLD		(0xB << 0)
#define HI_ASP_SIO_RX_FIFO_THRESHOLD_CLR	(0xF << 4)
#define HI_ASP_SIO_TX_FIFO_THRESHOLD_CLR	(0xF << 0)
#define HI_ASP_SIO_BURST                   			(0x4)


enum hisi_i2s_formats {
	HII2S_FORMAT_I2S,
	HII2S_FORMAT_PCM_STD,
	HII2S_FORMAT_PCM_USER,
	HII2S_FORMAT_LEFT_JUST,
	HII2S_FORMAT_RIGHT_JUST,
};

#endif/* _HISI_I2S_H */
