// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 * Copyright (c) 2024, Danila Tikhonov <danila@jiaxyga.com>
 */

#include <linux/device.h>
#include <linux/interconnect.h>
#include <linux/interconnect-provider.h>
#include <linux/mod_devicetable.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <dt-bindings/interconnect/qcom,sm7250.h>

#include "bcm-voter.h"
#include "icc-rpmh.h"

static struct qcom_icc_node acm_apps;
static struct qcom_icc_node acm_sys_tcu;
static struct qcom_icc_node ebi;
static struct qcom_icc_node llcc_mc;
static struct qcom_icc_node qhm_a1noc_cfg;
static struct qcom_icc_node qhm_a2noc_cfg;
static struct qcom_icc_node qhm_cnoc_dc_noc;
static struct qcom_icc_node qhm_gemnoc_cfg;
static struct qcom_icc_node qhm_ipa_virt;
static struct qcom_icc_node qhm_mnoc_cfg;
static struct qcom_icc_node qhm_qdss_bam;
static struct qcom_icc_node qhm_qup_center;
static struct qcom_icc_node qhm_qup_north;
static struct qcom_icc_node qhm_snoc_cfg;
static struct qcom_icc_node qhm_spdm;
static struct qcom_icc_node qhm_tsif;
static struct qcom_icc_node qhs_a1_noc_cfg;
static struct qcom_icc_node qhs_a2_noc_cfg;
static struct qcom_icc_node qhs_ahb2phy_north;
static struct qcom_icc_node qhs_ahb2phy_south;
static struct qcom_icc_node qhs_ahb2phy_west;
static struct qcom_icc_node qhs_aop;
static struct qcom_icc_node qhs_aoss;
static struct qcom_icc_node qhs_apss;
static struct qcom_icc_node qhs_camera_cfg;
static struct qcom_icc_node qhs_camera_nrt_thrott_cfg;
static struct qcom_icc_node qhs_camera_rt_throttle_cfg;
static struct qcom_icc_node qhs_clk_ctl;
static struct qcom_icc_node qhs_compute_dsp_cfg;
static struct qcom_icc_node qhs_cpr_cx;
static struct qcom_icc_node qhs_cpr_mx;
static struct qcom_icc_node qhs_crypto0_cfg;
static struct qcom_icc_node qhs_ddrss_cfg;
static struct qcom_icc_node qhs_display_cfg;
static struct qcom_icc_node qhs_display_throttle_cfg;
static struct qcom_icc_node qhs_emmc_cfg;
static struct qcom_icc_node qhs_gemnoc;
static struct qcom_icc_node qhs_glm;
static struct qcom_icc_node qhs_gpuss_cfg;
static struct qcom_icc_node qhs_imem_cfg;
static struct qcom_icc_node qhs_ipa;
static struct qcom_icc_node qhs_llcc;
static struct qcom_icc_node qhs_mdsp_ms_mpu_cfg;
static struct qcom_icc_node qhs_mnoc_cfg;
static struct qcom_icc_node qhs_pcie_cfg;
static struct qcom_icc_node qhs_pdm;
static struct qcom_icc_node qhs_pimem_cfg;
static struct qcom_icc_node qhs_prng;
static struct qcom_icc_node qhs_qdss_cfg;
static struct qcom_icc_node qhs_qup_core_0;
static struct qcom_icc_node qhs_qup_core_1;
static struct qcom_icc_node qhs_qupv3_center;
static struct qcom_icc_node qhs_qupv3_north;
static struct qcom_icc_node qhs_sdc2;
static struct qcom_icc_node qhs_sdc4;
static struct qcom_icc_node qhs_snoc_cfg;
static struct qcom_icc_node qhs_spdm;
static struct qcom_icc_node qhs_tcsr;
static struct qcom_icc_node qhs_tlmm_north;
static struct qcom_icc_node qhs_tlmm_south;
static struct qcom_icc_node qhs_tlmm_west;
static struct qcom_icc_node qhs_tsif;
static struct qcom_icc_node qhs_ufs_mem_cfg;
static struct qcom_icc_node qhs_usb3_0;
static struct qcom_icc_node qhs_venus_cfg;
static struct qcom_icc_node qhs_venus_cvp_throttle_cfg;
static struct qcom_icc_node qhs_venus_throttle_cfg;
static struct qcom_icc_node qhs_vsense_ctrl_cfg;
static struct qcom_icc_node qnm_aggre1_noc;
static struct qcom_icc_node qnm_aggre2_noc;
static struct qcom_icc_node qnm_cmpnoc;
static struct qcom_icc_node qnm_cnoc;
static struct qcom_icc_node qnm_gemnoc;
static struct qcom_icc_node qnm_mnoc_hf;
static struct qcom_icc_node qnm_mnoc_sf;
static struct qcom_icc_node qnm_npu;
static struct qcom_icc_node qnm_npu_dsp;
static struct qcom_icc_node qnm_pcie;
static struct qcom_icc_node qnm_snoc;
static struct qcom_icc_node qnm_snoc_gc;
static struct qcom_icc_node qnm_snoc_sf;
static struct qcom_icc_node qns2_mem_noc;
static struct qcom_icc_node qns_a1noc_snoc;
static struct qcom_icc_node qns_a2noc_snoc;
static struct qcom_icc_node qns_camnoc_uncomp;
static struct qcom_icc_node qns_cdsp_gemnoc;
static struct qcom_icc_node qns_cnoc;
static struct qcom_icc_node qns_cnoc_a2noc;
static struct qcom_icc_node qns_gem_noc_snoc;
static struct qcom_icc_node qns_gemnoc_gc;
static struct qcom_icc_node qns_gemnoc_sf;
static struct qcom_icc_node qns_ipa_virt;
static struct qcom_icc_node qns_llcc;
static struct qcom_icc_node qns_mem_noc_hf;
static struct qcom_icc_node qns_pcie_gemnoc;
static struct qcom_icc_node qxm_camnoc_hf0;
static struct qcom_icc_node qxm_camnoc_hf0_uncomp;
static struct qcom_icc_node qxm_camnoc_hf1;
static struct qcom_icc_node qxm_camnoc_hf1_uncomp;
static struct qcom_icc_node qxm_camnoc_icp;
static struct qcom_icc_node qxm_camnoc_icp_uncomp;
static struct qcom_icc_node qxm_camnoc_sf;
static struct qcom_icc_node qxm_camnoc_sf_uncomp;
static struct qcom_icc_node qxm_crypto;
static struct qcom_icc_node qxm_gpu;
static struct qcom_icc_node qxm_ipa;
static struct qcom_icc_node qxm_mdp0;
static struct qcom_icc_node qxm_mdp1;
static struct qcom_icc_node qxm_pimem;
static struct qcom_icc_node qxm_rot;
static struct qcom_icc_node qxm_venus0;
static struct qcom_icc_node qxm_venus1;
static struct qcom_icc_node qxm_venus_arm9;
static struct qcom_icc_node qxs_imem;
static struct qcom_icc_node qxs_pimem;
static struct qcom_icc_node srvc_aggre1_noc;
static struct qcom_icc_node srvc_aggre2_noc;
static struct qcom_icc_node srvc_cnoc;
static struct qcom_icc_node srvc_gemnoc;
static struct qcom_icc_node srvc_mnoc;
static struct qcom_icc_node srvc_snoc;
static struct qcom_icc_node xm_emmc;
static struct qcom_icc_node xm_gic;
static struct qcom_icc_node xm_pcie3_0;
static struct qcom_icc_node xm_qdss_dap;
static struct qcom_icc_node xm_qdss_etr;
static struct qcom_icc_node xm_sdc2;
static struct qcom_icc_node xm_sdc4;
static struct qcom_icc_node xm_ufs_mem;
static struct qcom_icc_node xm_usb3_0;
static struct qcom_icc_node xs_qdss_stm;
static struct qcom_icc_node xs_sys_tcu_cfg;

