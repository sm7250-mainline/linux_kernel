// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2018, The Linux Foundation. All rights reserved.
 */

#include <linux/bitmap.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/pinctrl/pinctrl.h>

#include "pinctrl-msm.h"

/*
 * lito TLMM 为单一映射区域（下游 dts: reg = <0xf000000 0x1000000>，无 tiles）。
 * 此前照抄 sc7180 的 north/south 双 tile 模型，msm_pinctrl_probe 会走
 * platform_get_resource_byname 分支，而 DT 只有无名 reg → "invalid resource
 * (null)" -EINVAL，tlmm 永远 probe 失败（串口 pinctrl 因此不可用）。
 * 全部 pin group 固定 tile=0、按 index 0 取 reg，与下游偏移（0x1000*id）一致。
 */

#define REG_SIZE 0x1000

/* lito TLMM 为 4-tile 布局（下游 pinctrl-lito.c 原值：NORTH=0 / SOUTH=0x100000 /
 * WEST=0x500000 / EAST=0x900000）。采用下游同款"单段映射 + tile 偏移并入寄存器
 * 偏移"的访问方式（.tile 恒 0、soc_data 不设 tiles）——DT 用原厂单段
 * reg = <0xf000000 0x1000000>，避免推断各 tile 独立基址（实测 east 段推断值
 * 写挂）。 */
#define NORTH	0x0
#define SOUTH	0x00100000
#define WEST	0x00500000
#define EAST	0x00900000

