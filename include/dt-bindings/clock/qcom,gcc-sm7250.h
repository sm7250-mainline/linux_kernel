/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2024, SM7250 GCC Clock IDs
 */

#ifndef __DT_BINDINGS_CLOCK_QCOM_GCC_SM7250_H
#define __DT_BINDINGS_CLOCK_QCOM_GCC_SM7250_H

/* GPLLs */
#define GPLL0							0
#define GPLL0_OUT_EVEN					1
#define GPLL6							2
#define GPLL7							3
/* dispcc GPLL0 分支（AON 投票 0x52000 bit20，供 dispcc 低频档） */
#define GCC_DISP_GPLL0_CLK_SRC				160
/* gpucc GMU RCG 的 GPLL0 父时钟（AON 投票 bit15/bit16，下游 gcc-lito 实值） */
#define GCC_GPU_GPLL0_CLK_SRC				161
#define GCC_GPU_GPLL0_DIV_CLK_SRC				162

/* QUPv3 Wrapper 0 SE Clock Sources (RCGs) */
#define GCC_QUPV3_WRAP0_S0_CLK_SRC		10
#define GCC_QUPV3_WRAP0_S1_CLK_SRC		11
#define GCC_QUPV3_WRAP0_S2_CLK_SRC		12
#define GCC_QUPV3_WRAP0_S4_CLK_SRC		13
#define GCC_QUPV3_WRAP0_S5_CLK_SRC		14

/* QUPv3 Wrapper 1 SE Clock Sources (RCGs) */
#define GCC_QUPV3_WRAP1_S0_CLK_SRC		15
#define GCC_QUPV3_WRAP1_S1_CLK_SRC		16
#define GCC_QUPV3_WRAP1_S2_CLK_SRC		17
#define GCC_QUPV3_WRAP1_S3_CLK_SRC		18
#define GCC_QUPV3_WRAP1_S4_CLK_SRC		19
#define GCC_QUPV3_WRAP1_S5_CLK_SRC		20

/* SDCC Clock Sources */
#define GCC_SDCC1_APPS_CLK_SRC			25
#define GCC_SDCC2_APPS_CLK_SRC			26

/* UFS Clock Sources */
#define GCC_UFS_PHY_AXI_CLK_SRC			30
#define GCC_UFS_PHY_UNIPRO_CORE_CLK_SRC		31

/* USB Clock Sources */
#define GCC_USB30_PRIM_MASTER_CLK_SRC		35

/* QUPv3 Wrapper 0 AHB Branch Clocks */
#define GCC_QUPV3_WRAP_0_M_AHB_CLK		40
#define GCC_QUPV3_WRAP_0_S_AHB_CLK		41

/* QUPv3 Wrapper 0 SE Branch Clocks */
#define GCC_QUPV3_WRAP0_S0_CLK			42
#define GCC_QUPV3_WRAP0_S1_CLK			43
#define GCC_QUPV3_WRAP0_S2_CLK			44
#define GCC_QUPV3_WRAP0_S4_CLK			45
#define GCC_QUPV3_WRAP0_S5_CLK			46

/* QUPv3 Wrapper 1 AHB Branch Clocks */
#define GCC_QUPV3_WRAP_1_M_AHB_CLK		50
#define GCC_QUPV3_WRAP_1_S_AHB_CLK		51

/* QUPv3 Wrapper 1 SE Branch Clocks */
#define GCC_QUPV3_WRAP1_S0_CLK			52
#define GCC_QUPV3_WRAP1_S1_CLK			53
#define GCC_QUPV3_WRAP1_S2_CLK			54
#define GCC_QUPV3_WRAP1_S3_CLK			55
#define GCC_QUPV3_WRAP1_S4_CLK			56
#define GCC_QUPV3_WRAP1_S5_CLK			57

/* UFS Branch Clocks */
#define GCC_UFS_PHY_AXI_CLK			60
#define GCC_UFS_PHY_AHB_CLK			61
#define GCC_UFS_PHY_UNIPRO_CORE_CLK		62
#define GCC_UFS_PHY_ICE_CORE_CLK		63
#define GCC_UFS_PHY_ICE_CORE_HW_CTL_CLK		64
#define GCC_UFS_PHY_TX_SYMBOL_0_CLK		65
#define GCC_UFS_PHY_RX_SYMBOL_0_CLK		66
#define GCC_UFS_PHY_RX_SYMBOL_1_CLK		67
#define GCC_UFS_1X_CLKREF_CLK			68
#define GCC_UFS_PHY_PHY_AUX_CLK			69
#define GCC_AGGRE_UFS_PHY_AXI_CLK		70

/* USB Branch Clocks */
#define GCC_USB30_PRIM_MASTER_CLK		75
#define GCC_USB30_PRIM_MOCK_UTMI_CLK		76
#define GCC_USB30_PRIM_SLEEP_CLK		77
#define GCC_CFG_NOC_USB3_PRIM_AXI_CLK		78
#define GCC_USB3_PRIM_CLKREF_CLK		79
#define GCC_USB3_PRIM_PHY_AUX_CLK		80
#define GCC_USB3_PRIM_PHY_COM_AUX_CLK		81
#define GCC_USB3_PRIM_PHY_PIPE_CLK		82
#define GCC_USB_PHY_CFG_AHB2PHY_CLK		83
#define GCC_AGGRE_USB3_PRIM_AXI_CLK		84

/* SDCC Branch Clocks */
#define GCC_SDCC1_AHB_CLK			90
#define GCC_SDCC1_APPS_CLK			91
#define GCC_SDCC1_ICE_CORE_CLK			92
#define GCC_SDCC2_AHB_CLK			93
#define GCC_SDCC2_APPS_CLK			94

/* Camera/Video/Display AHB */
#define GCC_CAMERA_AHB_CLK			100
#define GCC_VIDEO_AHB_CLK			110
/* 本树自有编号（下游 gcc-lito 为 108，与本项目时钟号空间冲突；
   无 DT 直接引用，仅驱动内部接线） */
#define GCC_UFS_PHY_PHY_AUX_CLK_SRC		111
#define GCC_DISP_AHB_CLK			120
#define GCC_DISP_HF_AXI_CLK			121
#define GCC_DISP_SF_AXI_CLK			122

/* PRNG */
#define GCC_PRNG_AHB_CLK			130

/* GPU GCC Clocks */
#define GCC_GPU_MEMNOC_GFX_CLK			140
#define GCC_GPU_SNOC_DVM_GFX_CLK		141
#define GCC_DDRSS_GPU_AXI_CLK			142

/* NPU GCC Clocks */
#define GCC_NPU_BWMON_CFG_AHB_CLK		150
#define GCC_NPU_BWMON_AXI_CLK			151
#define GCC_NPU_BWMON2_AXI_CLK			152

/* Reset IDs */
#define GCC_QUSB2PHY_PRIM_BCR			200
#define GCC_UFS_PHY_BCR				201
#define GCC_USB30_PRIM_BCR			202
#define GCC_USB3_DP_PHY_PRIM_BCR		203
#define GCC_USB3_PHY_PRIM_BCR			204
#define GCC_USB_PHY_CFG_AHB2PHY_BCR		205

/* GDSC IDs */
#define UFS_PHY_GDSC				0
#define USB30_PRIM_GDSC				1
#define HLOS1_VOTE_MMNOC_MMU_TBU_HF0_GDSC	2
#define HLOS1_VOTE_MMNOC_MMU_TBU_HF1_GDSC	3
#define HLOS1_VOTE_MMNOC_MMU_TBU_SF0_GDSC	4

#endif