static struct qcom_icc_node qhm_a1noc_cfg = {
	.name = "qhm_a1noc_cfg",
	.channels = 1,
	.buswidth = 4,
	.num_links = 1,
	.link_nodes = { &srvc_aggre1_noc },
};

static struct qcom_icc_node qhm_qup_center = {
	.name = "qhm_qup_center",
	.channels = 1,
	.buswidth = 4,
	.num_links = 1,
	.link_nodes = { &qns_a1noc_snoc },
};

static struct qcom_icc_node qhm_tsif = {
	.name = "qhm_tsif",
	.channels = 1,
	.buswidth = 4,
	.num_links = 1,
	.link_nodes = { &qns_a1noc_snoc },
};

static struct qcom_icc_node xm_emmc = {
	.name = "xm_emmc",
	.channels = 1,
	.buswidth = 8,
	.num_links = 1,
	.link_nodes = { &qns_a1noc_snoc },
};

static struct qcom_icc_node xm_sdc2 = {
	.name = "xm_sdc2",
	.channels = 1,
	.buswidth = 8,
	.num_links = 1,
	.link_nodes = { &qns_a1noc_snoc },
};

static struct qcom_icc_node xm_sdc4 = {
	.name = "xm_sdc4",
	.channels = 1,
	.buswidth = 8,
	.num_links = 1,
	.link_nodes = { &qns_a1noc_snoc },
};

static struct qcom_icc_node xm_ufs_mem = {
	.name = "xm_ufs_mem",
	.channels = 1,
	.buswidth = 8,
	.num_links = 1,
	.link_nodes = { &qns_a1noc_snoc },
};

static struct qcom_icc_node qhm_a2noc_cfg = {
	.name = "qhm_a2noc_cfg",
	.channels = 1,
	.buswidth = 4,
	.num_links = 1,
	.link_nodes = { &srvc_aggre2_noc },
};

static struct qcom_icc_node qhm_qdss_bam = {
	.name = "qhm_qdss_bam",
	.channels = 1,
	.buswidth = 4,
	.num_links = 1,
	.link_nodes = { &qns_a2noc_snoc },
};

static struct qcom_icc_node qhm_qup_north = {
	.name = "qhm_qup_north",
	.channels = 1,
	.buswidth = 4,
	.num_links = 1,
	.link_nodes = { &qns_a2noc_snoc },
};

static struct qcom_icc_node qnm_cnoc = {
	.name = "qnm_cnoc",
	.channels = 1,
	.buswidth = 8,
	.num_links = 1,
	.link_nodes = { &qns_a2noc_snoc },
};

static struct qcom_icc_node qxm_crypto = {
	.name = "qxm_crypto",
	.channels = 1,
	.buswidth = 8,
	.num_links = 1,
	.link_nodes = { &qns_a2noc_snoc },
};

static struct qcom_icc_node qxm_ipa = {
	.name = "qxm_ipa",
	.channels = 1,
	.buswidth = 8,
	.num_links = 1,
	.link_nodes = { &qns_a2noc_snoc },
};

static struct qcom_icc_node xm_pcie3_0 = {
	.name = "xm_pcie3_0",
	.channels = 1,
	.buswidth = 8,
	.num_links = 1,
	.link_nodes = { &qns_pcie_gemnoc },
};

static struct qcom_icc_node xm_qdss_etr = {
	.name = "xm_qdss_etr",
	.channels = 1,
	.buswidth = 8,
	.num_links = 1,
	.link_nodes = { &qns_a2noc_snoc },
};

static struct qcom_icc_node xm_usb3_0 = {
	.name = "xm_usb3_0",
	.channels = 1,
	.buswidth = 8,
	.num_links = 1,
	.link_nodes = { &qns_a2noc_snoc },
};

static struct qcom_icc_node qxm_camnoc_hf0_uncomp = {
	.name = "qxm_camnoc_hf0_uncomp",
	.channels = 2,
	.buswidth = 32,
	.num_links = 1,
	.link_nodes = { &qns_camnoc_uncomp },
};

static struct qcom_icc_node qxm_camnoc_hf1_uncomp = {
	.name = "qxm_camnoc_hf1_uncomp",
	.channels = 1,
	.buswidth = 32,
	.num_links = 1,
	.link_nodes = { &qns_camnoc_uncomp },
};

static struct qcom_icc_node qxm_camnoc_sf_uncomp = {
	.name = "qxm_camnoc_sf_uncomp",
	.channels = 1,
	.buswidth = 32,
	.num_links = 1,
	.link_nodes = { &qns_camnoc_uncomp },
};

static struct qcom_icc_node qxm_camnoc_icp_uncomp = {
	.name = "qxm_camnoc_icp_uncomp",
	.channels = 1,
	.buswidth = 32,
	.num_links = 1,
	.link_nodes = { &qns_camnoc_uncomp },
};

static struct qcom_icc_node qnm_npu = {
	.name = "qnm_npu",
	.channels = 1,
	.buswidth = 32,
	.num_links = 1,
	.link_nodes = { &qns_cdsp_gemnoc },
};

static struct qcom_icc_node qnm_npu_dsp = {
	.name = "qnm_npu_dsp",
	.channels = 1,
	.buswidth = 32,
	.num_links = 1,
	.link_nodes = { &qns_cdsp_gemnoc },
};

static struct qcom_icc_node qhm_spdm = {
	.name = "qhm_spdm",
	.channels = 1,
	.buswidth = 4,
	.num_links = 1,
	.link_nodes = { &qns_cnoc_a2noc },
};

static struct qcom_icc_node qnm_snoc = {
	.name = "qnm_snoc",
	.channels = 1,
	.buswidth = 8,
	.num_links = 49,
	.link_nodes = { &qhs_tlmm_south, &qhs_camera_cfg, &qhs_sdc4, &qhs_sdc2, &qhs_mnoc_cfg, &qhs_ufs_mem_cfg, &qhs_qupv3_center, &qhs_glm, &qhs_pdm, &qhs_camera_nrt_thrott_cfg, &qhs_a2_noc_cfg, &qhs_qdss_cfg, &qhs_camera_rt_throttle_cfg, &qhs_display_cfg, &qhs_pcie_cfg, &qhs_display_throttle_cfg, &qhs_tcsr, &qhs_venus_cvp_throttle_cfg, &qhs_ddrss_cfg, &qhs_ahb2phy_north, &qhs_snoc_cfg, &qhs_gpuss_cfg, &qhs_venus_cfg, &qhs_tsif, &qhs_compute_dsp_cfg, &qhs_clk_ctl, &qhs_aop, &qhs_qupv3_north, &qhs_ahb2phy_south, &srvc_cnoc, &qhs_ahb2phy_west, &qhs_usb3_0, &qhs_venus_throttle_cfg, &qhs_ipa, &qhs_cpr_cx, &qhs_tlmm_west, &qhs_a1_noc_cfg, &qhs_aoss, &qhs_prng, &qhs_vsense_ctrl_cfg, &qhs_emmc_cfg, &qhs_spdm, &qhs_crypto0_cfg, &qhs_pimem_cfg, &qhs_tlmm_north, &qhs_cpr_mx, &qhs_imem_cfg, &qhs_qup_core_0, &qhs_qup_core_1 },
};

