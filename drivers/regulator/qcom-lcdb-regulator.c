// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2016-2019, The Linux Foundation. All rights reserved.
 * Copyright (c) 2026, Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * PM8150L LCDB（LCD bias）调节器驱动 —— 主线裁剪移植版。
 *
 * 源自下游 drivers/regulator/qpnp-lcdb-regulator.c
 * （Xiaomi_Kernel_OpenSource-picasso-r-oss）。保留核心功能：模块使能
 * （ENABLE_CTL1@+0x46 + VREG_OK@+0x10 轮询）、LDO(avdd)/NCP(avee) 输出
 * 电压设定（+0x71/+0x81，4000-6000mV 非线性编码）、BST 电压联动
 * （+0x41，max(LDO,NCP)+headroom，默认 150mV）。
 * 裁剪掉：PM660L workaround、TTW 触摸唤醒、休眠 save/restore、SC 短路
 * 中断处理（bring-up 阶段后置）。下游 DT 结构保留：lcdb 节点的 ldo/ncp
 * 子节点即两路调节器。
 */

#include <linux/delay.h>
#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/regmap.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/regulator/of_regulator.h>
#include <linux/slab.h>
#include <linux/string.h>

/* LCDB 模块寄存器（相对 lcdb 节点 reg 基址） */
#define LCDB_ENABLE_CTL1_REG			0x46
#define MODULE_EN_BIT				BIT(7)

#define INT_RT_STATUS_REG			0x10
#define VREG_OK_RT_STS_BIT			BIT(0)

#define LCDB_BST_OUTPUT_VOLTAGE_REG		0x41
#define BST_OUTPUT_VOLTAGE_MASK			GENMASK(5, 0)
#define MIN_BST_VOLTAGE_MV			4700
#define MAX_BST_VOLTAGE_MV			6275
#define BST_VOLTAGE_STEP_MV			25
#define BST_HEADROOM_DEFAULT_MV			150

#define LCDB_LDO_OUTPUT_VOLTAGE_REG		0x71
#define LCDB_NCP_OUTPUT_VOLTAGE_REG		0x81
#define SET_OUTPUT_VOLTAGE_MASK			GENMASK(4, 0)
#define MIN_VOLTAGE_MV				4000
#define MAX_VOLTAGE_MV				6000
#define VOLTAGE_MIN_STEP_100_MV			4000
#define VOLTAGE_STEP_100_MV			100
#define VOLTAGE_MIN_STEP_50_MV			4950
#define VOLTAGE_STEP_50_MV			50
#define VOLTAGE_STEP_50MV_OFFSET		0xA

#define LCDB_VREG_OK_TIMEOUT_US			10000
#define LCDB_ENABLE_POLL_RETRIES		10

enum lcdb_reg_type {
	LCDB_LDO,
	LCDB_NCP,
};

struct lcdb_regulator {
	struct device *dev;
	struct regmap *regmap;
	unsigned int base;
	u16 headroom_mv;

	struct mutex lock;		/* 模块使能/电压互斥 */
	bool enabled;
	int ldo_mv;
	int ncp_mv;

	struct regulator_dev *ldo_rdev;
	struct regulator_dev *ncp_rdev;
};

static int lcdb_write(struct lcdb_regulator *lcdb, unsigned int reg, u8 val)
{
	return regmap_write(lcdb->regmap, lcdb->base + reg, val);
}

static int lcdb_read(struct lcdb_regulator *lcdb, unsigned int reg, u8 *val)
{
	unsigned int v;
	int ret;

	ret = regmap_read(lcdb->regmap, lcdb->base + reg, &v);
	if (!ret)
		*val = v;
	return ret;
}

static int lcdb_update_bits(struct lcdb_regulator *lcdb, unsigned int reg,
			    u8 mask, u8 val)
{
	return regmap_update_bits(lcdb->regmap, lcdb->base + reg, mask, val);
}

/* LDO/NCP 输出电压寄存器编码：4000-4900mV 按 100mV 步进（val 0-9），
 * 4950-6000mV 按 50mV 步进（val 0xA-0x1F）。 */
