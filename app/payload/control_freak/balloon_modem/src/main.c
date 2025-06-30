#include "../drivers/modem/rfm9Xw/rfm9Xw.h"
#include "f_core/radio/protocols/horus/horus.h"

#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <zephyr/drivers/gnss.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/lora.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/fs/fs.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/shell/shell.h>
#include <zephyr/types.h>
LOG_MODULE_REGISTER(main);

#define RADIO_NODE DT_ALIAS(lora0)
static const struct device *radio = DEVICE_DT_GET(RADIO_NODE);

const uint32_t carrier = 432950000;

static const struct gpio_dt_spec ldo_en = GPIO_DT_SPEC_GET(DT_ALIAS(ldo5v), gpios);
const struct gpio_dt_spec buzzer = GPIO_DT_SPEC_GET(DT_ALIAS(buzz), gpios);
int main() {
    struct rfm9Xw_modem_config cfg = {
        .modem_mode = RfmModemMode_FSK,
        .fsk =
            {
                .data_mode = RfmPacketConfigDataMode_Packet,
                .bitrate = 9600,
                .carrier_freq = 434000000,
                .deviation_freq = 1000,
                .modulation_shaping = RfmModulationShaping_FSK_NoShaping,
                .pa_ramp = RfmPaRamp_15us,
                .tx_power = 20,
                .sync_word = 0b1010101010101010,
                .sync_word_len = 2,
            },
    };
    int ret = rfm9Xw_configure_modem(radio, &cfg);

    rfm9Xw_test_cw(radio, 434000000, 20, K_SECONDS(4));

    if (ret < 0) {
        LOG_ERR("Couldn't configure modem: %d", ret);
    } else {
        LOG_INF("Successfully configured modem");
    }

    while (true) {
        int8_t c = 0;
        int ret = rfm9xw_read_temperature(radio, &c);
        if (ret != 0) {
            LOG_WRN_ONCE("Failed to read temp: %d", ret);
        } else {
            LOG_INF("Temp: %d", (int) c);
        }
        k_msleep(1000);
    }
}