static struct qcom_icc_node xm_qdss_dap = {
	.name = "xm_qdss_dap",
	.channels = 1,
	.buswidth = 8,
	.num_links = 50,
	.link_nodes = { &qhs_tlmm_south, &qhs_camera_cfg, &qhs_sdc4, &qhs_sdc2, &qhs_mnoc_cfg, &qhs_ufs_mem_cfg, &qhs_qupv3_center, &qhs_glm, &qhs_pdm, &qhs_camera_nrt_thrott_cfg, &qhs_a2_noc_cfg, &qhs_qdss_cfg, &qhs_camera_rt_throttle_cfg, &qhs_display_cfg, &qhs_pcie_cfg, &qhs_display_throttle_cfg, &qhs_tcsr, &qhs_venus_cvp_throttle_cfg, &qhs_ddrss_cfg, &qns_cnoc_a2noc, &qhs_ahb2phy_north, &qhs_snoc_cfg, &qhs_gpuss_cfg, &qhs_venus_cfg, &qhs_tsif, &qhs_compute_dsp_cfg, &qhs_clk_ctl, &qhs_aop, &qhs_qupv3_north, &qhs_ahb2phy_south, &srvc_cnoc, &qhs_ahb2phy_west, &qhs_usb3_0, &qhs_venus_throttle_cfg, &qhs_ipa, &qhs_cpr_cx, &qhs_tlmm_west, &qhs_a1_noc_cfg, &qhs_aoss, &qhs_prng, &qhs_vsense_ctrl_cfg, &qhs_emmc_cfg, &qhs_spdm, &qhs_crypto0_cfg, &qhs_pimem_cfg, &qhs_tlmm_north, &qhs_cpr_mx, &qhs_imem_cfg, &qhs_qup_core_0, &qhs_qup_core_1 },
};

static struct qcom_icc_node qhm_cnoc_dc_noc = {
	.name = "qhm_cnoc_dc_noc",
	.channels = 1,
	.buswidth = 4,
	.num_links = 2,
	.link_nodes = { &qhs_llcc, &qhs_gemnoc },
};

static struct qcom_icc_node acm_apps = {
	.name = "acm_apps",
	.channels = 1,
	.buswidth = 16,
	.num_links = 2,
	.link_nodes = { &qns_llcc, &qns_gem_noc_snoc },
};

static struct qcom_icc_node acm_sys_tcu = {
	.name = "acm_sys_tcu",
	.channels = 1,
	.buswidth = 8,
	.num_links = 2,
	.link_nodes = { &qns_llcc, &qns_gem_noc_snoc },
};

static struct qcom_icc_node qhm_gemnoc_cfg = {
	.name = "qhm_gemnoc_cfg",
	.channels = 1,
	.buswidth = 4,
	.num_links = 2,
	.link_nodes = { &srvc_gemnoc, &qhs_mdsp_ms_mpu_cfg },
};

static struct qcom_icc_node qnm_cmpnoc = {
	.name = "qnm_cmpnoc",
	.channels = 1,
	.buswidth = 32,
	.num_links = 2,
	.link_nodes = { &qns_llcc, &qns_gem_noc_snoc },
};

static struct qcom_icc_node qnm_mnoc_hf = {
	.name = "qnm_mnoc_hf",
	.channels = 2,
	.buswidth = 32,
	.num_links = 1,
	.link_nodes = { &qns_llcc },
};

static struct qcom_icc_node qnm_mnoc_sf = {
	.name = "qnm_mnoc_sf",
	.channels = 1,
	.buswidth = 32,
	.num_links = 2,
	.link_nodes = { &qns_llcc, &qns_gem_noc_snoc },
};

static struct qcom_icc_node qnm_pcie = {
	.name = "qnm_pcie",
	.channels = 1,
	.buswidth = 8,
	.num_links = 2,
	.link_nodes = { &qns_llcc, &qns_gem_noc_snoc },
};

static struct qcom_icc_node qnm_snoc_gc = {
	.name = "qnm_snoc_gc",
	.channels = 1,
	.buswidth = 8,
	.num_links = 1,
	.link_nodes = { &qns_llcc },
};

static struct qcom_icc_node qnm_snoc_sf = {
	.name = "qnm_snoc_sf",
	.channels = 1,
	.buswidth = 16,
	.num_links = 1,
	.link_nodes = { &qns_llcc },
};

static struct qcom_icc_node qxm_gpu = {
	.name = "qxm_gpu",
	.channels = 2,
	.buswidth = 32,
	.num_links = 2,
	.link_nodes = { &qns_llcc, &qns_gem_noc_snoc },
};

static struct qcom_icc_node llcc_mc = {
	.name = "llcc_mc",
	.channels = 2,
	.buswidth = 4,
	.num_links = 1,
	.link_nodes = { &ebi },
};

static struct qcom_icc_node qhm_mnoc_cfg = {
	.name = "qhm_mnoc_cfg",
	.channels = 1,
	.buswidth = 4,
	.num_links = 1,
	.link_nodes = { &srvc_mnoc },
};

static struct qcom_icc_node qxm_camnoc_hf0 = {
	.name = "qxm_camnoc_hf0",
	.channels = 2,
	.buswidth = 32,
	.num_links = 1,
	.link_nodes = { &qns_mem_noc_hf },
};

static struct qcom_icc_node qxm_camnoc_hf1 = {
	.name = "qxm_camnoc_hf1",
	.channels = 1,
	.buswidth = 32,
	.num_links = 1,
	.link_nodes = { &qns_mem_noc_hf },
};

static struct qcom_icc_node qxm_camnoc_sf = {
	.name = "qxm_camnoc_sf",
	.channels = 1,
	.buswidth = 32,
	.num_links = 1,
	.link_nodes = { &qns2_mem_noc },
};

static struct qcom_icc_node qxm_camnoc_icp = {
	.name = "qxm_camnoc_icp",
	.channels = 1,
	.buswidth = 32,
	.num_links = 1,
	.link_nodes = { &qns2_mem_noc },
};

static struct qcom_icc_node qxm_mdp0 = {
	.name = "qxm_mdp0",
	.channels = 1,
	.buswidth = 32,
	.num_links = 1,
	.link_nodes = { &qns_mem_noc_hf },
};

static struct qcom_icc_node qxm_mdp1 = {
	.name = "qxm_mdp1",
	.channels = 1,
	.buswidth = 32,
	.num_links = 1,
	.link_nodes = { &qns_mem_noc_hf },
};

static struct qcom_icc_node qxm_rot = {
	.name = "qxm_rot",
	.channels = 1,
	.buswidth = 32,
	.num_links = 1,
	.link_nodes = { &qns2_mem_noc },
};

