// SPDX-License-Identifier: GPL-2.0-only
/*
 * Qualcomm ADSP sensor protection-domain keepalive
 *
 * Picasso's downstream adsprpc driver is enabled with both
 * qcom,fastrpc-adsp-audio-pdr and qcom,fastrpc-adsp-sensors-pdr.  The sensor
 * lookup registers a PDR listener for tms/servreg @ msm/adsp/sensor_pd and
 * acknowledges that domain's state indications.
 *
 * Mainline fastrpc does not implement the downstream static-PD notification
 * framework.  Until a proper sensor client exists, keep a listener for the
 * sensor PD so the ADSP sensor_process initialization handshake can complete.
 */

#include <linux/device.h>
#include <linux/err.h>
#include <linux/module.h>
#include <linux/nvmem-consumer.h>
#include <linux/platform_device.h>
#include <linux/soc/qcom/pdr.h>

struct qcom_adsp_sensor_pd {
	struct device *dev;
	struct pdr_handle *pdr;
	struct pdr_service *pds;
};

static const char *qcom_adsp_sensor_pd_state(u32 state)
{
	switch (state) {
	case SERVREG_SERVICE_STATE_DOWN:
		return "down";
	case SERVREG_SERVICE_STATE_UP:
		return "up";
	case SERVREG_SERVICE_STATE_EARLY_DOWN:
		return "early-down";
	case SERVREG_SERVICE_STATE_UNINIT:
		return "uninitialized";
	case SERVREG_LOCATOR_ERR:
		return "locator-error";
	default:
		return "unknown";
	}
}

static void qcom_adsp_sensor_pd_status(int state, char *service_path, void *priv)
{
	struct qcom_adsp_sensor_pd *sensor_pd = priv;

	dev_info(sensor_pd->dev,
		 "sensor PD %s state %s (0x%08x); PDR core will acknowledge indication\n",
		 service_path, qcom_adsp_sensor_pd_state(state), state);
}

static int qcom_adsp_sensor_pd_log_variant(struct device *dev)
{
	struct nvmem_cell *cell;
	size_t len = 0;
	u8 variant = 0;
	int ret;
	void *value;

	cell = devm_nvmem_cell_get(dev, "adsp_variant");
	if (IS_ERR(cell)) {
		ret = PTR_ERR(cell);
		if (ret == -EPROBE_DEFER)
			return ret;

		dev_dbg(dev, "adsp_variant cell unavailable: %d\n", ret);
		return 0;
	}

	value = nvmem_cell_read(cell, &len);
	if (IS_ERR(value)) {
		dev_dbg(dev, "adsp_variant read failed: %ld\n",
			PTR_ERR(value));
		return 0;
	}

	if (len >= 1)
		variant = ((u8 *)value)[0];
	kfree(value);

	dev_info(dev, "adsp_variant fuse value: 0x%02x\n", variant);

	return 0;
}

static int qcom_adsp_sensor_pd_probe(struct platform_device *pdev)
{
	struct qcom_adsp_sensor_pd *sensor_pd;
	int ret;

	sensor_pd = devm_kzalloc(&pdev->dev, sizeof(*sensor_pd), GFP_KERNEL);
	if (!sensor_pd)
		return -ENOMEM;

	sensor_pd->dev = &pdev->dev;
	platform_set_drvdata(pdev, sensor_pd);

	ret = qcom_adsp_sensor_pd_log_variant(sensor_pd->dev);
	if (ret)
		return ret;

	sensor_pd->pdr = pdr_handle_alloc(qcom_adsp_sensor_pd_status,
					  sensor_pd);
	if (IS_ERR(sensor_pd->pdr))
		return dev_err_probe(sensor_pd->dev, PTR_ERR(sensor_pd->pdr),
				     "failed to allocate PDR handle\n");

	sensor_pd->pds = pdr_add_lookup(sensor_pd->pdr, "tms/servreg",
					"msm/adsp/sensor_pd");
	if (IS_ERR(sensor_pd->pds)) {
		dev_err(sensor_pd->dev,
			"failed to add sensor PD lookup: %ld\n",
			PTR_ERR(sensor_pd->pds));
		pdr_handle_release(sensor_pd->pdr);
		sensor_pd->pdr = NULL;
		return PTR_ERR(sensor_pd->pds);
	}

	dev_info(sensor_pd->dev,
		 "tracking tms/servreg -> msm/adsp/sensor_pd (downstream sensors PDR equivalent)\n");

	return 0;
}

static void qcom_adsp_sensor_pd_remove(struct platform_device *pdev)
{
	struct qcom_adsp_sensor_pd *sensor_pd = platform_get_drvdata(pdev);

	pdr_handle_release(sensor_pd->pdr);
}

static const struct of_device_id qcom_adsp_sensor_pd_of_match[] = {
	{ .compatible = "qcom,adsp-sensor-pd" },
	{ }
};
MODULE_DEVICE_TABLE(of, qcom_adsp_sensor_pd_of_match);

static struct platform_driver qcom_adsp_sensor_pd_driver = {
	.probe = qcom_adsp_sensor_pd_probe,
	.remove = qcom_adsp_sensor_pd_remove,
	.driver = {
		.name = "qcom-adsp-sensor-pd",
		.of_match_table = qcom_adsp_sensor_pd_of_match,
	},
};
module_platform_driver(qcom_adsp_sensor_pd_driver);

MODULE_DESCRIPTION("Qualcomm ADSP sensor protection-domain keepalive");
MODULE_LICENSE("GPL");
