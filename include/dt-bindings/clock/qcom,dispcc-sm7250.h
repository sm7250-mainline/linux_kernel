/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2024, SM7250 DISPCC Clock IDs
 *
 * 时钟清单移植自下游 drivers/clk/qcom/dispcc-lito.c
 * （Xiaomi_Kernel_OpenSource-picasso-r-oss，Android R 4.19）。
 * 注意：编号 0-10 是既有 ABI（sm7250.dtsi 与 runtime DT 的数字引用依赖），
 * 不可变更；其余时钟在其后继续递增编号。
 */

#ifndef __DT_BINDINGS_CLOCK_QCOM_DISPCC_SM7250_H
#define __DT_BINDINGS_CLOCK_QCOM_DISPCC_SM7250_H

/* DISPCC clocks —— 0-10 为既有编号（勿改） */
#define DISP_CC_MDSS_AHB_CLK				0
#define DISP_CC_MDSS_MDP_CLK				1
#define DISP_CC_MDSS_MDP_LUT_CLK			2
#define DISP_CC_MDSS_VSYNC_CLK				3
#define DISP_CC_MDSS_ROT_CLK				4
#define DISP_CC_MDSS_BYTE0_CLK				5
#define DISP_CC_MDSS_BYTE0_INTF_CLK			6
#define DISP_CC_MDSS_PCLK0_CLK				7
#define DISP_CC_MDSS_ESC0_CLK				8
#define DISP_CC_MDSS_BYTE0_CLK_SRC			9
#define DISP_CC_MDSS_PCLK0_CLK_SRC			10
/* 以下为新增（沿用下游命名） */
#define DISP_CC_PLL0					11
#define DISP_CC_MDSS_AHB_CLK_SRC			12
#define DISP_CC_MDSS_BYTE0_DIV_CLK_SRC			13
#define DISP_CC_MDSS_BYTE1_CLK				14
#define DISP_CC_MDSS_BYTE1_CLK_SRC			15
#define DISP_CC_MDSS_BYTE1_DIV_CLK_SRC			16
#define DISP_CC_MDSS_BYTE1_INTF_CLK			17
#define DISP_CC_MDSS_DP_AUX_CLK				18
#define DISP_CC_MDSS_DP_AUX_CLK_SRC			19
#define DISP_CC_MDSS_DP_CRYPTO_CLK			20
#define DISP_CC_MDSS_DP_CRYPTO_CLK_SRC			21
#define DISP_CC_MDSS_DP_LINK_CLK			22
#define DISP_CC_MDSS_DP_LINK_CLK_SRC			23
#define DISP_CC_MDSS_DP_LINK_DIV_CLK_SRC		24
#define DISP_CC_MDSS_DP_LINK_INTF_CLK			25
#define DISP_CC_MDSS_DP_PIXEL1_CLK			26
#define DISP_CC_MDSS_DP_PIXEL1_CLK_SRC			27
#define DISP_CC_MDSS_DP_PIXEL_CLK			28
#define DISP_CC_MDSS_DP_PIXEL_CLK_SRC			29
#define DISP_CC_MDSS_ESC0_CLK_SRC			30
#define DISP_CC_MDSS_ESC1_CLK				31
#define DISP_CC_MDSS_ESC1_CLK_SRC			32
#define DISP_CC_MDSS_MDP_CLK_SRC			33
#define DISP_CC_MDSS_NON_GDSC_AHB_CLK			34
#define DISP_CC_MDSS_PCLK1_CLK				35
#define DISP_CC_MDSS_PCLK1_CLK_SRC			36
#define DISP_CC_MDSS_ROT_CLK_SRC			37
#define DISP_CC_MDSS_RSCC_AHB_CLK			38
#define DISP_CC_MDSS_RSCC_VSYNC_CLK			39
#define DISP_CC_MDSS_VSYNC_CLK_SRC			40
#define DISP_CC_SLEEP_CLK				41
#define DISP_CC_SLEEP_CLK_SRC				42
#define DISP_CC_XO_CLK					43
#define DISP_CC_XO_CLK_SRC				44

/* DISPCC resets */
#define DISP_CC_MDSS_CORE_BCR				0
#define DISP_CC_MDSS_RSCC_BCR				1

/* DISPCC power domains */
#define MDSS_GDSC					0

#endif
