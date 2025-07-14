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

uint8_t tx_buf[1500];
uint8_t rx_buf[1600];

int main() {
    for (size_t i = 0; i < 256; i++) {
        tx_buf[i] = 0xe0;
    }
    for (size_t i = 0; i < 256; i++) {
        tx_buf[256 + i] = 0xaa;
    }
    for (size_t i = 0; i < 256; i++) {
        tx_buf[512 + i] = i;
    }

    for (size_t i = 512 + 256; i < sizeof(tx_buf); i++) {
        tx_buf[i] = 0xff;
    }

    tx_buf[0] = 'k';
    tx_buf[1] = 'c';
    tx_buf[2] = '1';
    tx_buf[3] = 't';
    tx_buf[4] = 'p';
    tx_buf[5] = 'r';
    struct rfm9Xw_modem_config cfg = {
        .modem_mode = RfmModemMode_FSK,
        .fsk =
            {
                .data_mode = RfmPacketConfigDataMode_Packet,
                .bitrate = 1200,
                .carrier_freq = 434000000,
                .deviation_freq = 1000,
                .modulation_shaping = RfmModulationShaping_FSK_NoShaping,
                .pa_ramp = RfmPaRamp_15us,
                .tx_power = 12,
                .sync_word = 0b1010101010101010,
                .sync_word_len = 2,
            },
    };
    int ret = rfm9Xw_configure_modem(radio, &cfg);

    if (ret < 0) {
        LOG_ERR("Couldn't configure modem: %d", ret);
    } else {
        LOG_INF("Successfully configured modem");
    }
    // rfm9Xw_test_cw(radio, 434000000, 20, K_SECONDS(2));
    LOG_INF("recv real data");
    while (true) {
        ret = rfm9xw_receive(radio, rx_buf, sizeof(rx_buf), K_SECONDS(5));
        if (ret != 0) {
            LOG_WRN("Ret: %d", ret);
        }
    }
    // while (true) {
    // ret = rfm9xw_transmit(radio, tx_buf, sizeof(tx_buf));
    // k_msleep(1000);
    // }

    if (ret != 0) {
        LOG_ERR("Couldnt tx: %d", ret);
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