static struct qcom_icc_node qxm_venus0 = {
	.name = "qxm_venus0",
	.channels = 1,
	.buswidth = 32,
	.num_links = 1,
	.link_nodes = { &qns2_mem_noc },
};

static struct qcom_icc_node qxm_venus1 = {
	.name = "qxm_venus1",
	.channels = 1,
	.buswidth = 32,
	.num_links = 1,
	.link_nodes = { &qns2_mem_noc },
};

static struct qcom_icc_node qxm_venus_arm9 = {
	.name = "qxm_venus_arm9",
	.channels = 1,
	.buswidth = 8,
	.num_links = 1,
	.link_nodes = { &qns2_mem_noc },
};

static struct qcom_icc_node qhm_snoc_cfg = {
	.name = "qhm_snoc_cfg",
	.channels = 1,
	.buswidth = 4,
	.num_links = 1,
	.link_nodes = { &srvc_snoc },
};

static struct qcom_icc_node qnm_aggre1_noc = {
	.name = "qnm_aggre1_noc",
	.channels = 1,
	.buswidth = 16,
	.num_links = 6,
	.link_nodes = { &qns_gemnoc_sf, &qxs_pimem, &qxs_imem, &qhs_apss, &qns_cnoc, &xs_qdss_stm },
};

static struct qcom_icc_node qnm_aggre2_noc = {
	.name = "qnm_aggre2_noc",
	.channels = 1,
	.buswidth = 16,
	.num_links = 7,
	.link_nodes = { &qns_gemnoc_sf, &qxs_pimem, &qxs_imem, &qhs_apss, &qns_cnoc, &xs_sys_tcu_cfg, &xs_qdss_stm },
};

static struct qcom_icc_node qnm_gemnoc = {
	.name = "qnm_gemnoc",
	.channels = 1,
	.buswidth = 8,
	.num_links = 6,
	.link_nodes = { &qxs_pimem, &qxs_imem, &qhs_apss, &qns_cnoc, &xs_sys_tcu_cfg, &xs_qdss_stm },
};

static struct qcom_icc_node qxm_pimem = {
	.name = "qxm_pimem",
	.channels = 1,
	.buswidth = 8,
	.num_links = 2,
	.link_nodes = { &qns_gemnoc_gc, &qxs_imem },
};

static struct qcom_icc_node xm_gic = {
	.name = "xm_gic",
	.channels = 1,
	.buswidth = 8,
	.num_links = 2,
	.link_nodes = { &qns_gemnoc_gc, &qxs_imem },
};

static struct qcom_icc_node qns_a1noc_snoc = {
	.name = "qns_a1noc_snoc",
	.channels = 1,
	.buswidth = 16,
	.num_links = 1,
	.link_nodes = { &qnm_aggre1_noc },
};

static struct qcom_icc_node srvc_aggre1_noc = {
	.name = "srvc_aggre1_noc",
	.channels = 1,
	.buswidth = 4,
};

static struct qcom_icc_node qns_a2noc_snoc = {
	.name = "qns_a2noc_snoc",
	.channels = 1,
	.buswidth = 16,
	.num_links = 1,
	.link_nodes = { &qnm_aggre2_noc },
};

static struct qcom_icc_node qns_pcie_gemnoc = {
	.name = "qns_pcie_gemnoc",
	.channels = 1,
	.buswidth = 8,
	.num_links = 1,
	.link_nodes = { &qnm_pcie },
};

static struct qcom_icc_node srvc_aggre2_noc = {
	.name = "srvc_aggre2_noc",
	.channels = 1,
	.buswidth = 4,
};

static struct qcom_icc_node qns_camnoc_uncomp = {
	.name = "qns_camnoc_uncomp",
	.channels = 1,
	.buswidth = 32,
};

static struct qcom_icc_node qns_cdsp_gemnoc = {
	.name = "qns_cdsp_gemnoc",
	.channels = 1,
	.buswidth = 32,
	.num_links = 1,
	.link_nodes = { &qnm_cmpnoc },
};

static struct qcom_icc_node qhs_a1_noc_cfg = {
	.name = "qhs_a1_noc_cfg",
	.channels = 1,
	.buswidth = 4,
	.num_links = 1,
	.link_nodes = { &qhm_a1noc_cfg },
};

static struct qcom_icc_node qhs_a2_noc_cfg = {
	.name = "qhs_a2_noc_cfg",
	.channels = 1,
	.buswidth = 4,
	.num_links = 1,
	.link_nodes = { &qhm_a2noc_cfg },
};

static struct qcom_icc_node qhs_ahb2phy_north = {
	.name = "qhs_ahb2phy_north",
	.channels = 1,
	.buswidth = 4,
};

static struct qcom_icc_node qhs_ahb2phy_south = {
	.name = "qhs_ahb2phy_south",
	.channels = 1,
	.buswidth = 4,
};

static struct qcom_icc_node qhs_ahb2phy_west = {
	.name = "qhs_ahb2phy_west",
	.channels = 1,
	.buswidth = 4,
};

static struct qcom_icc_node qhs_aop = {
	.name = "qhs_aop",
	.channels = 1,
	.buswidth = 4,
};

static struct qcom_icc_node qhs_aoss = {
	.name = "qhs_aoss",
	.channels = 1,
	.buswidth = 4,
};

static struct qcom_icc_node qhs_camera_cfg = {
	.name = "qhs_camera_cfg",
	.channels = 1,
	.buswidth = 4,
};

static struct qcom_icc_node qhs_camera_nrt_thrott_cfg = {
	.name = "qhs_camera_nrt_thrott_cfg",
	.channels = 1,
	.buswidth = 4,
};

static struct qcom_icc_node qhs_camera_rt_throttle_cfg = {
	.name = "qhs_camera_rt_throttle_cfg",
	.channels = 1,
	.buswidth = 4,
};

static struct qcom_icc_node qhs_clk_ctl = {
	.name = "qhs_clk_ctl",
	.channels = 1,
	.buswidth = 4,
};

static struct qcom_icc_node qhs_compute_dsp_cfg = {
	.name = "qhs_compute_dsp_cfg",
	.channels = 1,
	.buswidth = 4,
};

static struct qcom_icc_node qhs_cpr_cx = {
	.name = "qhs_cpr_cx",
	.channels = 1,
	.buswidth = 4,
};

static struct qcom_icc_node qhs_cpr_mx = {
	.name = "qhs_cpr_mx",
	.channels = 1,
	.buswidth = 4,
};

static struct qcom_icc_node qhs_crypto0_cfg = {
	.name = "qhs_crypto0_cfg",
	.channels = 1,
	.buswidth = 4,
};

static struct qcom_icc_node qhs_ddrss_cfg = {
	.name = "qhs_ddrss_cfg",
	.channels = 1,
	.buswidth = 4,
	.num_links = 1,
	.link_nodes = { &qhm_cnoc_dc_noc },
};

static struct qcom_icc_node qhs_display_cfg = {
	.name = "qhs_display_cfg",
	.channels = 1,
	.buswidth = 4,
};

static struct qcom_icc_node qhs_display_throttle_cfg = {
	.name = "qhs_display_throttle_cfg",
	.channels = 1,
	.buswidth = 4,
};