static int lcdb_voltage_to_reg(int voltage_mv, u8 *val)
{
	if (voltage_mv < MIN_VOLTAGE_MV || voltage_mv > MAX_VOLTAGE_MV)
		return -EINVAL;

	if (voltage_mv < VOLTAGE_MIN_STEP_50_MV)
		*val = DIV_ROUND_UP(voltage_mv - VOLTAGE_MIN_STEP_100_MV,
				    VOLTAGE_STEP_100_MV);
	else
		*val = DIV_ROUND_UP(voltage_mv - VOLTAGE_MIN_STEP_50_MV,
				    VOLTAGE_STEP_50_MV) +
		       VOLTAGE_STEP_50MV_OFFSET;
	return 0;
}

static int lcdb_reg_from_voltage(int voltage_mv)
{
	u8 val = 0;

	if (lcdb_voltage_to_reg(voltage_mv, &val))
		return -EINVAL;
	return val;
}

static int lcdb_voltage_from_reg(u8 val)
{
	val &= SET_OUTPUT_VOLTAGE_MASK;
	if (val < VOLTAGE_STEP_50MV_OFFSET)
		return VOLTAGE_MIN_STEP_100_MV + val * VOLTAGE_STEP_100_MV;
	return VOLTAGE_MIN_STEP_50_MV +
	       (val - VOLTAGE_STEP_50MV_OFFSET) * VOLTAGE_STEP_50_MV;
}

/* Vout_Boost = headroom + max(Vout_LDO, abs(Vout_NCP)) */
static int lcdb_program_bst_voltage(struct lcdb_regulator *lcdb)
{
	int bst_mv;
	u8 val;

	bst_mv = max(lcdb->ldo_mv, lcdb->ncp_mv) + lcdb->headroom_mv;
	bst_mv = clamp(bst_mv, MIN_BST_VOLTAGE_MV, MAX_BST_VOLTAGE_MV);

	val = DIV_ROUND_UP(bst_mv - MIN_BST_VOLTAGE_MV, BST_VOLTAGE_STEP_MV);
	return lcdb_update_bits(lcdb, LCDB_BST_OUTPUT_VOLTAGE_REG,
				BST_OUTPUT_VOLTAGE_MASK, val);
}

static int lcdb_program_output_voltage(struct lcdb_regulator *lcdb,
				       enum lcdb_reg_type type)
{
	int voltage_mv = (type == LCDB_LDO) ? lcdb->ldo_mv : lcdb->ncp_mv;
	unsigned int reg = (type == LCDB_LDO) ?
			   LCDB_LDO_OUTPUT_VOLTAGE_REG :
			   LCDB_NCP_OUTPUT_VOLTAGE_REG;
	u8 val;
	int ret;

	ret = lcdb_voltage_to_reg(voltage_mv, &val);
	if (ret)
		return ret;

	return lcdb_update_bits(lcdb, reg, SET_OUTPUT_VOLTAGE_MASK, val);
}

static int lcdb_enable_module(struct lcdb_regulator *lcdb)
{
	int ret, i;
	u8 val;

	/* 输出电压须在模块使能前编程（寄存器在模块关闭时同样可写） */
	ret = lcdb_program_output_voltage(lcdb, LCDB_LDO);
	if (ret)
		return ret;
	ret = lcdb_program_output_voltage(lcdb, LCDB_NCP);
	if (ret)
		return ret;
	ret = lcdb_program_bst_voltage(lcdb);
	if (ret)
		return ret;

	ret = lcdb_write(lcdb, LCDB_ENABLE_CTL1_REG, MODULE_EN_BIT);
	if (ret)
		return ret;

	/* 轮询 VREG_OK（软启动完成） */
	for (i = 0; i < LCDB_ENABLE_POLL_RETRIES; i++) {
		ret = lcdb_read(lcdb, INT_RT_STATUS_REG, &val);
		if (ret)
			return ret;
		if (val & VREG_OK_RT_STS_BIT)
			return 0;
		usleep_range(LCDB_VREG_OK_TIMEOUT_US, LCDB_VREG_OK_TIMEOUT_US + 100);
	}

	return -ETIMEDOUT;
}

static int lcdb_disable_module(struct lcdb_regulator *lcdb)
{
	return lcdb_write(lcdb, LCDB_ENABLE_CTL1_REG, 0);
}

