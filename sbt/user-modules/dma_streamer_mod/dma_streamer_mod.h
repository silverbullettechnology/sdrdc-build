/** \file      dma_streamer_mod.h
 *  \brief     Zero-copy DMA with userspace buffers for SDRDC sample data, using Xilinx
 *             AXI-DMA or ADI DMAC driver / PL
 *
 *  \copyright Copyright 2013,2014 Silver Bullet Technologies
 *
 *             This program is free software; you can redistribute it and/or modify it
 *             under the terms of the GNU General Public License as published by the Free
 *             Software Foundation; either version 2 of the License, or (at your option)
 *             any later version.
 *
 *             This program is distributed in the hope that it will be useful, but WITHOUT
 *             ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *             FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 *             more details.
 *
 * vim:ts=4:noexpandtab
 */
#ifndef _INCLUDE_DMA_STREAMER_MOD_H
#define _INCLUDE_DMA_STREAMER_MOD_H

#ifdef __KERNEL__
#include <linux/time.h>
#endif


#define DSM_DRIVER_NODE "dma_streamer_mod"


#define DSM_BUS_WIDTH  8
#define DSM_MAX_SIZE   64000000

#define DSM_IOCTL_MAGIC 1

#define DSM_DSXX_CTRL_ENABLE      0x01
#define DSM_DSXX_CTRL_RESET       0x02
#define DSM_DSXX_CTRL_DISABLE     0x03
#define DSM_DSXX_STAT_ENABLED     0
#define DSM_DSXX_STAT_DONE        1
#define DSM_DSXX_STAT_ERROR       2
#define DSM_DSRC_TYPE_INCREMENT   0x00
#define DSM_DSRC_TYPE_DECREMENT   0x01

#define DSM_LVDS_CTRL_STOP        0x00
#define DSM_LVDS_CTRL_TX_ENB      0x01
#define DSM_LVDS_CTRL_RX_ENB      0x02
#define DSM_LVDS_CTRL_T1R1        0x04
#define DSM_LVDS_CTRL_RESET       0x08
#define DSM_LVDS_CTRL_TLAST       0x10
#define DSM_LVDS_CTRL_TX_SWP      0x20
#define DSM_LVDS_CTRL_RX_SWP      0x40

#define DSM_TARGT_ADI1            0x01
#define DSM_TARGT_ADI2            0x02
#define DSM_TARGT_DSXX            0x04
#define DSM_TARGT_NEW             0x08
#define DSM_TARGT_MASK            0x0F

#define DSM_NEW_CTRL_RX1          0x01
#define DSM_NEW_CTRL_RX2          0x02
#define DSM_NEW_CTRL_TX1          0x04
#define DSM_NEW_CTRL_TX2          0x08

struct dsm_xfer_buff
{
	unsigned long  addr;   /* Userspace address for get_user_pages() */
	unsigned long  size;   /* Size of userspace buffer in bytes */
	unsigned long  words;  /* Number of words to transfer per repetition */
};

struct dsm_chan_buffs
{
	struct dsm_xfer_buff  tx;
	struct dsm_xfer_buff  rx;
	unsigned long         ctrl;
};

struct dsm_user_buffs
{
	struct dsm_chan_buffs  adi1;
	struct dsm_chan_buffs  adi2;
	struct dsm_chan_buffs  dsxx;
};


struct dsm_xfer_stats
{
	unsigned long long  bytes;
	struct timespec     total;
	unsigned long       completes;
	unsigned long       errors;
	unsigned long       timeouts;
};

struct dsm_chan_stats
{
	struct dsm_xfer_stats  tx;
	struct dsm_xfer_stats  rx;
};

struct dsm_user_stats
{
	struct dsm_chan_stats  adi1;
	struct dsm_chan_stats  adi2;
	struct dsm_chan_stats  dsxx;
};

struct dsm_fifo_counts
{
	unsigned long  rx_1_ins;
	unsigned long  rx_1_ext;
	unsigned long  rx_2_ins;
	unsigned long  rx_2_ext;
	unsigned long  tx_1_ins;
	unsigned long  tx_1_ext;
	unsigned long  tx_2_ins;
	unsigned long  tx_2_ext;
};

struct dsm_new_adi_regs
{
	unsigned long  adi;
	unsigned long  tx;
	unsigned long  ofs;
	unsigned long  val;
};

// Set the (userspace) addresses and sizes of the buffers.  These must be page-aligned (ie
// allocated with posix_memalign()), locked in with mlock(), and size a multiple of
// DSM_BUS_WIDTH.  
#define  DSM_IOCS_MAP  _IOW(DSM_IOCTL_MAGIC, 0, struct dsm_user_buffs *)

// Trigger a transaction, after setting up the buffers with a successful DSM_IOCS_MAP.  If
// the mapped buffers have a nonzero tx_size, a TX transfer is started.  If the mapped
// buffers have a nonzero rx_size, a RX transfer is started.  The two directions may be
// run in parallel.  The calling process will block until both transfers are complete, or
// a timeout occurs.
#define  DSM_IOCS_TRIGGER  _IOW(DSM_IOCTL_MAGIC, 1, unsigned long)

// Read statistics from the transfer triggered with DSM_IOCS_TRIGGER
#define  DSM_IOCG_STATS  _IOR(DSM_IOCTL_MAGIC, 2, struct dsm_user_stats *)

// Unmap the buffers buffer mapped with DSM_IOCS_MAP, before DSM_IOCS_TRIGGER.  
#define  DSM_IOCS_UNMAP  _IOW(DSM_IOCTL_MAGIC, 3, unsigned long)