static struct qcom_icc_node qhs_emmc_cfg = {
	.name = "qhs_emmc_cfg",
	.channels = 1,
	.buswidth = 4,
};

static struct qcom_icc_node qhs_glm = {
	.name = "qhs_glm",
	.channels = 1,
	.buswidth = 4,
};

static struct qcom_icc_node qhs_gpuss_cfg = {
	.name = "qhs_gpuss_cfg",
	.channels = 1,
	.buswidth = 8,
};

static struct qcom_icc_node qhs_imem_cfg = {
	.name = "qhs_imem_cfg",
	.channels = 1,
	.buswidth = 4,
};

static struct qcom_icc_node qhs_ipa = {
	.name = "qhs_ipa",
	.channels = 1,
	.buswidth = 4,
};

static struct qcom_icc_node qhs_mnoc_cfg = {
	.name = "qhs_mnoc_cfg",
	.channels = 1,
	.buswidth = 4,
	.num_links = 1,
	.link_nodes = { &qhm_mnoc_cfg },
};

static struct qcom_icc_node qhs_pcie_cfg = {
	.name = "qhs_pcie_cfg",
	.channels = 1,
	.buswidth = 4,
};

static struct qcom_icc_node qhs_pdm = {
	.name = "qhs_pdm",
	.channels = 1,
	.buswidth = 4,
};

static struct qcom_icc_node qhs_pimem_cfg = {
	.name = "qhs_pimem_cfg",
	.channels = 1,
	.buswidth = 4,
};

static struct qcom_icc_node qhs_prng = {
	.name = "qhs_prng",
	.channels = 1,
	.buswidth = 4,
};

static struct qcom_icc_node qhs_qdss_cfg = {
	.name = "qhs_qdss_cfg",
	.channels = 1,
	.buswidth = 4,
};

static struct qcom_icc_node qhs_qupv3_center = {
	.name = "qhs_qupv3_center",
	.channels = 1,
	.buswidth = 4,
};

static struct qcom_icc_node qhs_qupv3_north = {
	.name = "qhs_qupv3_north",
	.channels = 1,
	.buswidth = 4,
};

static struct qcom_icc_node qhs_qup_core_0 = {
	.name = "qhs_qup_core_0",
	.channels = 1,
	.buswidth = 4,
};

static struct qcom_icc_node qhs_qup_core_1 = {
	.name = "qhs_qup_core_1",
	.channels = 1,
	.buswidth = 4,
};

static struct qcom_icc_node qhs_sdc2 = {
	.name = "qhs_sdc2",
	.channels = 1,
	.buswidth = 4,
};

static struct qcom_icc_node qhs_sdc4 = {
	.name = "qhs_sdc4",
	.channels = 1,
	.buswidth = 4,
};

static struct qcom_icc_node qhs_snoc_cfg = {
	.name = "qhs_snoc_cfg",
	.channels = 1,
	.buswidth = 4,
	.num_links = 1,
	.link_nodes = { &qhm_snoc_cfg },
};

static struct qcom_icc_node qhs_spdm = {
	.name = "qhs_spdm",
	.channels = 1,
	.buswidth = 4,
};

static struct qcom_icc_node qhs_tcsr = {
	.name = "qhs_tcsr",
	.channels = 1,
	.buswidth = 4,
};

static struct qcom_icc_node qhs_tlmm_north = {
	.name = "qhs_tlmm_north",
	.channels = 1,
	.buswidth = 4,
};

static struct qcom_icc_node qhs_tlmm_south = {
	.name = "qhs_tlmm_south",
	.channels = 1,
	.buswidth = 4,
};

static struct qcom_icc_node qhs_tlmm_west = {
	.name = "qhs_tlmm_west",
	.channels = 1,
	.buswidth = 4,
};

static struct qcom_icc_node qhs_tsif = {
	.name = "qhs_tsif",
	.channels = 1,
	.buswidth = 4,
};

static struct qcom_icc_node qhs_ufs_mem_cfg = {
	.name = "qhs_ufs_mem_cfg",
	.channels = 1,
	.buswidth = 4,
};

static struct qcom_icc_node qhs_usb3_0 = {
	.name = "qhs_usb3_0",
	.channels = 1,
	.buswidth = 4,
};

static struct qcom_icc_node qhs_venus_cfg = {
	.name = "qhs_venus_cfg",
	.channels = 1,
	.buswidth = 4,
};

static struct qcom_icc_node qhs_venus_cvp_throttle_cfg = {
	.name = "qhs_venus_cvp_throttle_cfg",
	.channels = 1,
	.buswidth = 4,
};

static struct qcom_icc_node qhs_venus_throttle_cfg = {
	.name = "qhs_venus_throttle_cfg",
	.channels = 1,
	.buswidth = 4,
};

static struct qcom_icc_node qhs_vsense_ctrl_cfg = {
	.name = "qhs_vsense_ctrl_cfg",
	.channels = 1,
	.buswidth = 4,
};

static struct qcom_icc_node qns_cnoc_a2noc = {
	.name = "qns_cnoc_a2noc",
	.channels = 1,
	.buswidth = 8,
	.num_links = 1,
	.link_nodes = { &qnm_cnoc },
};

static struct qcom_icc_node srvc_cnoc = {
	.name = "srvc_cnoc",
	.channels = 1,
	.buswidth = 4,
};

static struct qcom_icc_node qhs_gemnoc = {
	.name = "qhs_gemnoc",
	.channels = 1,
	.buswidth = 4,
	.num_links = 1,
	.link_nodes = { &qhm_gemnoc_cfg },
};

static struct qcom_icc_node qhs_llcc = {
	.name = "qhs_llcc",
	.channels = 1,
	.buswidth = 4,
};

static struct qcom_icc_node qhs_mdsp_ms_mpu_cfg = {
	.name = "qhs_mdsp_ms_mpu_cfg",
	.channels = 1,
	.buswidth = 4,
};

static struct qcom_icc_node qns_gem_noc_snoc = {
	.name = "qns_gem_noc_snoc",
	.channels = 1,
	.buswidth = 8,
	.num_links = 1,
	.link_nodes = { &qnm_gemnoc },
};

static struct qcom_icc_node qns_llcc = {
	.name = "qns_llcc",
	.channels = 2,
	.buswidth = 16,
	.num_links = 1,
	.link_nodes = { &llcc_mc },
};

static struct qcom_icc_node srvc_gemnoc = {
	.name = "srvc_gemnoc",
	.channels = 1,
	.buswidth = 4,
};

static struct qcom_icc_node ebi = {
	.name = "ebi",
	.channels = 2,
	.buswidth = 4,
};

static struct qcom_icc_node qns2_mem_noc = {
	.name = "qns2_mem_noc",
	.channels = 1,
	.buswidth = 32,
	.num_links = 1,
	.link_nodes = { &qnm_mnoc_sf },
};

static struct qcom_icc_node qns_mem_noc_hf = {
	.name = "qns_mem_noc_hf",
	.channels = 2,
	.buswidth = 32,
	.num_links = 1,
	.link_nodes = { &qnm_mnoc_hf },
};

static struct qcom_icc_node srvc_mnoc = {
	.name = "srvc_mnoc",
	.channels = 1,
	.buswidth = 4,
};

static struct qcom_icc_node qhs_apss = {
	.name = "qhs_apss",
	.channels = 1,
	.buswidth = 8,
};

