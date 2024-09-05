#include "linear_fit.hpp"

#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>
#include <zephyr/settings/settings.h>
#include <zephyr/sys/reboot.h>

using Scalar = double;
using SampleType = LinearFitSample<Scalar>;
static constexpr std::size_t window_size = 50;
RollingSum<SampleType, window_size> summer;

struct Line {
    Scalar m;
    Scalar b;
};

Line find_line() {
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

#define DEFAULT_ACCEL_BOOST_DETECT_WINDOW_SIZE_MS 250

static uint32_t accel_boost_detect_window_size_ms = DEFAULT_ACCEL_BOOST_DETECT_WINDOW_SIZE_MS;

static int detect_settings_set(const char *name, size_t len, settings_read_cb read_cb, void *cb_arg) {
    const char *next;
    int rc;

    if (settings_name_steq(name, "bar", &next) && !next) {
        if (len != sizeof(accel_boost_detect_window_size_ms)) {
            return -EINVAL;
        }

        rc = read_cb(cb_arg, &accel_boost_detect_window_size_ms, sizeof(accel_boost_detect_window_size_ms));
        if (rc >= 0) {
            return 0;
        }

        return rc;
    }

    return -ENOENT;
}

struct settings_handler my_conf = {.name = "detect", .h_set = detect_settings_set};

int main() {

    settings_subsys_init();
    settings_register(&my_conf);
    settings_load();

    // foo_val++;
    // settings_save_one("detect/bar", &foo_val, sizeof(foo_val));

    printk("detect acell win size ms: %d\n", accel_boost_detect_window_size_ms);

    k_msleep(100000);
    sys_reboot(SYS_REBOOT_COLD);
    return 0;
    while (true) {
        struct sensor_value pressure;
        int err = sensor_sample_fetch(barom);
        if (err < 0) {
            printk("err fetching");
        }
        sensor_channel_get(barom, SENSOR_CHAN_PRESS, &pressure);
        if (err < 0) {
            printk("err getting");
        }
        double p = sensor_value_to_double(&pressure);
        Scalar s = p;
        uint64_t ms = k_uptime_get();
        Scalar t = (Scalar) ms / 1000.0;
        summer.feed(SampleType{t, s});
        Line l = find_line();
        printk("%f %f %f %f\n", (double) t, (double) s, (double) l.m, (double) l.b);
        k_msleep(1000);
    }
}