#define PINGROUP(id, _tile, f1, f2, f3, f4, f5, f6, f7, f8, f9) \
	{						\
		.grp = PINCTRL_PINGROUP("gpio" #id, 	\
			gpio##id##_pins, 		\
			ARRAY_SIZE(gpio##id##_pins)),	\
		.funcs = (int[]){			\
			msm_mux_gpio, /* gpio mode */	\
			msm_mux_##f1,			\
			msm_mux_##f2,			\
			msm_mux_##f3,			\
			msm_mux_##f4,			\
			msm_mux_##f5,			\
			msm_mux_##f6,			\
			msm_mux_##f7,			\
			msm_mux_##f8,			\
			msm_mux_##f9			\
		},					\
		.nfuncs = 10,				\
		.ctl_reg = _tile + REG_SIZE * id,	\
		.io_reg = _tile + 0x4 + REG_SIZE * id,	\
		.intr_cfg_reg = _tile + 0x8 + REG_SIZE * id,	\
		.intr_status_reg = _tile + 0xc + REG_SIZE * id,	\
		.intr_target_reg = _tile + 0x8 + REG_SIZE * id,	\
		.tile = 0,				\
		.mux_bit = 2,				\
		.pull_bit = 0,				\
		.drv_bit = 6,				\
		.oe_bit = 9,				\
		.in_bit = 0,				\
		.out_bit = 1,				\
		.intr_enable_bit = 0,			\
		.intr_status_bit = 0,			\
		.intr_target_bit = 5,			\
		.intr_target_kpss_val = 3,		\
		.intr_raw_status_bit = 4,		\
		.intr_polarity_bit = 1,			\
		.intr_detection_bit = 2,		\
		.intr_detection_width = 2,		\
	}

#define SDC_QDSD_PINGROUP(pg_name, _tile, ctl, pull, drv) \
	{						\
		.grp = PINCTRL_PINGROUP(#pg_name, 	\
			pg_name##_pins, 		\
			ARRAY_SIZE(pg_name##_pins)),	\
		.ctl_reg = _tile + ctl,			\
		.io_reg = 0,				\
		.intr_cfg_reg = 0,			\
		.intr_status_reg = 0,			\
		.intr_target_reg = 0,			\
		.tile = 0,				\
		.mux_bit = -1,				\
		.pull_bit = pull,			\
		.drv_bit = drv,			\
		.oe_bit = -1,				\
		.in_bit = -1,				\
		.out_bit = -1,				\
		.intr_enable_bit = -1,			\
		.intr_status_bit = -1,			\
		.intr_target_bit = -1,			\
		.intr_raw_status_bit = -1,		\
		.intr_polarity_bit = -1,		\
		.intr_detection_bit = -1,		\
		.intr_detection_width = -1,		\
	}

#define UFS_RESET(pg_name, _tile, offset) \
	{						\
		.grp = PINCTRL_PINGROUP(#pg_name, 	\
			pg_name##_pins, 		\
			ARRAY_SIZE(pg_name##_pins)),	\
		.ctl_reg = _tile + offset,		\
		.io_reg = _tile + offset + 0x4,		\
		.intr_cfg_reg = 0,			\
		.intr_status_reg = 0,			\
		.intr_target_reg = 0,			\
		.tile = 0,				\
		.mux_bit = -1,				\
		.pull_bit = 3,				\
		.drv_bit = 0,				\
		.oe_bit = -1,				\
		.in_bit = -1,				\
		.out_bit = 0,				\
		.intr_enable_bit = -1,			\
		.intr_status_bit = -1,			\
		.intr_target_bit = -1,			\
		.intr_raw_status_bit = -1,		\
		.intr_polarity_bit = -1,		\
		.intr_detection_bit = -1,		\
		.intr_detection_width = -1,		\
	}


static const struct pinctrl_pin_desc sm7250_pins[] = {
	PINCTRL_PIN(0, "GPIO_0"),
	PINCTRL_PIN(1, "GPIO_1"),
	PINCTRL_PIN(2, "GPIO_2"),
	PINCTRL_PIN(3, "GPIO_3"),
	PINCTRL_PIN(4, "GPIO_4"),
	PINCTRL_PIN(5, "GPIO_5"),
	PINCTRL_PIN(6, "GPIO_6"),
	PINCTRL_PIN(7, "GPIO_7"),
	PINCTRL_PIN(8, "GPIO_8"),
	PINCTRL_PIN(9, "GPIO_9"),
	PINCTRL_PIN(10, "GPIO_10"),
	PINCTRL_PIN(11, "GPIO_11"),
	PINCTRL_PIN(12, "GPIO_12"),
	PINCTRL_PIN(13, "GPIO_13"),
	PINCTRL_PIN(14, "GPIO_14"),
	PINCTRL_PIN(15, "GPIO_15"),
	PINCTRL_PIN(16, "GPIO_16"),
	PINCTRL_PIN(17, "GPIO_17"),
	PINCTRL_PIN(18, "GPIO_18"),
	PINCTRL_PIN(19, "GPIO_19"),
	PINCTRL_PIN(20, "GPIO_20"),
	PINCTRL_PIN(21, "GPIO_21"),
	PINCTRL_PIN(22, "GPIO_22"),
	PINCTRL_PIN(23, "GPIO_23"),
	PINCTRL_PIN(24, "GPIO_24"),
	PINCTRL_PIN(25, "GPIO_25"),
	PINCTRL_PIN(26, "GPIO_26"),
	PINCTRL_PIN(27, "GPIO_27"),
	PINCTRL_PIN(28, "GPIO_28"),
	PINCTRL_PIN(29, "GPIO_29"),
	PINCTRL_PIN(30, "GPIO_30"),
	PINCTRL_PIN(31, "GPIO_31"),
	PINCTRL_PIN(32, "GPIO_32"),
	PINCTRL_PIN(33, "GPIO_33"),
	PINCTRL_PIN(34, "GPIO_34"),
	PINCTRL_PIN(35, "GPIO_35"),
	PINCTRL_PIN(36, "GPIO_36"),
	PINCTRL_PIN(37, "GPIO_37"),
	PINCTRL_PIN(38, "GPIO_38"),
	PINCTRL_PIN(39, "GPIO_39"),
	PINCTRL_PIN(40, "GPIO_40"),
	PINCTRL_PIN(41, "GPIO_41"),
	PINCTRL_PIN(42, "GPIO_42"),
	PINCTRL_PIN(43, "GPIO_43"),
	PINCTRL_PIN(44, "GPIO_44"),
	PINCTRL_PIN(45, "GPIO_45"),
	PINCTRL_PIN(46, "GPIO_46"),
	PINCTRL_PIN(47, "GPIO_47"),
	PINCTRL_PIN(48, "GPIO_48"),
	PINCTRL_PIN(49, "GPIO_49"),
	PINCTRL_PIN(50, "GPIO_50"),
	PINCTRL_PIN(51, "GPIO_51"),
	PINCTRL_PIN(52, "GPIO_52"),
	PINCTRL_PIN(53, "GPIO_53"),
	PINCTRL_PIN(54, "GPIO_54"),
	PINCTRL_PIN(55, "GPIO_55"),
	PINCTRL_PIN(56, "GPIO_56"),
	PINCTRL_PIN(57, "GPIO_57"),
	PINCTRL_PIN(58, "GPIO_58"),
	PINCTRL_PIN(59, "GPIO_59"),
	PINCTRL_PIN(60, "GPIO_60"),
	PINCTRL_PIN(61, "GPIO_61"),
	PINCTRL_PIN(62, "GPIO_62"),
	PINCTRL_PIN(63, "GPIO_63"),
	PINCTRL_PIN(64, "GPIO_64"),
	PINCTRL_PIN(65, "GPIO_65"),
	PINCTRL_PIN(66, "GPIO_66"),
	PINCTRL_PIN(67, "GPIO_67"),
	PINCTRL_PIN(68, "GPIO_68"),
	PINCTRL_PIN(69, "GPIO_69"),
	PINCTRL_PIN(70, "GPIO_70"),
	PINCTRL_PIN(71, "GPIO_71"),
	PINCTRL_PIN(72, "GPIO_72"),
	PINCTRL_PIN(73, "GPIO_73"),
	PINCTRL_PIN(74, "GPIO_74"),
	PINCTRL_PIN(75, "GPIO_75"),
	PINCTRL_PIN(76, "GPIO_76"),
	PINCTRL_PIN(77, "GPIO_77"),
	PINCTRL_PIN(78, "GPIO_78"),
	PINCTRL_PIN(79, "GPIO_79"),
	PINCTRL_PIN(80, "GPIO_80"),
	PINCTRL_PIN(81, "GPIO_81"),
	PINCTRL_PIN(82, "GPIO_82"),
	PINCTRL_PIN(83, "GPIO_83"),
	PINCTRL_PIN(84, "GPIO_84"),
	PINCTRL_PIN(85, "GPIO_85"),
	PINCTRL_PIN(86, "GPIO_86"),
	PINCTRL_PIN(87, "GPIO_87"),
	PINCTRL_PIN(88, "GPIO_88"),
	PINCTRL_PIN(89, "GPIO_89"),
	PINCTRL_PIN(90, "GPIO_90"),
	PINCTRL_PIN(91, "GPIO_91"),
	PINCTRL_PIN(92, "GPIO_92"),
	PINCTRL_PIN(93, "GPIO_93"),
	PINCTRL_PIN(94, "GPIO_94"),
	PINCTRL_PIN(95, "GPIO_95"),
	PINCTRL_PIN(96, "GPIO_96"),
	PINCTRL_PIN(97, "GPIO_97"),
	PINCTRL_PIN(98, "GPIO_98"),
	PINCTRL_PIN(99, "GPIO_99"),
	PINCTRL_PIN(100, "GPIO_100"),
	PINCTRL_PIN(101, "GPIO_101"),
	PINCTRL_PIN(102, "GPIO_102"),
	PINCTRL_PIN(103, "GPIO_103"),
	PINCTRL_PIN(104, "GPIO_104"),
	PINCTRL_PIN(105, "GPIO_105"),
	PINCTRL_PIN(106, "GPIO_106"),
	PINCTRL_PIN(107, "GPIO_107"),
	PINCTRL_PIN(108, "GPIO_108"),
	PINCTRL_PIN(109, "GPIO_109"),
	PINCTRL_PIN(110, "GPIO_110"),
	PINCTRL_PIN(111, "GPIO_111"),
	PINCTRL_PIN(112, "GPIO_112"),
	PINCTRL_PIN(113, "GPIO_113"),
	PINCTRL_PIN(114, "GPIO_114"),
	PINCTRL_PIN(115, "GPIO_115"),
	PINCTRL_PIN(116, "GPIO_116"),
	PINCTRL_PIN(117, "GPIO_117"),
	PINCTRL_PIN(118, "GPIO_118"),
	PINCTRL_PIN(119, "GPIO_119"),
	PINCTRL_PIN(120, "GPIO_120"),
	PINCTRL_PIN(121, "GPIO_121"),
	PINCTRL_PIN(122, "GPIO_122"),
	PINCTRL_PIN(123, "GPIO_123"),
	PINCTRL_PIN(124, "GPIO_124"),
	PINCTRL_PIN(125, "GPIO_125"),
	PINCTRL_PIN(126, "GPIO_126"),
	PINCTRL_PIN(127, "GPIO_127"),
	PINCTRL_PIN(128, "GPIO_128"),
	PINCTRL_PIN(129, "GPIO_129"),
	PINCTRL_PIN(130, "GPIO_130"),
	PINCTRL_PIN(131, "GPIO_131"),
	PINCTRL_PIN(132, "GPIO_132"),
	PINCTRL_PIN(133, "GPIO_133"),
	PINCTRL_PIN(134, "GPIO_134"),
	PINCTRL_PIN(135, "GPIO_135"),
	PINCTRL_PIN(136, "GPIO_136"),
	PINCTRL_PIN(137, "GPIO_137"),
	PINCTRL_PIN(138, "GPIO_138"),
	PINCTRL_PIN(139, "GPIO_139"),
	PINCTRL_PIN(140, "GPIO_140"),
	PINCTRL_PIN(141, "GPIO_141"),
	PINCTRL_PIN(142, "GPIO_142"),
	PINCTRL_PIN(143, "GPIO_143"),
	PINCTRL_PIN(144, "GPIO_144"),
	PINCTRL_PIN(145, "GPIO_145"),
	PINCTRL_PIN(146, "UFS_RESET"),
	PINCTRL_PIN(147, "SDC1_RCLK"),
	PINCTRL_PIN(148, "SDC1_CLK"),
	PINCTRL_PIN(149, "SDC1_CMD"),
	PINCTRL_PIN(150, "SDC1_DATA"),
	PINCTRL_PIN(151, "SDC2_CLK"),
	PINCTRL_PIN(152, "SDC2_CMD"),
	PINCTRL_PIN(153, "SDC2_DATA"),
};

#define DECLARE_MSM_GPIO_PINS(pin) \
	static const unsigned int gpio##pin##_pins[] = { pin }
DECLARE_MSM_GPIO_PINS(0);
DECLARE_MSM_GPIO_PINS(1);
DECLARE_MSM_GPIO_PINS(2);
DECLARE_MSM_GPIO_PINS(3);
DECLARE_MSM_GPIO_PINS(4);
DECLARE_MSM_GPIO_PINS(5);
DECLARE_MSM_GPIO_PINS(6);
DECLARE_MSM_GPIO_PINS(7);
DECLARE_MSM_GPIO_PINS(8);
DECLARE_MSM_GPIO_PINS(9);
DECLARE_MSM_GPIO_PINS(10);
DECLARE_MSM_GPIO_PINS(11);
DECLARE_MSM_GPIO_PINS(12);
DECLARE_MSM_GPIO_PINS(13);
DECLARE_MSM_GPIO_PINS(14);
DECLARE_MSM_GPIO_PINS(15);
DECLARE_MSM_GPIO_PINS(16);
DECLARE_MSM_GPIO_PINS(17);
DECLARE_MSM_GPIO_PINS(18);
DECLARE_MSM_GPIO_PINS(19);
DECLARE_MSM_GPIO_PINS(20);
DECLARE_MSM_GPIO_PINS(21);
DECLARE_MSM_GPIO_PINS(22);
DECLARE_MSM_GPIO_PINS(23);
DECLARE_MSM_GPIO_PINS(24);
DECLARE_MSM_GPIO_PINS(25);
DECLARE_MSM_GPIO_PINS(26);
DECLARE_MSM_GPIO_PINS(27);
DECLARE_MSM_GPIO_PINS(28);
DECLARE_MSM_GPIO_PINS(29);
DECLARE_MSM_GPIO_PINS(30);
DECLARE_MSM_GPIO_PINS(31);
DECLARE_MSM_GPIO_PINS(32);
DECLARE_MSM_GPIO_PINS(33);
DECLARE_MSM_GPIO_PINS(34);
DECLARE_MSM_GPIO_PINS(35);
DECLARE_MSM_GPIO_PINS(36);
DECLARE_MSM_GPIO_PINS(37);
DECLARE_MSM_GPIO_PINS(38);
DECLARE_MSM_GPIO_PINS(39);
DECLARE_MSM_GPIO_PINS(40);
DECLARE_MSM_GPIO_PINS(41);
DECLARE_MSM_GPIO_PINS(42);
DECLARE_MSM_GPIO_PINS(43);
DECLARE_MSM_GPIO_PINS(44);
DECLARE_MSM_GPIO_PINS(45);
DECLARE_MSM_GPIO_PINS(46);
DECLARE_MSM_GPIO_PINS(47);
DECLARE_MSM_GPIO_PINS(48);
DECLARE_MSM_GPIO_PINS(49);
DECLARE_MSM_GPIO_PINS(50);
DECLARE_MSM_GPIO_PINS(51);
DECLARE_MSM_GPIO_PINS(52);
DECLARE_MSM_GPIO_PINS(53);
DECLARE_MSM_GPIO_PINS(54);
DECLARE_MSM_GPIO_PINS(55);
DECLARE_MSM_GPIO_PINS(56);
DECLARE_MSM_GPIO_PINS(57);
DECLARE_MSM_GPIO_PINS(58);
DECLARE_MSM_GPIO_PINS(59);
DECLARE_MSM_GPIO_PINS(60);
DECLARE_MSM_GPIO_PINS(61);
DECLARE_MSM_GPIO_PINS(62);
DECLARE_MSM_GPIO_PINS(63);
DECLARE_MSM_GPIO_PINS(64);
DECLARE_MSM_GPIO_PINS(65);
DECLARE_MSM_GPIO_PINS(66);
DECLARE_MSM_GPIO_PINS(67);
DECLARE_MSM_GPIO_PINS(68);
DECLARE_MSM_GPIO_PINS(69);
DECLARE_MSM_GPIO_PINS(70);
DECLARE_MSM_GPIO_PINS(71);
DECLARE_MSM_GPIO_PINS(72);
DECLARE_MSM_GPIO_PINS(73);
DECLARE_MSM_GPIO_PINS(74);
DECLARE_MSM_GPIO_PINS(75);
DECLARE_MSM_GPIO_PINS(76);
DECLARE_MSM_GPIO_PINS(77);
DECLARE_MSM_GPIO_PINS(78);
DECLARE_MSM_GPIO_PINS(79);
DECLARE_MSM_GPIO_PINS(80);
DECLARE_MSM_GPIO_PINS(81);
DECLARE_MSM_GPIO_PINS(82);
DECLARE_MSM_GPIO_PINS(83);
DECLARE_MSM_GPIO_PINS(84);
DECLARE_MSM_GPIO_PINS(85);
DECLARE_MSM_GPIO_PINS(86);
DECLARE_MSM_GPIO_PINS(87);
DECLARE_MSM_GPIO_PINS(88);
DECLARE_MSM_GPIO_PINS(89);
DECLARE_MSM_GPIO_PINS(90);
DECLARE_MSM_GPIO_PINS(91);
DECLARE_MSM_GPIO_PINS(92);
DECLARE_MSM_GPIO_PINS(93);
DECLARE_MSM_GPIO_PINS(94);
DECLARE_MSM_GPIO_PINS(95);
DECLARE_MSM_GPIO_PINS(96);
DECLARE_MSM_GPIO_PINS(97);
DECLARE_MSM_GPIO_PINS(98);
DECLARE_MSM_GPIO_PINS(99);
DECLARE_MSM_GPIO_PINS(100);
DECLARE_MSM_GPIO_PINS(101);
DECLARE_MSM_GPIO_PINS(102);
DECLARE_MSM_GPIO_PINS(103);
DECLARE_MSM_GPIO_PINS(104);
DECLARE_MSM_GPIO_PINS(105);
DECLARE_MSM_GPIO_PINS(106);
DECLARE_MSM_GPIO_PINS(107);
DECLARE_MSM_GPIO_PINS(108);
DECLARE_MSM_GPIO_PINS(109);
DECLARE_MSM_GPIO_PINS(110);
DECLARE_MSM_GPIO_PINS(111);
DECLARE_MSM_GPIO_PINS(112);
DECLARE_MSM_GPIO_PINS(113);
DECLARE_MSM_GPIO_PINS(114);
DECLARE_MSM_GPIO_PINS(115);
DECLARE_MSM_GPIO_PINS(116);
DECLARE_MSM_GPIO_PINS(117);
DECLARE_MSM_GPIO_PINS(118);
DECLARE_MSM_GPIO_PINS(119);
DECLARE_MSM_GPIO_PINS(120);
DECLARE_MSM_GPIO_PINS(121);
DECLARE_MSM_GPIO_PINS(122);
DECLARE_MSM_GPIO_PINS(123);
DECLARE_MSM_GPIO_PINS(124);
DECLARE_MSM_GPIO_PINS(125);
DECLARE_MSM_GPIO_PINS(126);
DECLARE_MSM_GPIO_PINS(127);
DECLARE_MSM_GPIO_PINS(128);
DECLARE_MSM_GPIO_PINS(129);
DECLARE_MSM_GPIO_PINS(130);
DECLARE_MSM_GPIO_PINS(131);
DECLARE_MSM_GPIO_PINS(132);
DECLARE_MSM_GPIO_PINS(133);
DECLARE_MSM_GPIO_PINS(134);
DECLARE_MSM_GPIO_PINS(135);
DECLARE_MSM_GPIO_PINS(136);
DECLARE_MSM_GPIO_PINS(137);
DECLARE_MSM_GPIO_PINS(138);
DECLARE_MSM_GPIO_PINS(139);
DECLARE_MSM_GPIO_PINS(140);
DECLARE_MSM_GPIO_PINS(141);
DECLARE_MSM_GPIO_PINS(142);
DECLARE_MSM_GPIO_PINS(143);
DECLARE_MSM_GPIO_PINS(144);
DECLARE_MSM_GPIO_PINS(145);

static const unsigned int ufs_reset_pins[] = { 146 };
static const unsigned int sdc1_rclk_pins[] = { 147 };
static const unsigned int sdc1_clk_pins[] = { 148 };
static const unsigned int sdc1_cmd_pins[] = { 149 };
static const unsigned int sdc1_data_pins[] = { 150 };
static const unsigned int sdc2_clk_pins[] = { 151 };
static const unsigned int sdc2_cmd_pins[] = { 152 };
static const unsigned int sdc2_data_pins[] = { 153 };

enum sm7250_functions {
	msm_mux_gpio,
	msm_mux_adsp_ext,
	msm_mux_aoss_cti,
	msm_mux_atest_char,
	msm_mux_atest_tsens,
	msm_mux_atest_tsens2,
	msm_mux_atest_usb1,
	msm_mux_atest_usb2,
	msm_mux_aud_ref,
	msm_mux_cam_mclk,
	msm_mux_cci_async,
	msm_mux_cci_i2c,
	msm_mux_cci_timer0,
	msm_mux_cci_timer1,
	msm_mux_cci_timer2,
	msm_mux_cci_timer3,
	msm_mux_cci_timer4,
	msm_mux_cri_trng,
	msm_mux_dbg_out,
	msm_mux_ddr_bist,
	msm_mux_ddr_pxi0,
	msm_mux_ddr_pxi1,
	msm_mux_ddr_pxi2,
	msm_mux_ddr_pxi3,
	msm_mux_edp_hot,
	msm_mux_edp_lcd,
	msm_mux_gcc_gp1,
	msm_mux_gcc_gp2,
	msm_mux_gcc_gp3,
	msm_mux_gp_pdm0,
	msm_mux_gp_pdm1,
	msm_mux_gp_pdm2,
	msm_mux_gps_tx,
	msm_mux_ibi_i3c,
	msm_mux_jitter_bist,
	msm_mux_ldo_en,
	msm_mux_ldo_update,
	msm_mux_lpass_slimbus,
	msm_mux_m_voc,
	msm_mux_mdp_vsync,
	msm_mux_mdp_vsync0,
	msm_mux_mdp_vsync1,
	msm_mux_mdp_vsync2,
	msm_mux_mdp_vsync3,
	msm_mux_mss_lte,
	msm_mux_nav_gpio,
	msm_mux_nav_pps_in,
	msm_mux_nav_pps_out,
	msm_mux_pa_indicator,
	msm_mux_phase_flag,
	msm_mux_pll_bist,
	msm_mux_pll_bypassnl,
	msm_mux_pll_clk,
	msm_mux_pll_reset,
	msm_mux_pon_beamer,
	msm_mux_pri_mi2s,
	msm_mux_pri_mi2s_ws,
	msm_mux_prng_rosc,
	msm_mux_qdss,
	msm_mux_qdss_cti,
	msm_mux_qlink0_enable,
	msm_mux_qlink0_request,
	msm_mux_qlink0_wmss,
	msm_mux_qlink1_enable,
	msm_mux_qlink1_request,
	msm_mux_qlink1_wmss,
	msm_mux_qua_mi2s,
	msm_mux_qup00,
	msm_mux_qup01,
	msm_mux_qup02,
	msm_mux_qup04,
	msm_mux_qup05,
	msm_mux_qup10,
	msm_mux_qup11,
	msm_mux_qup12,
	msm_mux_qup13,
	msm_mux_qup14,
	msm_mux_qup15,
	msm_mux_sd_write,
	msm_mux_sdc40,
	msm_mux_sdc41,
	msm_mux_sdc42,
	msm_mux_sdc43,
	msm_mux_sdc4_clk,
	msm_mux_sdc4_cmd,
	msm_mux_sec_mi2s,
	msm_mux_ter_mi2s,
	msm_mux_tgu_ch0,
	msm_mux_tgu_ch1,
	msm_mux_tgu_ch2,
	msm_mux_tgu_ch3,
	msm_mux_tsense_pwm,
	msm_mux_uim1_clk,
	msm_mux_uim1_data,
	msm_mux_uim1_present,
	msm_mux_uim1_reset,
	msm_mux_uim2_clk,
	msm_mux_uim2_data,
	msm_mux_uim2_present,
	msm_mux_uim2_reset,
	msm_mux_uim_batt,
	msm_mux_usb2phy_ac,
	msm_mux_usb_phy,
	msm_mux_vfr_1,
	msm_mux_vsense_trigger,
	msm_mux_wlan1_adc0,
	msm_mux_wlan1_adc1,
	msm_mux_wlan2_adc0,
	msm_mux_wlan2_adc1,
	msm_mux__,
};

static const char * const gpio_groups[] = {
	"gpio0", "gpio1", "gpio2", "gpio3", "gpio4", "gpio5", "gpio6", "gpio7",
	"gpio8", "gpio9", "gpio10", "gpio11", "gpio12", "gpio13", "gpio14", "gpio15",
	"gpio16", "gpio17", "gpio18", "gpio19", "gpio20", "gpio21", "gpio22", "gpio23",
	"gpio24", "gpio25", "gpio26", "gpio27", "gpio28", "gpio29", "gpio30", "gpio31",
	"gpio32", "gpio33", "gpio34", "gpio35", "gpio36", "gpio37", "gpio38", "gpio39",
	"gpio40", "gpio41", "gpio42", "gpio43", "gpio44", "gpio45", "gpio46", "gpio47",
	"gpio48", "gpio49", "gpio50", "gpio51", "gpio52", "gpio53", "gpio54", "gpio55",
	"gpio56", "gpio57", "gpio58", "gpio59", "gpio60", "gpio61", "gpio62", "gpio63",
	"gpio64", "gpio65", "gpio66", "gpio67", "gpio68", "gpio69", "gpio70", "gpio71",
	"gpio72", "gpio73", "gpio74", "gpio75", "gpio76", "gpio77", "gpio78", "gpio79",
	"gpio80", "gpio81", "gpio82", "gpio83", "gpio84", "gpio85", "gpio86", "gpio87",
	"gpio88", "gpio89", "gpio90", "gpio91", "gpio92", "gpio93", "gpio94", "gpio95",
	"gpio96", "gpio97", "gpio98", "gpio99", "gpio100", "gpio101", "gpio102", "gpio103",
	"gpio104", "gpio105", "gpio106", "gpio107", "gpio108", "gpio109", "gpio110", "gpio111",
	"gpio112", "gpio113", "gpio114", "gpio115", "gpio116", "gpio117", "gpio118", "gpio119",
	"gpio120", "gpio121", "gpio122", "gpio123", "gpio124", "gpio125", "gpio126", "gpio127",
	"gpio128", "gpio129", "gpio130", "gpio131", "gpio132", "gpio133", "gpio134", "gpio135",
	"gpio136", "gpio137", "gpio138", "gpio139", "gpio140", "gpio141", "gpio142", "gpio143",
	"gpio144", "gpio145",
};

static const char * const adsp_ext_groups[] = {
	"gpio109",
};

static const char * const aoss_cti_groups[] = {
	"gpio84",
};

static const char * const atest_char_groups[] = {
	"gpio107", "gpio108", "gpio109", "gpio110", "gpio111",
};

static const char * const atest_tsens_groups[] = {
	"gpio84",
};

static const char * const atest_tsens2_groups[] = {
	"gpio44",
};

static const char * const atest_usb1_groups[] = {
	"gpio44", "gpio78", "gpio79", "gpio86", "gpio92",
};

static const char * const atest_usb2_groups[] = {
	"gpio59", "gpio60", "gpio73", "gpio95", "gpio97",
};

static const char * const aud_ref_groups[] = {
	"gpio83",
};

static const char * const cam_mclk_groups[] = {
	"gpio13", "gpio14", "gpio15", "gpio16", "gpio25",
};

static const char * const cci_async_groups[] = {
	"gpio24", "gpio25", "gpio26",
};

static const char * const cci_i2c_groups[] = {
	"gpio17", "gpio18", "gpio19", "gpio20", "gpio27", "gpio28",
};

static const char * const cci_timer0_groups[] = {
	"gpio21",
};

static const char * const cci_timer1_groups[] = {
	"gpio22",
};

static const char * const cci_timer2_groups[] = {
	"gpio23",
};

static const char * const cci_timer3_groups[] = {
	"gpio24",
};

static const char * const cci_timer4_groups[] = {
	"gpio25",
};

static const char * const cri_trng_groups[] = {
	"gpio115", "gpio116", "gpio117",
};

static const char * const dbg_out_groups[] = {
	"gpio91",
};

static const char * const ddr_bist_groups[] = {
	"gpio7", "gpio8", "gpio9", "gpio10",
};

static const char * const ddr_pxi0_groups[] = {
	"gpio43", "gpio44",
};

static const char * const ddr_pxi1_groups[] = {
	"gpio86", "gpio92",
};

static const char * const ddr_pxi2_groups[] = {
	"gpio78", "gpio79",
};

static const char * const ddr_pxi3_groups[] = {
	"gpio80", "gpio81",
};

static const char * const edp_hot_groups[] = {
	"gpio85",
};

static const char * const edp_lcd_groups[] = {
	"gpio11",
};

static const char * const gcc_gp1_groups[] = {
	"gpio48", "gpio56",
};

static const char * const gcc_gp2_groups[] = {
	"gpio21",
};

static const char * const gcc_gp3_groups[] = {
	"gpio22",
};

static const char * const gp_pdm0_groups[] = {
	"gpio37", "gpio68",
};

static const char * const gp_pdm1_groups[] = {
	"gpio8", "gpio50",
};

static const char * const gp_pdm2_groups[] = {
	"gpio57",
};

static const char * const gps_tx_groups[] = {
	"gpio90", "gpio92", "gpio93", "gpio104",
};

static const char * const ibi_i3c_groups[] = {
	"gpio42", "gpio43", "gpio59", "gpio60",
};

static const char * const jitter_bist_groups[] = {
	"gpio75",
};

static const char * const ldo_en_groups[] = {
	"gpio70",
};

static const char * const ldo_update_groups[] = {
	"gpio71",
};

static const char * const lpass_slimbus_groups[] = {
	"gpio53", "gpio54", "gpio55", "gpio56",
};

static const char * const m_voc_groups[] = {
	"gpio12",
};

static const char * const mdp_vsync_groups[] = {
	"gpio10", "gpio11", "gpio12", "gpio70", "gpio71",
};

static const char * const mdp_vsync0_groups[] = {
	"gpio63",
};

static const char * const mdp_vsync1_groups[] = {
	"gpio63",
};

static const char * const mdp_vsync2_groups[] = {
	"gpio63",
};

static const char * const mdp_vsync3_groups[] = {
	"gpio63",
};

static const char * const mss_lte_groups[] = {
	"gpio104", "gpio105",
};

static const char * const nav_gpio_groups[] = {
	"gpio92", "gpio93", "gpio105",
};

static const char * const nav_pps_in_groups[] = {
	"gpio90", "gpio92", "gpio93",
};

static const char * const nav_pps_out_groups[] = {
	"gpio90", "gpio92", "gpio93",
};

static const char * const pa_indicator_groups[] = {
	"gpio91",
};

static const char * const phase_flag_groups[] = {
	"gpio1", "gpio6", "gpio7", "gpio29", "gpio43", "gpio44", "gpio48", "gpio54",
	"gpio59", "gpio60", "gpio71", "gpio73", "gpio75", "gpio76", "gpio77", "gpio78",
	"gpio79", "gpio80", "gpio81", "gpio82", "gpio84", "gpio85", "gpio86", "gpio94",
	"gpio95", "gpio96", "gpio97", "gpio98", "gpio99", "gpio100", "gpio101", "gpio102",
	"gpio103",
};

static const char * const pll_bist_groups[] = {
	"gpio76",
};

static const char * const pll_bypassnl_groups[] = {
	"gpio13",
};

static const char * const pll_clk_groups[] = {
	"gpio77",
};

static const char * const pll_reset_groups[] = {
	"gpio14",
};

static const char * const pon_beamer_groups[] = {
	"gpio90",
};

static const char * const pri_mi2s_groups[] = {
	"gpio49", "gpio51", "gpio52",
};

static const char * const pri_mi2s_ws_groups[] = {
	"gpio50",
};

static const char * const prng_rosc_groups[] = {
	"gpio72",
};

static const char * const qdss_groups[] = {
	"gpio13", "gpio14", "gpio15", "gpio16", "gpio17", "gpio18", "gpio19", "gpio20",
	"gpio21", "gpio22", "gpio23", "gpio24", "gpio25", "gpio26", "gpio27", "gpio28",
	"gpio29", "gpio30", "gpio31", "gpio34", "gpio35", "gpio36", "gpio37", "gpio49",
	"gpio50", "gpio51", "gpio52", "gpio57", "gpio107", "gpio108", "gpio109", "gpio110",
	"gpio111", "gpio112", "gpio113", "gpio114",
};

static const char * const qdss_cti_groups[] = {
	"gpio4", "gpio5", "gpio32", "gpio45", "gpio46", "gpio63",
};

static const char * const qlink0_enable_groups[] = {
	"gpio89",
};

static const char * const qlink0_request_groups[] = {
	"gpio88",
};

static const char * const qlink0_wmss_groups[] = {
	"gpio87",
};

static const char * const qlink1_enable_groups[] = {
	"gpio86",
};

static const char * const qlink1_request_groups[] = {
	"gpio85",
};

static const char * const qlink1_wmss_groups[] = {
	"gpio84",
};

static const char * const qua_mi2s_groups[] = {
	"gpio58",
};

static const char * const qup00_groups[] = {
	"gpio42", "gpio43", "gpio44", "gpio45",
};

static const char * const qup01_groups[] = {
	"gpio0", "gpio1", "gpio2", "gpio3", "gpio12", "gpio37",
};

static const char * const qup02_groups[] = {
	"gpio34", "gpio35", "gpio36", "gpio37",
};

static const char * const qup04_groups[] = {
	"gpio29", "gpio30", "gpio31", "gpio32",
};

static const char * const qup05_groups[] = {
	"gpio38", "gpio39", "gpio40", "gpio41",
};

static const char * const qup10_groups[] = {
	"gpio59", "gpio60", "gpio61", "gpio62", "gpio63", "gpio70", "gpio71",
};

static const char * const qup11_groups[] = {
	"gpio6", "gpio7", "gpio8", "gpio9",
};

static const char * const qup12_groups[] = {
	"gpio49", "gpio50", "gpio51", "gpio52", "gpio57", "gpio58",
};

static const char * const qup13_groups[] = {
	"gpio46", "gpio47", "gpio48", "gpio63",
};

static const char * const qup14_groups[] = {
	"gpio53", "gpio54", "gpio55", "gpio56",
};

static const char * const qup15_groups[] = {
	"gpio108", "gpio109", "gpio112", "gpio113",
};

static const char * const sd_write_groups[] = {
	"gpio33",
};

static const char * const sdc40_groups[] = {
	"gpio69",
};

static const char * const sdc41_groups[] = {
	"gpio68",
};

static const char * const sdc42_groups[] = {
	"gpio67",
};

static const char * const sdc43_groups[] = {
	"gpio65",
};

static const char * const sdc4_clk_groups[] = {
	"gpio66",
};

static const char * const sdc4_cmd_groups[] = {
	"gpio64",
};

static const char * const sec_mi2s_groups[] = {
	"gpio57", "gpio70", "gpio71", "gpio72", "gpio74",
};

static const char * const ter_mi2s_groups[] = {
	"gpio53", "gpio54", "gpio55", "gpio56",
};

static const char * const tgu_ch0_groups[] = {
	"gpio63",
};

static const char * const tgu_ch1_groups[] = {
	"gpio64",
};

static const char * const tgu_ch2_groups[] = {
	"gpio65",
};

static const char * const tgu_ch3_groups[] = {
	"gpio62",
};

static const char * const tsense_pwm_groups[] = {
	"gpio135",
};

static const char * const uim1_clk_groups[] = {
	"gpio80",
};

static const char * const uim1_data_groups[] = {
	"gpio79",
};

static const char * const uim1_present_groups[] = {
	"gpio82",
};

static const char * const uim1_reset_groups[] = {
	"gpio81",
};

static const char * const uim2_clk_groups[] = {
	"gpio76",
};

static const char * const uim2_data_groups[] = {
	"gpio75",
};

static const char * const uim2_present_groups[] = {
	"gpio78",
};

static const char * const uim2_reset_groups[] = {
	"gpio77",
};

static const char * const uim_batt_groups[] = {
	"gpio84",
};

static const char * const usb2phy_ac_groups[] = {
	"gpio83",
};

static const char * const usb_phy_groups[] = {
	"gpio114",
};

static const char * const vfr_1_groups[] = {
	"gpio65",
};

static const char * const vsense_trigger_groups[] = {
	"gpio44",
};

static const char * const wlan1_adc0_groups[] = {
	"gpio86",
};

static const char * const wlan1_adc1_groups[] = {
	"gpio92",
};

static const char * const wlan2_adc0_groups[] = {
	"gpio79",
};

static const char * const wlan2_adc1_groups[] = {
	"gpio78",
};

static const struct pinfunction sm7250_functions[] = {
	MSM_PIN_FUNCTION(gpio),
	MSM_PIN_FUNCTION(adsp_ext),
	MSM_PIN_FUNCTION(aoss_cti),
	MSM_PIN_FUNCTION(atest_char),
	MSM_PIN_FUNCTION(atest_tsens),
	MSM_PIN_FUNCTION(atest_tsens2),
	MSM_PIN_FUNCTION(atest_usb1),
	MSM_PIN_FUNCTION(atest_usb2),
	MSM_PIN_FUNCTION(aud_ref),
	MSM_PIN_FUNCTION(cam_mclk),
	MSM_PIN_FUNCTION(cci_async),
	MSM_PIN_FUNCTION(cci_i2c),
	MSM_PIN_FUNCTION(cci_timer0),
	MSM_PIN_FUNCTION(cci_timer1),
	MSM_PIN_FUNCTION(cci_timer2),
	MSM_PIN_FUNCTION(cci_timer3),
	MSM_PIN_FUNCTION(cci_timer4),
	MSM_PIN_FUNCTION(cri_trng),
	MSM_PIN_FUNCTION(dbg_out),
	MSM_PIN_FUNCTION(ddr_bist),
	MSM_PIN_FUNCTION(ddr_pxi0),
	MSM_PIN_FUNCTION(ddr_pxi1),
	MSM_PIN_FUNCTION(ddr_pxi2),
	MSM_PIN_FUNCTION(ddr_pxi3),
	MSM_PIN_FUNCTION(edp_hot),
	MSM_PIN_FUNCTION(edp_lcd),
	MSM_PIN_FUNCTION(gcc_gp1),
	MSM_PIN_FUNCTION(gcc_gp2),
	MSM_PIN_FUNCTION(gcc_gp3),
	MSM_PIN_FUNCTION(gp_pdm0),
	MSM_PIN_FUNCTION(gp_pdm1),
	MSM_PIN_FUNCTION(gp_pdm2),
	MSM_PIN_FUNCTION(gps_tx),
	MSM_PIN_FUNCTION(ibi_i3c),
	MSM_PIN_FUNCTION(jitter_bist),
	MSM_PIN_FUNCTION(ldo_en),
	MSM_PIN_FUNCTION(ldo_update),
	MSM_PIN_FUNCTION(lpass_slimbus),
	MSM_PIN_FUNCTION(m_voc),
	MSM_PIN_FUNCTION(mdp_vsync),
	MSM_PIN_FUNCTION(mdp_vsync0),
	MSM_PIN_FUNCTION(mdp_vsync1),
	MSM_PIN_FUNCTION(mdp_vsync2),
	MSM_PIN_FUNCTION(mdp_vsync3),
	MSM_PIN_FUNCTION(mss_lte),
	MSM_PIN_FUNCTION(nav_gpio),
	MSM_PIN_FUNCTION(nav_pps_in),
	MSM_PIN_FUNCTION(nav_pps_out),
	MSM_PIN_FUNCTION(pa_indicator),
	MSM_PIN_FUNCTION(phase_flag),
	MSM_PIN_FUNCTION(pll_bist),
	MSM_PIN_FUNCTION(pll_bypassnl),
	MSM_PIN_FUNCTION(pll_clk),
	MSM_PIN_FUNCTION(pll_reset),
	MSM_PIN_FUNCTION(pon_beamer),
	MSM_PIN_FUNCTION(pri_mi2s),
	MSM_PIN_FUNCTION(pri_mi2s_ws),
	MSM_PIN_FUNCTION(prng_rosc),
	MSM_PIN_FUNCTION(qdss),
	MSM_PIN_FUNCTION(qdss_cti),
	MSM_PIN_FUNCTION(qlink0_enable),
	MSM_PIN_FUNCTION(qlink0_request),
	MSM_PIN_FUNCTION(qlink0_wmss),
	MSM_PIN_FUNCTION(qlink1_enable),
	MSM_PIN_FUNCTION(qlink1_request),
	MSM_PIN_FUNCTION(qlink1_wmss),
	MSM_PIN_FUNCTION(qua_mi2s),
	MSM_PIN_FUNCTION(qup00),
	MSM_PIN_FUNCTION(qup01),
	MSM_PIN_FUNCTION(qup02),
	MSM_PIN_FUNCTION(qup04),
	MSM_PIN_FUNCTION(qup05),
	MSM_PIN_FUNCTION(qup10),
	MSM_PIN_FUNCTION(qup11),
	MSM_PIN_FUNCTION(qup12),
	MSM_PIN_FUNCTION(qup13),
	MSM_PIN_FUNCTION(qup14),
	MSM_PIN_FUNCTION(qup15),
	MSM_PIN_FUNCTION(sd_write),
	MSM_PIN_FUNCTION(sdc40),
	MSM_PIN_FUNCTION(sdc41),
	MSM_PIN_FUNCTION(sdc42),
	MSM_PIN_FUNCTION(sdc43),
	MSM_PIN_FUNCTION(sdc4_clk),
	MSM_PIN_FUNCTION(sdc4_cmd),
	MSM_PIN_FUNCTION(sec_mi2s),
	MSM_PIN_FUNCTION(ter_mi2s),
	MSM_PIN_FUNCTION(tgu_ch0),
	MSM_PIN_FUNCTION(tgu_ch1),
	MSM_PIN_FUNCTION(tgu_ch2),
	MSM_PIN_FUNCTION(tgu_ch3),
	MSM_PIN_FUNCTION(tsense_pwm),
	MSM_PIN_FUNCTION(uim1_clk),
	MSM_PIN_FUNCTION(uim1_data),
	MSM_PIN_FUNCTION(uim1_present),
	MSM_PIN_FUNCTION(uim1_reset),
	MSM_PIN_FUNCTION(uim2_clk),
	MSM_PIN_FUNCTION(uim2_data),
	MSM_PIN_FUNCTION(uim2_present),
	MSM_PIN_FUNCTION(uim2_reset),
	MSM_PIN_FUNCTION(uim_batt),
	MSM_PIN_FUNCTION(usb2phy_ac),
	MSM_PIN_FUNCTION(usb_phy),
	MSM_PIN_FUNCTION(vfr_1),
	MSM_PIN_FUNCTION(vsense_trigger),
	MSM_PIN_FUNCTION(wlan1_adc0),
	MSM_PIN_FUNCTION(wlan1_adc1),
	MSM_PIN_FUNCTION(wlan2_adc0),
	MSM_PIN_FUNCTION(wlan2_adc1),
};

/*
 * Every pin is maintained as a single group, and missing or non-existing pin
 * would be maintained as dummy group to synchronize pin group index with
 * pin descriptor registered with pinctrl core.
 * Clients would not be able to request these dummy pin groups.
 */
/*
 * Every pin is maintained as a single group...
 */
/*
 * Every pin is maintained as a single group...
 */
static const struct msm_pingroup sm7250_groups[] = {
	[0] = PINGROUP(0, EAST, qup01, _, _, _, _, _, _, _, _),
	[1] = PINGROUP(1, EAST, qup01, phase_flag, _, _, _, _, _, _, _),
	[2] = PINGROUP(2, EAST, qup01, _, _, _, _, _, _, _, _),
	[3] = PINGROUP(3, EAST, qup01, _, _, _, _, _, _, _, _),
	[4] = PINGROUP(4, EAST, qdss_cti, _, _, _, _, _, _, _, _),
	[5] = PINGROUP(5, EAST, qdss_cti, _, _, _, _, _, _, _, _),
	[6] = PINGROUP(6, WEST, qup11, phase_flag, _, _, _, _, _, _, _),
	[7] = PINGROUP(7, WEST, qup11, ddr_bist, phase_flag, _, _, _, _, _, _),
	[8] = PINGROUP(8, WEST, qup11, gp_pdm1, ddr_bist, _, _, _, _, _, _),
	[9] = PINGROUP(9, WEST, qup11, ddr_bist, _, _, _, _, _, _, _),
	[10] = PINGROUP(10, WEST, mdp_vsync, ddr_bist, _, _, _, _, _, _, _),
	[11] = PINGROUP(11, EAST, mdp_vsync, edp_lcd, _, _, _, _, _, _, _),
	[12] = PINGROUP(12, EAST, mdp_vsync, m_voc, qup01, _, _, _, _, _, _),
	[13] = PINGROUP(13, EAST, cam_mclk, pll_bypassnl, qdss, _, _, _, _, _, _),
	[14] = PINGROUP(14, EAST, cam_mclk, pll_reset, qdss, _, _, _, _, _, _),
	[15] = PINGROUP(15, EAST, cam_mclk, qdss, _, _, _, _, _, _, _),
	[16] = PINGROUP(16, EAST, cam_mclk, qdss, _, _, _, _, _, _, _),
	[17] = PINGROUP(17, EAST, cci_i2c, qdss, _, _, _, _, _, _, _),
	[18] = PINGROUP(18, EAST, cci_i2c, qdss, _, _, _, _, _, _, _),
	[19] = PINGROUP(19, EAST, cci_i2c, qdss, _, _, _, _, _, _, _),
	[20] = PINGROUP(20, EAST, cci_i2c, qdss, _, _, _, _, _, _, _),
	[21] = PINGROUP(21, EAST, cci_timer0, gcc_gp2, qdss, _, _, _, _, _, _),
	[22] = PINGROUP(22, EAST, cci_timer1, gcc_gp3, qdss, _, _, _, _, _, _),
	[23] = PINGROUP(23, EAST, cci_timer2, qdss, _, _, _, _, _, _, _),
	[24] = PINGROUP(24, EAST, cci_timer3, cci_async, qdss, _, _, _, _, _, _),
	[25] = PINGROUP(25, EAST, cci_timer4, cci_async, cam_mclk, qdss, _, _, _, _, _),
	[26] = PINGROUP(26, EAST, cci_async, qdss, _, _, _, _, _, _, _),
	[27] = PINGROUP(27, EAST, cci_i2c, qdss, _, _, _, _, _, _, _),
	[28] = PINGROUP(28, EAST, cci_i2c, qdss, _, _, _, _, _, _, _),
	[29] = PINGROUP(29, EAST, qup04, phase_flag, qdss, _, _, _, _, _, _),
	[30] = PINGROUP(30, EAST, qup04, qdss, _, _, _, _, _, _, _),
	[31] = PINGROUP(31, EAST, qup04, qdss, _, _, _, _, _, _, _),
	[32] = PINGROUP(32, EAST, qup04, qdss_cti, _, _, _, _, _, _, _),
	[33] = PINGROUP(33, WEST, sd_write, _, _, _, _, _, _, _, _),
	[34] = PINGROUP(34, EAST, qup02, qdss, _, _, _, _, _, _, _),
	[35] = PINGROUP(35, EAST, qup02, qdss, _, _, _, _, _, _, _),
	[36] = PINGROUP(36, EAST, qup02, qdss, _, _, _, _, _, _, _),
	[37] = PINGROUP(37, EAST, qup02, qup01, gp_pdm0, qdss, _, _, _, _, _),
	[38] = PINGROUP(38, EAST, qup05, _, _, _, _, _, _, _, _),
	[39] = PINGROUP(39, EAST, qup05, _, _, _, _, _, _, _, _),
	[40] = PINGROUP(40, EAST, qup05, _, _, _, _, _, _, _, _),
	[41] = PINGROUP(41, EAST, qup05, _, _, _, _, _, _, _, _),
	[42] = PINGROUP(42, EAST, qup00, ibi_i3c, _, _, _, _, _, _, _),
	[43] = PINGROUP(43, EAST, qup00, ibi_i3c, phase_flag, ddr_pxi0, _, _, _, _, _),
	[44] = PINGROUP(44, EAST, qup00, phase_flag, atest_tsens2, vsense_trigger, atest_usb1, ddr_pxi0, _, _, _),
	[45] = PINGROUP(45, EAST, qup00, qdss_cti, _, _, _, _, _, _, _),
	[46] = PINGROUP(46, WEST, qup13, qdss_cti, _, _, _, _, _, _, _),
	[47] = PINGROUP(47, WEST, qup13, _, _, _, _, _, _, _, _),
	[48] = PINGROUP(48, WEST, qup13, gcc_gp1, phase_flag, _, _, _, _, _, _),
	[49] = PINGROUP(49, WEST, pri_mi2s, qup12, qdss, _, _, _, _, _, _),
	[50] = PINGROUP(50, WEST, pri_mi2s_ws, qup12, gp_pdm1, qdss, _, _, _, _, _),
	[51] = PINGROUP(51, WEST, pri_mi2s, qup12, qdss, _, _, _, _, _, _),
	[52] = PINGROUP(52, WEST, pri_mi2s, qup12, qdss, _, _, _, _, _, _),
	[53] = PINGROUP(53, WEST, ter_mi2s, qup14, lpass_slimbus, _, _, _, _, _, _),
	[54] = PINGROUP(54, WEST, ter_mi2s, qup14, lpass_slimbus, phase_flag, _, _, _, _, _),
	[55] = PINGROUP(55, WEST, ter_mi2s, qup14, lpass_slimbus, _, _, _, _, _, _),
	[56] = PINGROUP(56, WEST, ter_mi2s, qup14, lpass_slimbus, gcc_gp1, _, _, _, _, _),
	[57] = PINGROUP(57, WEST, sec_mi2s, qup12, gp_pdm2, qdss, _, _, _, _, _),
	[58] = PINGROUP(58, WEST, qua_mi2s, qup12, _, _, _, _, _, _, _),
	[59] = PINGROUP(59, WEST, qup10, ibi_i3c, phase_flag, atest_usb2, _, _, _, _, _),
	[60] = PINGROUP(60, WEST, qup10, ibi_i3c, phase_flag, atest_usb2, _, _, _, _, _),
	[61] = PINGROUP(61, WEST, qup10, _, _, _, _, _, _, _, _),
	[62] = PINGROUP(62, WEST, qup10, tgu_ch3, _, _, _, _, _, _, _),
	[63] = PINGROUP(63, WEST, qup13, mdp_vsync0, qup10, mdp_vsync1, mdp_vsync2, mdp_vsync3, tgu_ch0, qdss_cti, _),
	[64] = PINGROUP(64, EAST, sdc4_cmd, tgu_ch1, _, _, _, _, _, _, _),
	[65] = PINGROUP(65, EAST, sdc43, vfr_1, tgu_ch2, _, _, _, _, _, _),
	[66] = PINGROUP(66, EAST, sdc4_clk, _, _, _, _, _, _, _, _),
	[67] = PINGROUP(67, EAST, sdc42, _, _, _, _, _, _, _, _),
	[68] = PINGROUP(68, EAST, sdc41, gp_pdm0, _, _, _, _, _, _, _),
	[69] = PINGROUP(69, EAST, sdc40, _, _, _, _, _, _, _, _),
	[70] = PINGROUP(70, WEST, sec_mi2s, qup10, mdp_vsync, ldo_en, _, _, _, _, _),
	[71] = PINGROUP(71, WEST, sec_mi2s, qup10, mdp_vsync, ldo_update, phase_flag, _, _, _, _),
	[72] = PINGROUP(72, WEST, sec_mi2s, prng_rosc, _, _, _, _, _, _, _),
	[73] = PINGROUP(73, EAST, phase_flag, atest_usb2, _, _, _, _, _, _, _),
	[74] = PINGROUP(74, WEST, sec_mi2s, _, _, _, _, _, _, _, _),
	[75] = PINGROUP(75, SOUTH, uim2_data, jitter_bist, phase_flag, _, _, _, _, _, _),
	[76] = PINGROUP(76, SOUTH, uim2_clk, pll_bist, phase_flag, _, _, _, _, _, _),
	[77] = PINGROUP(77, SOUTH, uim2_reset, pll_clk, phase_flag, _, _, _, _, _, _),
	[78] = PINGROUP(78, SOUTH, uim2_present, phase_flag, wlan2_adc1, atest_usb1, ddr_pxi2, _, _, _, _),
	[79] = PINGROUP(79, SOUTH, uim1_data, phase_flag, wlan2_adc0, atest_usb1, ddr_pxi2, _, _, _, _),
	[80] = PINGROUP(80, SOUTH, uim1_clk, phase_flag, ddr_pxi3, _, _, _, _, _, _),
	[81] = PINGROUP(81, SOUTH, uim1_reset, phase_flag, ddr_pxi3, _, _, _, _, _, _),
	[82] = PINGROUP(82, SOUTH, uim1_present, phase_flag, _, _, _, _, _, _, _),
	[83] = PINGROUP(83, WEST, aud_ref, usb2phy_ac, _, _, _, _, _, _, _),
	[84] = PINGROUP(84, SOUTH, qlink1_wmss, uim_batt, aoss_cti, phase_flag, atest_tsens, _, _, _, _),
	[85] = PINGROUP(85, SOUTH, qlink1_request, edp_hot, phase_flag, _, _, _, _, _, _),
	[86] = PINGROUP(86, SOUTH, qlink1_enable, phase_flag, wlan1_adc0, atest_usb1, ddr_pxi1, _, _, _, _),
	[87] = PINGROUP(87, EAST, qlink0_wmss, _, _, _, _, _, _, _, _),
	[88] = PINGROUP(88, EAST, qlink0_request, _, _, _, _, _, _, _, _),
	[89] = PINGROUP(89, EAST, qlink0_enable, _, _, _, _, _, _, _, _),
	[90] = PINGROUP(90, SOUTH, pon_beamer, nav_pps_in, nav_pps_out, gps_tx, _, _, _, _, _),
	[91] = PINGROUP(91, SOUTH, pa_indicator, dbg_out, _, _, _, _, _, _, _),
	[92] = PINGROUP(92, SOUTH, nav_gpio, nav_pps_in, nav_pps_out, gps_tx, wlan1_adc1, atest_usb1, ddr_pxi1, _, _),
	[93] = PINGROUP(93, SOUTH, gps_tx, nav_gpio, nav_pps_in, nav_pps_out, _, _, _, _, _),
	[94] = PINGROUP(94, SOUTH, phase_flag, _, _, _, _, _, _, _, _),
	[95] = PINGROUP(95, SOUTH, phase_flag, atest_usb2, _, _, _, _, _, _, _),
	[96] = PINGROUP(96, SOUTH, phase_flag, _, _, _, _, _, _, _, _),
	[97] = PINGROUP(97, SOUTH, phase_flag, atest_usb2, _, _, _, _, _, _, _),
	[98] = PINGROUP(98, SOUTH, phase_flag, _, _, _, _, _, _, _, _),
	[99] = PINGROUP(99, SOUTH, phase_flag, _, _, _, _, _, _, _, _),
	[100] = PINGROUP(100, SOUTH, phase_flag, _, _, _, _, _, _, _, _),
	[101] = PINGROUP(101, SOUTH, phase_flag, _, _, _, _, _, _, _, _),
	[102] = PINGROUP(102, SOUTH, phase_flag, _, _, _, _, _, _, _, _),
	[103] = PINGROUP(103, SOUTH, phase_flag, _, _, _, _, _, _, _, _),
	[104] = PINGROUP(104, EAST, mss_lte, gps_tx, _, _, _, _, _, _, _),
	[105] = PINGROUP(105, EAST, mss_lte, nav_gpio, _, _, _, _, _, _, _),
	[106] = PINGROUP(106, SOUTH, _, _, _, _, _, _, _, _, _),
	[107] = PINGROUP(107, EAST, qdss, atest_char, _, _, _, _, _, _, _),
	[108] = PINGROUP(108, WEST, qup15, qdss, atest_char, _, _, _, _, _, _),
	[109] = PINGROUP(109, WEST, qup15, adsp_ext, qdss, atest_char, _, _, _, _, _),
	[110] = PINGROUP(110, EAST, qdss, atest_char, _, _, _, _, _, _, _),
	[111] = PINGROUP(111, EAST, qdss, atest_char, _, _, _, _, _, _, _),
	[112] = PINGROUP(112, WEST, qup15, qdss, _, _, _, _, _, _, _),
	[113] = PINGROUP(113, WEST, qup15, qdss, _, _, _, _, _, _, _),
	[114] = PINGROUP(114, WEST, usb_phy, qdss, _, _, _, _, _, _, _),
	[115] = PINGROUP(115, WEST, cri_trng, _, _, _, _, _, _, _, _),
	[116] = PINGROUP(116, WEST, cri_trng, _, _, _, _, _, _, _, _),
	[117] = PINGROUP(117, WEST, cri_trng, _, _, _, _, _, _, _, _),
	[118] = PINGROUP(118, WEST, _, _, _, _, _, _, _, _, _),
	[119] = PINGROUP(119, WEST, _, _, _, _, _, _, _, _, _),
	[120] = PINGROUP(120, WEST, _, _, _, _, _, _, _, _, _),
	[121] = PINGROUP(121, WEST, _, _, _, _, _, _, _, _, _),
	[122] = PINGROUP(122, WEST, _, _, _, _, _, _, _, _, _),
	[123] = PINGROUP(123, WEST, _, _, _, _, _, _, _, _, _),
	[124] = PINGROUP(124, WEST, _, _, _, _, _, _, _, _, _),
	[125] = PINGROUP(125, WEST, _, _, _, _, _, _, _, _, _),
	[126] = PINGROUP(126, WEST, _, _, _, _, _, _, _, _, _),
	[127] = PINGROUP(127, WEST, _, _, _, _, _, _, _, _, _),
	[128] = PINGROUP(128, WEST, _, _, _, _, _, _, _, _, _),
	[129] = PINGROUP(129, WEST, _, _, _, _, _, _, _, _, _),
	[130] = PINGROUP(130, WEST, _, _, _, _, _, _, _, _, _),
	[131] = PINGROUP(131, WEST, _, _, _, _, _, _, _, _, _),
	[132] = PINGROUP(132, WEST, _, _, _, _, _, _, _, _, _),
	[133] = PINGROUP(133, WEST, _, _, _, _, _, _, _, _, _),
	[134] = PINGROUP(134, WEST, _, _, _, _, _, _, _, _, _),
	[135] = PINGROUP(135, WEST, tsense_pwm, _, _, _, _, _, _, _, _),
	[136] = PINGROUP(136, WEST, _, _, _, _, _, _, _, _, _),
	[137] = PINGROUP(137, WEST, _, _, _, _, _, _, _, _, _),
	[138] = PINGROUP(138, WEST, _, _, _, _, _, _, _, _, _),
	[139] = PINGROUP(139, WEST, _, _, _, _, _, _, _, _, _),
	[140] = PINGROUP(140, WEST, _, _, _, _, _, _, _, _, _),
	[141] = PINGROUP(141, WEST, _, _, _, _, _, _, _, _, _),
	[142] = PINGROUP(142, WEST, _, _, _, _, _, _, _, _, _),
	[143] = PINGROUP(143, WEST, _, _, _, _, _, _, _, _, _),
	[144] = PINGROUP(144, WEST, _, _, _, _, _, _, _, _, _),
	[145] = PINGROUP(145, WEST, _, _, _, _, _, _, _, _, _),
	[146] = UFS_RESET(ufs_reset, WEST, 0xbc000),
	[147] = SDC_QDSD_PINGROUP(sdc1_rclk, WEST, 0xb8000, 15, 0),
	[148] = SDC_QDSD_PINGROUP(sdc1_clk, WEST, 0xb8000, 13, 6),
	[149] = SDC_QDSD_PINGROUP(sdc1_cmd, WEST, 0xb8000, 11, 3),
	[150] = SDC_QDSD_PINGROUP(sdc1_data, WEST, 0xb8000, 9, 0),
	[151] = SDC_QDSD_PINGROUP(sdc2_clk, SOUTH, 0xb7000, 14, 6),
	[152] = SDC_QDSD_PINGROUP(sdc2_cmd, SOUTH, 0xb7000, 11, 3),
	[153] = SDC_QDSD_PINGROUP(sdc2_data, SOUTH, 0xb7000, 9, 0),
};
static const struct msm_gpio_wakeirq_map sm7250_pdc_map[] = {
	{0, 40},
	{3, 50},
	{4, 42},
	{5, 70},
	{6, 41},
	{9, 57},
	{10, 80},
	{11, 51},
	{22, 90},
	{24, 61},
	{26, 52},
	{30, 56},
	{31, 33},
	{32, 81},
	{33, 62},
	{34, 43},
	{36, 91},
	{37, 53},
	{38, 63},
	{39, 72},
	{41, 101},
	{42, 35},
	{43, 34},
	{45, 73},
	{47, 82},
	{48, 36},
	{49, 37},
	{50, 38},
	{52, 39},
	{53, 102},
	{55, 92},
	{56, 45},
	{57, 46},
	{58, 83},
	{59, 47},
	{62, 48},
	{64, 74},
	{65, 44},
	{66, 93},
	{67, 49},
	{68, 55},
	{69, 32},
	{70, 54},
	{73, 64},
	{74, 71},
	{78, 31},
	{82, 30},
	{84, 58},
	{85, 103},
	{86, 59},
	{87, 60},
	{88, 65},
	{89, 66},
	{90, 67},
	{91, 68},
	{92, 69},
	{93, 75},
	{94, 84},
	{95, 94},
	{96, 76},
	{98, 77},
	{101, 78},
	{104, 99},
	{109, 104},
	{110, 79},
	{113, 85},
	{121, 105},
	{122, 106},
	{125, 107},
	{126, 108},
	{127, 109},
};

/* 下游 Android 实际使用的引脚白名单（live_dts 全量审计 2026-10-10：
 * 解析 197 个 pinctrl 状态节点 + 全部 gpio 引用属性，仅保留启用消费者
 * 的引用；剔除数据数组伪匹配 gpio3/26/92/93/95）。注意：数据属性引用
 * ≠槽位可访问——gpio0 有启用的相机消费者引用但 ctl 读实测挂死（已
 * 剔除；其 PDC direct-connect 唤醒脚身份疑 TZ 接管）。probe 阶段的
 * 寄存器访问由 soc_data.no_get_direction 整体关闭（下游 4.19 注册零
 * 硬件访问）；本白名单只放行 gpio request / irq 分配。后续 bring-up
 * 新增消费者时按 live_dts 对应引脚补入（触摸 gpio4/5/9、DSI TE
 * gpio10/11、G7A ESD gpio55 已在）。 */
static const unsigned int sm7250_used_gpios[] = {
	/* 触摸 novatek: reset/irq/cs = 4/5/9；相机 cci sensor/eprom:
	 * 13-21,23,25,27-33；相机 LDO enable: 122,125,126,127 */
	4, 5, 9, 13, 14, 15, 16, 17, 18, 19, 20, 21, 23, 25, 27, 28, 29,
	30, 31, 32, 33, 122, 125, 126, 127,
	/* NFC: 12,31,34,35；UART SE2=36/37、SE5=38-41；SE0 i2c=42/43；
	 * pm8008=44；gpio-regulator: 45,56,68；SE7 spi=6/7；SE9 i2c=46/47；
	 * SE11 spi=108,109,112,113 */
	6, 7, 12, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
	56, 68, 108, 109, 112, 113,
	/* 显示: DSI TE0/TE1=10/11、G7A esd-irq=55；音频: wcd938x reset=57、
	 * sec_i2s=70-72,74、dmic=115-117,121,124、soundwire=8；
	 * smartpa: i2c=49/50、reset=58、irq=65 */
	8, 10, 11, 49, 50, 55, 57, 58, 65, 70, 71, 72, 74, 115, 116, 117,
	121, 124,
	/* 指纹: 63,121；充电: bq25970 irq=64、smb5 vbus=67；onewire=66；
	 * BT sw-ctrl=73；SIM tray=82 */
	63, 64, 66, 67, 73, 82,
};

static int sm7250_init_valid_mask(struct gpio_chip *gc,
				  unsigned long *valid_mask,
				  unsigned int ngpios)
{
	unsigned int i;

	bitmap_zero(valid_mask, ngpios);
	for (i = 0; i < ARRAY_SIZE(sm7250_used_gpios); i++)
		if (sm7250_used_gpios[i] < ngpios)
			set_bit(sm7250_used_gpios[i], valid_mask);

	return 0;
}

static const struct msm_pinctrl_soc_data sm7250_tlmm = {
	.pins = sm7250_pins,
	.npins = ARRAY_SIZE(sm7250_pins),
	.functions = sm7250_functions,
	.nfunctions = ARRAY_SIZE(sm7250_functions),
	.groups = sm7250_groups,
	.ngroups = ARRAY_SIZE(sm7250_groups),
	.ngpios = 146,
	.wakeirq_map = sm7250_pdc_map,
	.nwakeirq_map = ARRAY_SIZE(sm7250_pdc_map),
	.wakeirq_dual_edge_errata = true,
	.init_valid_mask = sm7250_init_valid_mask,
	.no_get_direction = true,
};


static int sm7250_tlmm_probe(struct platform_device *pdev)
{
	return msm_pinctrl_probe(pdev, &sm7250_tlmm);
}

static const struct of_device_id sm7250_tlmm_of_match[] = {
	{ .compatible = "qcom,sm7250-tlmm", },
	{ },
};
MODULE_DEVICE_TABLE(of, sm7250_tlmm_of_match);

static struct platform_driver sm7250_tlmm_driver = {
	.driver = {
		.name = "sm7250-tlmm",
		.pm = &msm_pinctrl_dev_pm_ops,
		.of_match_table = sm7250_tlmm_of_match,
	},
	.probe = sm7250_tlmm_probe,
};

static int __init sm7250_tlmm_init(void)
{
	return platform_driver_register(&sm7250_tlmm_driver);
}
arch_initcall(sm7250_tlmm_init);

static void __exit sm7250_tlmm_exit(void)
{
	platform_driver_unregister(&sm7250_tlmm_driver);
}
module_exit(sm7250_tlmm_exit);

MODULE_DESCRIPTION("Qualcomm SM7250 TLMM driver");
MODULE_LICENSE("GPL");