static struct qcom_icc_node qns_cnoc = {
	.name = "qns_cnoc",
	.channels = 1,
	.buswidth = 8,
	.num_links = 1,
	.link_nodes = { &qnm_snoc },
};

static struct qcom_icc_node qns_gemnoc_gc = {
	.name = "qns_gemnoc_gc",
	.channels = 1,
	.buswidth = 8,
	.num_links = 1,
	.link_nodes = { &qnm_snoc_gc },
};

static struct qcom_icc_node qns_gemnoc_sf = {
	.name = "qns_gemnoc_sf",
	.channels = 1,
	.buswidth = 16,
	.num_links = 1,
	.link_nodes = { &qnm_snoc_sf },
};

static struct qcom_icc_node qxs_imem = {
	.name = "qxs_imem",
	.channels = 1,
	.buswidth = 8,
};

static struct qcom_icc_node qxs_pimem = {
	.name = "qxs_pimem",
	.channels = 1,
	.buswidth = 8,
};

static struct qcom_icc_node srvc_snoc = {
	.name = "srvc_snoc",
	.channels = 1,
	.buswidth = 4,
};

static struct qcom_icc_node xs_qdss_stm = {
	.name = "xs_qdss_stm",
	.channels = 1,
	.buswidth = 4,
};

static struct qcom_icc_node xs_sys_tcu_cfg = {
	.name = "xs_sys_tcu_cfg",
	.channels = 1,
	.buswidth = 8,
};

/*
 * IPA Virtual NOC nodes
 */
static struct qcom_icc_node qhm_ipa_virt = {
	.name = "qhm_ipa_virt",
	.channels = 1,
	.buswidth = 8,
	.num_links = 1,
	.link_nodes = { &qns_ipa_virt },
};

static struct qcom_icc_node qns_ipa_virt = {
	.name = "qns_ipa_virt",
	.channels = 1,
	.buswidth = 8,
};

static struct qcom_icc_bcm bcm_acv = {
	.name = "ACV",
	.enable_mask = BIT(3),
	.keepalive = false,
	.num_nodes = 1,
	.nodes = { &ebi },
};

static struct qcom_icc_bcm bcm_mc0 = {
	.name = "MC0",
	.keepalive = true,
	.num_nodes = 1,
	.nodes = { &ebi },
};

static struct qcom_icc_bcm bcm_sh0 = {
	.name = "SH0",
	.keepalive = true,
	.num_nodes = 1,
	.nodes = { &qns_llcc },
};

static struct qcom_icc_bcm bcm_mm0 = {
	.name = "MM0",
	.keepalive = true,
	.num_nodes = 1,
	.nodes = { &qns_mem_noc_hf },
};

static struct qcom_icc_bcm bcm_mm1 = {
	.name = "MM1",
	.keepalive = true,
	.num_nodes = 8,
	.nodes = { &qxm_camnoc_hf0_uncomp,
		   &qxm_camnoc_hf1_uncomp,
		   &qxm_camnoc_sf_uncomp,
		   &qxm_camnoc_icp_uncomp,
		   &qxm_camnoc_hf0,
		   &qxm_camnoc_hf1,
		   &qxm_mdp0,
		   &qxm_mdp1
	},
};

static struct qcom_icc_bcm bcm_sh2 = {
	.name = "SH2",
	.keepalive = false,
	.num_nodes = 1,
	.nodes = { &qns_gem_noc_snoc },
};

static struct qcom_icc_bcm bcm_sh3 = {
	.name = "SH3",
	.keepalive = false,
	.num_nodes = 1,
	.nodes = { &acm_sys_tcu },
};

static struct qcom_icc_bcm bcm_mm2 = {
	.name = "MM2",
	.keepalive = false,
	.num_nodes = 2,
	.nodes = { &qxm_camnoc_icp,
		   &qns2_mem_noc
	},
};

static struct qcom_icc_bcm bcm_mm3 = {
	.name = "MM3",
	.keepalive = false,
	.num_nodes = 5,
	.nodes = { &qxm_camnoc_sf,
		   &qxm_rot,
		   &qxm_venus0,
		   &qxm_venus1,
		   &qxm_venus_arm9
	},
};

static struct qcom_icc_bcm bcm_sh5 = {
	.name = "SH5",
	.keepalive = false,
	.num_nodes = 1,
	.nodes = { &acm_apps },
};

static struct qcom_icc_bcm bcm_sn0 = {
	.name = "SN0",
	.keepalive = true,
	.num_nodes = 1,
	.nodes = { &qns_gemnoc_sf },
};

static struct qcom_icc_bcm bcm_ce0 = {
	.name = "CE0",
	.keepalive = false,
	.num_nodes = 1,
	.nodes = { &qxm_crypto },
};

static struct qcom_icc_bcm bcm_cn0 = {
	.name = "CN0",
	.keepalive = true,
	.num_nodes = 56,
	.nodes = { &qhm_tsif,
		   &xm_emmc,
		   &xm_sdc2,
		   &xm_sdc4,
		   &qhm_spdm,
		   &qnm_snoc,
		   &qhs_a1_noc_cfg,
		   &qhs_a2_noc_cfg,
		   &qhs_ahb2phy_north,
		   &qhs_ahb2phy_south,
		   &qhs_ahb2phy_west,
		   &qhs_aop,
		   &qhs_aoss,
		   &qhs_camera_cfg,
		   &qhs_camera_nrt_thrott_cfg,
		   &qhs_camera_rt_throttle_cfg,
		   &qhs_clk_ctl,
		   &qhs_compute_dsp_cfg,
		   &qhs_cpr_cx,
		   &qhs_cpr_mx,
		   &qhs_crypto0_cfg,
		   &qhs_ddrss_cfg,
		   &qhs_display_cfg,
		   &qhs_display_throttle_cfg,
		   &qhs_emmc_cfg,
		   &qhs_glm,
		   &qhs_gpuss_cfg,
		   &qhs_imem_cfg,
		   &qhs_ipa,
		   &qhs_mnoc_cfg,
		   &qhs_pcie_cfg,
		   &qhs_pdm,
		   &qhs_pimem_cfg,
		   &qhs_prng,
		   &qhs_qdss_cfg,
		   &qhs_qupv3_center,
		   &qhs_qupv3_north,
		   &qhs_qup_core_0,
		   &qhs_qup_core_1,
		   &qhs_sdc2,
		   &qhs_sdc4,
		   &qhs_snoc_cfg,
		   &qhs_spdm,
		   &qhs_tcsr,
		   &qhs_tlmm_north,
		   &qhs_tlmm_south,
		   &qhs_tlmm_west,
		   &qhs_tsif,
		   &qhs_ufs_mem_cfg,
		   &qhs_usb3_0,
		   &qhs_venus_cfg,
		   &qhs_venus_cvp_throttle_cfg,
		   &qhs_venus_throttle_cfg,
		   &qhs_vsense_ctrl_cfg,
		   &qns_cnoc_a2noc,
		   &srvc_cnoc
	},
};

static struct qcom_icc_bcm bcm_qup0 = {
	.name = "QUP0",
	.keepalive = false,
	.num_nodes = 2,
	.nodes = { &qhm_qup_center,
		   &qhm_qup_north
	},
};