// Set a timeout in jiffies on the DMA transfer
#define  DSM_IOCS_TIMEOUT  _IOW(DSM_IOCTL_MAGIC, 4, unsigned long)

// Access to data source regs
#define  DSM_IOCS_DSRC_CTRL   _IOW(DSM_IOCTL_MAGIC,  5, unsigned long)
#define  DSM_IOCG_DSRC_STAT   _IOR(DSM_IOCTL_MAGIC,  6, unsigned long *)
#define  DSM_IOCG_DSRC_BYTES  _IOR(DSM_IOCTL_MAGIC,  7, unsigned long *)
#define  DSM_IOCS_DSRC_BYTES  _IOW(DSM_IOCTL_MAGIC,  8, unsigned long)
#define  DSM_IOCG_DSRC_SENT   _IOR(DSM_IOCTL_MAGIC,  9, unsigned long *)
#define  DSM_IOCG_DSRC_TYPE   _IOR(DSM_IOCTL_MAGIC, 10, unsigned long *)
#define  DSM_IOCS_DSRC_TYPE   _IOW(DSM_IOCTL_MAGIC, 11, unsigned long)

// Access to data sink regs
#define  DSM_IOCS_DSNK_CTRL   _IOW(DSM_IOCTL_MAGIC, 12, unsigned long)
#define  DSM_IOCG_DSNK_STAT   _IOR(DSM_IOCTL_MAGIC, 13, unsigned long *)
#define  DSM_IOCG_DSNK_BYTES  _IOR(DSM_IOCTL_MAGIC, 14, unsigned long *)
#define  DSM_IOCG_DSNK_SUM    _IOR(DSM_IOCTL_MAGIC, 15, unsigned long *)

// Access to ADI LVDS regs
#define  DSM_IOCS_ADI1_OLD_CTRL    _IOW(DSM_IOCTL_MAGIC, 20, unsigned long)
#define  DSM_IOCG_ADI1_OLD_CTRL    _IOR(DSM_IOCTL_MAGIC, 21, unsigned long *)
#define  DSM_IOCS_ADI1_OLD_TX_CNT  _IOW(DSM_IOCTL_MAGIC, 22, unsigned long)
#define  DSM_IOCG_ADI1_OLD_TX_CNT  _IOR(DSM_IOCTL_MAGIC, 23, unsigned long *)
#define  DSM_IOCS_ADI1_OLD_RX_CNT  _IOW(DSM_IOCTL_MAGIC, 24, unsigned long)
#define  DSM_IOCG_ADI1_OLD_RX_CNT  _IOR(DSM_IOCTL_MAGIC, 25, unsigned long *)
#define  DSM_IOCG_ADI1_OLD_SUM     _IOR(DSM_IOCTL_MAGIC, 26, unsigned long *)
#define  DSM_IOCG_ADI1_OLD_LAST    _IOR(DSM_IOCTL_MAGIC, 27, unsigned long *)
#define  DSM_IOCS_ADI2_OLD_CTRL    _IOW(DSM_IOCTL_MAGIC, 30, unsigned long)
#define  DSM_IOCG_ADI2_OLD_CTRL    _IOR(DSM_IOCTL_MAGIC, 31, unsigned long *)
#define  DSM_IOCS_ADI2_OLD_TX_CNT  _IOW(DSM_IOCTL_MAGIC, 32, unsigned long)
#define  DSM_IOCG_ADI2_OLD_TX_CNT  _IOR(DSM_IOCTL_MAGIC, 33, unsigned long *)
#define  DSM_IOCS_ADI2_OLD_RX_CNT  _IOW(DSM_IOCTL_MAGIC, 34, unsigned long)
#define  DSM_IOCG_ADI2_OLD_RX_CNT  _IOR(DSM_IOCTL_MAGIC, 35, unsigned long *)
#define  DSM_IOCG_ADI2_OLD_SUM     _IOR(DSM_IOCTL_MAGIC, 36, unsigned long *)
#define  DSM_IOCG_ADI2_OLD_LAST    _IOR(DSM_IOCTL_MAGIC, 37, unsigned long *)

// Target list bitmap from xparameters
#define  DSM_IOCG_TARGET_LIST  _IOR(DSM_IOCTL_MAGIC, 40, unsigned long *)

// Get counters from ADI DPFD/LVDS interface FIFOs. 
#define  DSM_IOCG_FIFO_CNT     _IOR(DSM_IOCTL_MAGIC, 41, unsigned long *)

// Get clock counters for digital interface
#define  DSM_IOCG_ADI1_OLD_CLK_CNT      _IOR(DSM_IOCTL_MAGIC, 50, unsigned long *)
#define  DSM_IOCG_ADI2_OLD_CLK_CNT      _IOR(DSM_IOCTL_MAGIC, 51, unsigned long *)

// Arbitrary register access for now 
#define  DSM_IOCG_ADI_NEW_REG      _IOR(DSM_IOCTL_MAGIC, 52, struct dsm_new_adi_regs *)
#define  DSM_IOCS_ADI_NEW_REG_SO   _IOW(DSM_IOCTL_MAGIC, 53, struct dsm_new_adi_regs *)
#define  DSM_IOCS_ADI_NEW_REG_RB   _IOW(DSM_IOCTL_MAGIC, 54, struct dsm_new_adi_regs *)


#endif // _INCLUDE_DMA_STREAMER_MOD_H
/* Ends    : dma_streamer_mod.h */