static int lcdb_regulator_enable(struct regulator_dev *rdev)
{
	struct lcdb_regulator *lcdb = rdev_get_drvdata(rdev);
	int ret;

	mutex_lock(&lcdb->lock);
	if (!lcdb->enabled) {
		ret = lcdb_enable_module(lcdb);
		if (ret) {
			mutex_unlock(&lcdb->lock);
			return ret;
		}
		lcdb->enabled = true;
	}
	mutex_unlock(&lcdb->lock);

	return 0;
}

static int lcdb_regulator_disable(struct regulator_dev *rdev)
{
	struct lcdb_regulator *lcdb = rdev_get_drvdata(rdev);
	int ret = 0;

	mutex_lock(&lcdb->lock);
	if (lcdb->enabled) {
		ret = lcdb_disable_module(lcdb);
		if (!ret)
			lcdb->enabled = false;
	}
	mutex_unlock(&lcdb->lock);

	return ret;
}

static int lcdb_regulator_is_enabled(struct regulator_dev *rdev)
{
	struct lcdb_regulator *lcdb = rdev_get_drvdata(rdev);

	return lcdb->enabled;
}

static int lcdb_regulator_set_voltage(struct regulator_dev *rdev,
				      int min_uV, int max_uV,
				      unsigned int *selector)
{
	struct lcdb_regulator *lcdb = rdev_get_drvdata(rdev);
	enum lcdb_reg_type type = (rdev == lcdb->ldo_rdev) ?
				  LCDB_LDO : LCDB_NCP;
	int voltage_mv = min_uV / 1000;
	int ret;

	ret = lcdb_reg_from_voltage(voltage_mv);
	if (ret < 0)
		return ret;
	if (selector)
		*selector = ret;

	mutex_lock(&lcdb->lock);
	if (type == LCDB_LDO)
		lcdb->ldo_mv = voltage_mv;
	else
		lcdb->ncp_mv = voltage_mv;

	ret = lcdb_program_output_voltage(lcdb, type);
	if (!ret)
		ret = lcdb_program_bst_voltage(lcdb);
	mutex_unlock(&lcdb->lock);

	return ret;
}

static int lcdb_regulator_get_voltage(struct regulator_dev *rdev)
{
	struct lcdb_regulator *lcdb = rdev_get_drvdata(rdev);
	enum lcdb_reg_type type = (rdev == lcdb->ldo_rdev) ?
				  LCDB_LDO : LCDB_NCP;
	unsigned int reg = (type == LCDB_LDO) ?
			   LCDB_LDO_OUTPUT_VOLTAGE_REG :
			   LCDB_NCP_OUTPUT_VOLTAGE_REG;
	u8 val;
	int ret;

	mutex_lock(&lcdb->lock);
	ret = lcdb_read(lcdb, reg, &val);
	mutex_unlock(&lcdb->lock);
	if (ret)
		return ret;

	return lcdb_voltage_from_reg(val) * 1000;
}

static int lcdb_regulator_list_voltage(struct regulator_dev *rdev,
				       unsigned int selector)
{
	if (selector >= 32)
		return -EINVAL;
	return lcdb_voltage_from_reg(selector) * 1000;
}

static const struct regulator_ops lcdb_regulator_ops = {
	.enable		= lcdb_regulator_enable,
	.disable	= lcdb_regulator_disable,
	.is_enabled	= lcdb_regulator_is_enabled,
	.set_voltage	= lcdb_regulator_set_voltage,
	.get_voltage	= lcdb_regulator_get_voltage,
	.list_voltage	= lcdb_regulator_list_voltage,
};

static int lcdb_register_child(struct lcdb_regulator *lcdb,
			       struct device_node *np,
			       enum lcdb_reg_type type)
{
	struct regulator_config cfg = {};
	struct regulator_desc *desc;
	const char *name = (type == LCDB_LDO) ? "lcdb_ldo" : "lcdb_ncp";

	desc = devm_kzalloc(lcdb->dev, sizeof(*desc), GFP_KERNEL);
	if (!desc)
		return -ENOMEM;