static struct qcom_icc_bcm bcm_sn1 = {
	.name = "SN1",
	.keepalive = false,
	.num_nodes = 1,
	.nodes = { &qxs_imem },
};

static struct qcom_icc_bcm bcm_sn2 = {
	.name = "SN2",
	.keepalive = false,
	.num_nodes = 1,
	.nodes = { &qns_gemnoc_gc },
};

static struct qcom_icc_bcm bcm_sn4 = {
	.name = "SN4",
	.keepalive = false,
	.num_nodes = 1,
	.nodes = { &qxs_pimem },
};

static struct qcom_icc_bcm bcm_sn9 = {
	.name = "SN9",
	.keepalive = false,
	.num_nodes = 2,
	.nodes = { &qnm_aggre1_noc,
		   &qns_a1noc_snoc
	},
};

/* lito 裁剪: SN11/SN12/SN14/SN15 定义已删（cmd-db 无此资源） */

static struct qcom_icc_bcm * const aggre1_noc_bcms[] = {
	&bcm_cn0,
	&bcm_qup0,
	&bcm_sn9,
};

static struct qcom_icc_node * const aggre1_noc_nodes[] = {
	&qhm_a1noc_cfg,
	&qhm_qup_center,
	&qhm_tsif,
	&xm_emmc,
	&xm_sdc2,
	&xm_sdc4,
	&xm_ufs_mem,
	&qns_a1noc_snoc,
	&srvc_aggre1_noc,
};

static const struct qcom_icc_desc sm7250_aggre1_noc = {
	.nodes = aggre1_noc_nodes,
	.num_nodes = ARRAY_SIZE(aggre1_noc_nodes),
	.bcms = aggre1_noc_bcms,
	.num_bcms = ARRAY_SIZE(aggre1_noc_bcms),
};

/* lito 裁剪: SN11/SN14 不在本机 cmd-db（照抄 kona 的幻影资源,
 * addr=0 投票会冲坏 RPMh——迭代 19 实测复位），从表中移除 */
static struct qcom_icc_bcm * const aggre2_noc_bcms[] = {
	&bcm_ce0,
	&bcm_qup0,
};

static struct qcom_icc_node * const aggre2_noc_nodes[] = {
	&qhm_a2noc_cfg,
	&qhm_qdss_bam,
	&qhm_qup_north,
	&qnm_cnoc,
	&qxm_crypto,
	&qxm_ipa,
	&xm_pcie3_0,
	&xm_qdss_etr,
	&xm_usb3_0,
	&qns_a2noc_snoc,
	&qns_pcie_gemnoc,
	&srvc_aggre2_noc,
};

static const struct qcom_icc_desc sm7250_aggre2_noc = {
	.nodes = aggre2_noc_nodes,
	.num_nodes = ARRAY_SIZE(aggre2_noc_nodes),
	.bcms = aggre2_noc_bcms,
	.num_bcms = ARRAY_SIZE(aggre2_noc_bcms),
};

static struct qcom_icc_bcm * const camnoc_virt_bcms[] = {
	&bcm_mm1,
};

static struct qcom_icc_node * const camnoc_virt_nodes[] = {
	&qxm_camnoc_hf0_uncomp,
	&qxm_camnoc_hf1_uncomp,
	&qxm_camnoc_sf_uncomp,
	&qxm_camnoc_icp_uncomp,
	&qns_camnoc_uncomp,
};

static const struct qcom_icc_desc sm7250_camnoc_virt = {
	.nodes = camnoc_virt_nodes,
	.num_nodes = ARRAY_SIZE(camnoc_virt_nodes),
	.bcms = camnoc_virt_bcms,
	.num_bcms = ARRAY_SIZE(camnoc_virt_bcms),
};

/* lito 裁剪: SH8/SH10(=CDSP/NPU)不在本机 cmd-db, 见 aggre2 注释 */
static struct qcom_icc_bcm * const compute_noc_bcms[] = {
};

static struct qcom_icc_node * const compute_noc_nodes[] = {
	&qnm_npu,
	&qnm_npu_dsp,
	&qns_cdsp_gemnoc,
};

static const struct qcom_icc_desc sm7250_compute_noc = {
	.nodes = compute_noc_nodes,
	.num_nodes = ARRAY_SIZE(compute_noc_nodes),
	.bcms = compute_noc_bcms,
	.num_bcms = ARRAY_SIZE(compute_noc_bcms),
};

static struct qcom_icc_bcm * const config_noc_bcms[] = {
	&bcm_cn0,
};

static struct qcom_icc_node * const config_noc_nodes[] = {
	&qhm_spdm,
	&qnm_snoc,
	&xm_qdss_dap,
	&qhs_a1_noc_cfg,
	&qhs_a2_noc_cfg,
	&qhs_ahb2phy_north,
	&qhs_ahb2phy_south,
	&qhs_ahb2phy_west,
	&qhs_aop,
	&qhs_aoss,
	&qhs_camera_cfg,
	&qhs_camera_nrt_thrott_cfg,
	&qhs_camera_rt_throttle_cfg,
	&qhs_clk_ctl,
	&qhs_compute_dsp_cfg,
	&qhs_cpr_cx,
	&qhs_cpr_mx,
	&qhs_crypto0_cfg,
	&qhs_ddrss_cfg,
	&qhs_display_cfg,
	&qhs_display_throttle_cfg,
	&qhs_emmc_cfg,
	&qhs_glm,
	&qhs_gpuss_cfg,
	&qhs_imem_cfg,
	&qhs_ipa,
	&qhs_mnoc_cfg,
	&qhs_pcie_cfg,
	&qhs_pdm,
	&qhs_pimem_cfg,
	&qhs_prng,
	&qhs_qdss_cfg,
	&qhs_qupv3_center,
	&qhs_qupv3_north,
	&qhs_qup_core_0,
	&qhs_qup_core_1,
	&qhs_sdc2,
	&qhs_sdc4,
	&qhs_snoc_cfg,
	&qhs_spdm,
	&qhs_tcsr,
	&qhs_tlmm_north,
	&qhs_tlmm_south,
	&qhs_tlmm_west,
	&qhs_tsif,
	&qhs_ufs_mem_cfg,
	&qhs_usb3_0,
	&qhs_venus_cfg,
	&qhs_venus_cvp_throttle_cfg,
	&qhs_venus_throttle_cfg,
	&qhs_vsense_ctrl_cfg,
	&qns_cnoc_a2noc,
	&srvc_cnoc,
};

static const struct qcom_icc_desc sm7250_config_noc = {
	.nodes = config_noc_nodes,
	.num_nodes = ARRAY_SIZE(config_noc_nodes),
	.bcms = config_noc_bcms,
	.num_bcms = ARRAY_SIZE(config_noc_bcms),
};

static struct qcom_icc_bcm * const dc_noc_bcms[] = {
};

static struct qcom_icc_node * const dc_noc_nodes[] = {
	&qhm_cnoc_dc_noc,
	&qhs_gemnoc,
	&qhs_llcc,
};

static const struct qcom_icc_desc sm7250_dc_noc = {
	.nodes = dc_noc_nodes,
	.num_nodes = ARRAY_SIZE(dc_noc_nodes),
	.bcms = dc_noc_bcms,
	.num_bcms = ARRAY_SIZE(dc_noc_bcms),
};

