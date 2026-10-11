/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-2-Clause) */
/*
 * Copyright (c) 2026, picasso mainline bring-up
 */

#ifndef _DT_BINDINGS_CLK_QCOM_GPUCC_SM7250_H
#define _DT_BINDINGS_CLK_QCOM_GPUCC_SM7250_H

/* ABI：runtime DT 的 GMU 节点引用 gpucc 时钟 0/1/2（cxo/gmu/ahb），
 * 编号不可变动；其余顺序分配。 */
#define GPU_CC_CXO_CLK					0
#define GPU_CC_CX_GMU_CLK				1
#define GPU_CC_AHB_CLK					2
#define GPU_CC_CRC_AHB_CLK				3
#define GPU_CC_CX_APB_CLK				4
#define GPU_CC_CX_SNOC_DVM_CLK				5
#define GPU_CC_CXO_AON_CLK				6
#define GPU_CC_GMU_CLK_SRC				7
#define GPU_CC_GX_GMU_CLK				8
#define GPU_CC_GX_VSENSE_CLK				9
#define GPU_CC_PLL1					10
#define GPU_CC_SLEEP_CLK				11
#define GPU_CC_HLOS1_VOTE_GPU_SMMU_CLK			12

/* GDSC（独立编号空间，GMU power-domains 引用 0=cx / 1=gx） */
#define GPU_CX_GDSC					0
#define GPU_GX_GDSC					1

#endif
