// SPDX-License-Identifier: GPL-2.0-only
/*
 * Qualcomm SMP2P sleep-state client
 *
 * Qualcomm sensor firmware can use the ADSP "sleepstate_see" SMP2P entry to
 * learn whether apps is awake.  Downstream Android always marks apps awake
 * during boot.  Preserve that contract for boards which boot sensor firmware
 * integrated in the ADSP image.
 */

#include <linux/bitops.h>
#include <linux/err.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/of_irq.h>
#include <linux/platform_device.h>
#include <linux/soc/qcom/smem_state.h>

#define QCOM_SLEEPSTATE_AWAKE_BIT	12

struct qcom_sleepstate {
	struct device *dev;
	struct qcom_smem_state *state;
	unsigned int dt_bit;
	u32 awake_bit;
};

static irqreturn_t qcom_sleepstate_handler(int irq, void *ctxt)
{
	struct qcom_sleepstate *sleepstate = ctxt;

	dev_info(sleepstate->dev,
		 "sleepstate_see edge irq=%d\n", irq);

	return IRQ_HANDLED;
}

static int qcom_sleepstate_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct qcom_sleepstate *sleepstate;
	int irq;
	int ret;

	sleepstate = devm_kzalloc(dev, sizeof(*sleepstate), GFP_KERNEL);
	if (!sleepstate)
		return -ENOMEM;

	sleepstate->dev = dev;
	sleepstate->state = devm_qcom_smem_state_get(dev, NULL,
						     &sleepstate->dt_bit);
	if (IS_ERR(sleepstate->state))
		return dev_err_probe(dev, PTR_ERR(sleepstate->state),
				     "failed to get sleepstate state\n");

	/* Downstream ignores the legacy state argument and updates bit 12. */
	sleepstate->awake_bit = QCOM_SLEEPSTATE_AWAKE_BIT;

	irq = of_irq_get_byname(dev->of_node, "smp2p-sleepstate-in");
	if (irq <= 0) {
		ret = irq < 0 ? irq : -ENODEV;
		dev_err_probe(dev, ret, "failed to get sleepstate irq\n");
		return ret;
	}

	ret = devm_request_threaded_irq(dev, irq, NULL,
					qcom_sleepstate_handler,
					IRQF_ONESHOT | IRQF_TRIGGER_RISING,
					"qcom_sleepstate", sleepstate);
	if (ret) {
		dev_err(dev, "failed to request sleepstate irq %d: %d\n",
			irq, ret);
		return ret;
	}

	platform_set_drvdata(pdev, sleepstate);

	/* Downstream announces apps as awake as soon as its client probes. */
	qcom_smem_state_update_bits(sleepstate->state,
				    BIT(sleepstate->awake_bit),
				    BIT(sleepstate->awake_bit));
	dev_info(dev,
		 "ADSP sleepstate ready: DT state bit %u, apps-awake bit %u asserted\n",
		 sleepstate->dt_bit, sleepstate->awake_bit);

	return 0;
}

static void qcom_sleepstate_remove(struct platform_device *pdev)
{
	struct qcom_sleepstate *sleepstate = platform_get_drvdata(pdev);

	qcom_smem_state_update_bits(sleepstate->state,
				    BIT(sleepstate->awake_bit), 0);
}

static const struct of_device_id qcom_sleepstate_of_match[] = {
	{ .compatible = "qcom,smp2p-sleepstate" },
	{ }
};
MODULE_DEVICE_TABLE(of, qcom_sleepstate_of_match);

static struct platform_driver qcom_sleepstate_driver = {
	.probe = qcom_sleepstate_probe,
	.remove = qcom_sleepstate_remove,
	.driver = {
		.name = "qcom-smp2p-sleepstate",
		.of_match_table = qcom_sleepstate_of_match,
	},
};
module_platform_driver(qcom_sleepstate_driver);

MODULE_DESCRIPTION("Qualcomm SMP2P sleep-state client");
MODULE_LICENSE("GPL");