static struct qcom_icc_bcm * const gem_noc_bcms[] = {
	&bcm_sh0,
	&bcm_sh2,
	&bcm_sh3,
	&bcm_sh5,
};

static struct qcom_icc_node * const gem_noc_nodes[] = {
	&acm_apps,
	&acm_sys_tcu,
	&qhm_gemnoc_cfg,
	&qnm_cmpnoc,
	&qnm_mnoc_hf,
	&qnm_mnoc_sf,
	&qnm_pcie,
	&qnm_snoc_gc,
	&qnm_snoc_sf,
	&qxm_gpu,
	&qhs_mdsp_ms_mpu_cfg,
	&qns_gem_noc_snoc,
	&qns_llcc,
	&srvc_gemnoc,
};

static const struct qcom_icc_desc sm7250_gem_noc = {
	.nodes = gem_noc_nodes,
	.num_nodes = ARRAY_SIZE(gem_noc_nodes),
	.bcms = gem_noc_bcms,
	.num_bcms = ARRAY_SIZE(gem_noc_bcms),
};

static struct qcom_icc_bcm * const ipa_virt_bcms[] = {
};

static struct qcom_icc_node * const ipa_virt_nodes[] = {
	&qhm_ipa_virt,
	&qns_ipa_virt,
};

static const struct qcom_icc_desc sm7250_ipa_virt = {
	.nodes = ipa_virt_nodes,
	.num_nodes = ARRAY_SIZE(ipa_virt_nodes),
	.bcms = ipa_virt_bcms,
	.num_bcms = ARRAY_SIZE(ipa_virt_bcms),
};

static struct qcom_icc_bcm * const mc_virt_bcms[] = {
	&bcm_acv,
	&bcm_mc0,
};

static struct qcom_icc_node * const mc_virt_nodes[] = {
	&llcc_mc,
	&ebi,
};

static const struct qcom_icc_desc sm7250_mc_virt = {
	.nodes = mc_virt_nodes,
	.num_nodes = ARRAY_SIZE(mc_virt_nodes),
	.bcms = mc_virt_bcms,
	.num_bcms = ARRAY_SIZE(mc_virt_bcms),
};

static struct qcom_icc_bcm * const mmss_noc_bcms[] = {
	&bcm_mm0,
	&bcm_mm1,
	&bcm_mm2,
	&bcm_mm3,
};

static struct qcom_icc_node * const mmss_noc_nodes[] = {
	&qhm_mnoc_cfg,
	&qxm_camnoc_hf0,
	&qxm_camnoc_hf1,
	&qxm_camnoc_sf,
	&qxm_camnoc_icp,
	&qxm_mdp0,
	&qxm_mdp1,
	&qxm_rot,
	&qxm_venus0,
	&qxm_venus1,
	&qxm_venus_arm9,
	&qns2_mem_noc,
	&qns_mem_noc_hf,
	&srvc_mnoc,
};

static const struct qcom_icc_desc sm7250_mmss_noc = {
	.nodes = mmss_noc_nodes,
	.num_nodes = ARRAY_SIZE(mmss_noc_nodes),
	.bcms = mmss_noc_bcms,
	.num_bcms = ARRAY_SIZE(mmss_noc_bcms),
};

/* lito 裁剪: SN11/SN12/SN15 不在本机 cmd-db, 见 aggre2 注释;
 * 移除后经由这些节点的路径仍可建(图节点保留), 只是该跳无带宽投票 */
static struct qcom_icc_bcm * const system_noc_bcms[] = {
	&bcm_sn0,
	&bcm_sn1,
	&bcm_sn2,
	&bcm_sn4,
	&bcm_sn9,
};

static struct qcom_icc_node * const system_noc_nodes[] = {
	&qhm_snoc_cfg,
	&qnm_aggre1_noc,
	&qnm_aggre2_noc,
	&qnm_gemnoc,
	&qxm_pimem,
	&xm_gic,
	&qhs_apss,
	&qns_cnoc,
	&qns_gemnoc_gc,
	&qns_gemnoc_sf,
	&qxs_imem,
	&qxs_pimem,
	&srvc_snoc,
	&xs_qdss_stm,
	&xs_sys_tcu_cfg,
};

static const struct qcom_icc_desc sm7250_system_noc = {
	.nodes = system_noc_nodes,
	.num_nodes = ARRAY_SIZE(system_noc_nodes),
	.bcms = system_noc_bcms,
	.num_bcms = ARRAY_SIZE(system_noc_bcms),
};

static const struct of_device_id qnoc_of_match[] = {
	{ .compatible = "qcom,sm7250-aggre1-noc", .data = &sm7250_aggre1_noc },
	{ .compatible = "qcom,sm7250-aggre2-noc", .data = &sm7250_aggre2_noc },
	{ .compatible = "qcom,sm7250-camnoc-virt", .data = &sm7250_camnoc_virt },
	{ .compatible = "qcom,sm7250-compute-noc", .data = &sm7250_compute_noc },
	{ .compatible = "qcom,sm7250-config-noc", .data = &sm7250_config_noc },
	{ .compatible = "qcom,sm7250-dc-noc", .data = &sm7250_dc_noc },
	{ .compatible = "qcom,sm7250-gem-noc", .data = &sm7250_gem_noc },
	{ .compatible = "qcom,sm7250-ipa-virt", .data = &sm7250_ipa_virt },
	{ .compatible = "qcom,sm7250-mc-virt", .data = &sm7250_mc_virt },
	{ .compatible = "qcom,sm7250-mmss-noc", .data = &sm7250_mmss_noc },
	{ .compatible = "qcom,sm7250-system-noc", .data = &sm7250_system_noc },
	{ }
};
MODULE_DEVICE_TABLE(of, qnoc_of_match);

static struct platform_driver qnoc_driver = {
	.probe = qcom_icc_rpmh_probe,
	.remove = qcom_icc_rpmh_remove,
	.driver = {
		.name = "qnoc-sm7250",
		.of_match_table = qnoc_of_match,
		/*
		 * picasso bring-up: 无 .sync_state（迭代 31）。
		 * 当前 DT 里 qnoc 的唯一消费者是 ufshc；icc_sync_state 的全局
		 * 屏障在第 11 个 provider 同步时把所有节点的 init(INT_MAX) 票
		 * 清零，非 ufshc 路径的 BCM（MM2/MM3/QUP0/SH2/SH3/SH5/ACV/
		 * CE0/SN1/SN2/SN4…，多数 keepalive=false）被 valid=0 显式禁用
		 * → fabric 断供整机死机（迭代 30 实测 1.78s 死机重启）。
		 * 保留 init 票恒为 INT_MAX = 与 cmdline 的 clk/pd/
		 * regulator_ignore_unused 同族的第四个 bring-up 保活参数。
		 * 将来显示/USB/CPU 等真实消费者接齐后恢复。
		 */
	},
};

static int __init qnoc_driver_init(void)
{
	return platform_driver_register(&qnoc_driver);
}
core_initcall(qnoc_driver_init);

static void __exit qnoc_driver_exit(void)
{
	platform_driver_unregister(&qnoc_driver);
}
module_exit(qnoc_driver_exit);

MODULE_DESCRIPTION("Qualcomm SM7250 NoC driver");
MODULE_LICENSE("GPL");
