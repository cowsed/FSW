#include "linear_fit.hpp"
#include "types.h"

#include <f_core/util/debouncer.hpp>
#include <math.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/settings/settings.h>
#include <zephyr/sys/reboot.h>
#include <zephyr/zbus/zbus.h>
LOG_MODULE_DECLARE(dep_mod);

using SampleType = LinearFitSample<FeetPerSec_f32>;
static constexpr std::size_t window_size = 10;

using SummerType = RollingSum<SampleType, window_size>;
using NoseoverDebouncerT = Debuouncer<ThresholdDirection::Under, Scalar>;

Line find_line(const SummerType &summer) {
    std::size_t N = summer.size();
    SampleType E = summer.sum();
    Scalar denom = (N * E.xx - E.x * E.x);
    if (denom == 0) {
        // printf("Would have divided by 0\n");
        return {0, 0};
    }
    Scalar m = (N * E.xy - E.x * E.y) / denom;
    Scalar b = (E.y - m * E.x) / N;
    return {m, b};
}

const struct device *imu = DEVICE_DT_GET_ONE(openrocket_imu);
const struct device *barom = DEVICE_DT_GET(DT_ALIAS(barom));

struct LineFittingCBData {
    const char *label;
    SummerType summer;
    NoseoverDebouncerT noseover_debouncer;
    int samples = 0; // we need to get at least window_size samples before any of our data is valid
};

Scalar calc_alt(float press_kpa, float temp_c) {
    Scalar pressure = press_kpa * 10;
    Scalar altitude = (1 - pow(pressure / 1'013.25, 0.190284)) * 145'366.45;
    return altitude;
}

static void line_fitting_cb(const struct zbus_channel *chan) {
    const Timestamped<altim_telem> *telem = (Timestamped<altim_telem> *) zbus_chan_const_msg(chan);
    LineFittingCBData *dat = (LineFittingCBData *) chan->user_data;

    Scalar time_ms = telem->time;
    Scalar time = time_ms / 1000.0; // we want velocity in ft/s
    Scalar alt = calc_alt(telem->value.press, telem->value.temp);

    dat->summer.feed(SampleType{time, alt});
    Line l = find_line(dat->summer);
    Scalar vel = l.m;
    dat->noseover_debouncer.feed(time_ms, vel);
    dat->samples++;

    if (dat->samples < window_size + 1) {
        return;
    }
    // if (dat->nodeover_debouncer.passed()) {
    // printk("PASSED\n");
    // }
    if (dat->samples % 10 == 0) {
        printk("%f, %f, %f, %d\n", time, alt, (double) vel, (int) dat->noseover_debouncer.passed());
    }

    // LOG_INF("From listener -> telem p=%f, t=%f", (double) telem->value.press, (double) telem->value.temp);
}

ZBUS_LISTENER_DEFINE(bme_lis, line_fitting_cb);

Milliseconds_u32 noseover_time = 100;
FeetPerSec_f32 noseover_velocity = 0.5;

LineFittingCBData bmecbd{
    .label = "bme280",
    .summer = RollingSum<SampleType, window_size>{},
    .noseover_debouncer = NoseoverDebouncerT{noseover_time, noseover_velocity},
};
// LineFittingCBData mscbd{
// .label = "ms5611",
// .summer = RollingSum<SampleType, window_size>{},
// };

ZBUS_CHAN_DEFINE(bme_telem_chan,           /* Name */
                 Timestamped<altim_telem>, /* Message type */

                 NULL,                                                         /* Validator */
                 &bmecbd,                                                      /* User Data */
                 ZBUS_OBSERVERS(bme_lis),                                      /* observers */
                 ZBUS_MSG_INIT(Timestamped<altim_telem>{0, altim_telem{0, 0}}) /* Initial value {0} */
);
// ZBUS_CHAN_DEFINE(bme_telem_chan,         /* Name */
//  Timestamped<bme_telem>, /* Message type */
//
//  NULL,                                                     /* Validator */
//  &mscbd,                                                   /* User Data */
//  ZBUS_OBSERVERS(foo_lis),                                  /* observers */
//  ZBUS_MSG_INIT(Timestamped<bme_telem>{0, bme_telem{0, 0}}) /* Initial value {0} */
// );

int main() {

    Debuouncer<ThresholdDirection::Under, Scalar, uint32_t> db{100, 0.2};
    int i = 0;
    Timestamped<altim_telem> telem = {};
    while (true) {

        struct sensor_value pressure;
        struct sensor_value temp;
        int err = sensor_sample_fetch(barom);
        if (err < 0) {
            printk("err fetching");
        }
        err = sensor_channel_get(barom, SENSOR_CHAN_PRESS, &pressure);
        if (err < 0) {
            printk("err getting");
        }
        err = sensor_channel_get(barom, SENSOR_CHAN_AMBIENT_TEMP, &temp);
        if (err < 0) {
            printk("err getting");
        }

        uint64_t ms = k_uptime_get();

        telem.time = ms;
        telem.value.press = sensor_value_to_float(&pressure);
        telem.value.temp = sensor_value_to_float(&temp);
        int got = zbus_chan_pub(&bme_telem_chan, &telem, K_SECONDS(1));
        if (got != 0) {
            printk("Err: %d\n", got);
        }
        i++;
        // printk("published\n");
        k_msleep(10);
    }
}