	desc->name = name;
	/* of_match 与子节点名（ldo/ncp）对应，供 of_get_regulator_init_data 匹配 */
	desc->of_match = (type == LCDB_LDO) ? "ldo" : "ncp";
	desc->owner = THIS_MODULE;
	desc->type = REGULATOR_VOLTAGE;
	desc->ops = &lcdb_regulator_ops;
	desc->n_voltages = 32;

	cfg.dev = lcdb->dev;
	cfg.driver_data = lcdb;
	cfg.of_node = np;
	cfg.init_data = of_get_regulator_init_data(lcdb->dev, np, desc);

	if (!cfg.init_data) {
		dev_err(lcdb->dev, "no init data for %s\n", name);
		return -ENOMEM;
	}

	if (type == LCDB_LDO) {
		lcdb->ldo_rdev = devm_regulator_register(lcdb->dev, desc, &cfg);
		if (IS_ERR(lcdb->ldo_rdev))
			return PTR_ERR(lcdb->ldo_rdev);
	} else {
		lcdb->ncp_rdev = devm_regulator_register(lcdb->dev, desc, &cfg);
		if (IS_ERR(lcdb->ncp_rdev))
			return PTR_ERR(lcdb->ncp_rdev);
	}

	return 0;
}

static int lcdb_regulator_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct lcdb_regulator *lcdb;
	struct device_node *child;
	struct resource *res;
	u32 headroom;
	int ret;

	lcdb = devm_kzalloc(dev, sizeof(*lcdb), GFP_KERNEL);
	if (!lcdb)
		return -ENOMEM;

	lcdb->dev = dev;
	lcdb->regmap = dev_get_regmap(dev->parent, NULL);
	if (!lcdb->regmap) {
		dev_err(dev, "failed to get parent's regmap\n");
		return -ENODEV;
	}

	/* SPMI 孙节点的 platform resource 在 #size-cells=0 下不可靠
	 * （同结构 wled 可取到而本节点取不到，原因未明）——直接读 DT
	 * reg 第一个地址 cell（SPMI 外设基址）绕过。 */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res) {
		lcdb->base = res->start;
	} else if (of_property_read_u32(dev->of_node, "reg", &lcdb->base)) {
		dev_err(dev, "missing reg\n");
		return -EINVAL;
	}

	mutex_init(&lcdb->lock);
	lcdb->ldo_mv = 5500;	/* 安全默认，实际以 DT 约束为准 */
	lcdb->ncp_mv = 5500;

	headroom = BST_HEADROOM_DEFAULT_MV;
	of_property_read_u32(dev->of_node, "qcom,bst-headroom-mv", &headroom);
	lcdb->headroom_mv = headroom;

	for_each_available_child_of_node(dev->of_node, child) {
		const char *s = NULL;

		of_property_read_string(child, "label", &s);

		if (!s) {
			/* 无 label 属性时按子节点名匹配 */
			if (of_node_name_eq(child, "ldo"))
				s = "ldo";
			else if (of_node_name_eq(child, "ncp"))
				s = "ncp";
		}
		if (!s)
			continue;

		if (!strcmp(s, "ldo")) {
			ret = lcdb_register_child(lcdb, child, LCDB_LDO);
		} else if (!strcmp(s, "ncp")) {
			ret = lcdb_register_child(lcdb, child, LCDB_NCP);
		} else {
			continue;
		}

		if (ret) {
			of_node_put(child);
			return ret;
		}
	}

	if (!lcdb->ldo_rdev || !lcdb->ncp_rdev) {
		dev_err(dev, "missing ldo/ncp child nodes\n");
		return -EINVAL;
	}

	platform_set_drvdata(pdev, lcdb);
	dev_info(dev, "PM8150L LCDB LCD bias regulator registered\n");

	return 0;
}

static const struct of_device_id lcdb_regulator_match[] = {
	{ .compatible = "qcom,pm8150l-lcdb", },
	{ }
};
MODULE_DEVICE_TABLE(of, lcdb_regulator_match);

static struct platform_driver lcdb_regulator_driver = {
	.probe = lcdb_regulator_probe,
	.driver = {
		.name = "qcom-lcdb-regulator",
		.of_match_table = lcdb_regulator_match,
	},
};
module_platform_driver(lcdb_regulator_driver);

MODULE_DESCRIPTION("PM8150L LCDB LCD bias regulator driver");
MODULE_LICENSE("GPL v2");
