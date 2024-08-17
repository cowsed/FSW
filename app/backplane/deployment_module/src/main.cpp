#include "linear_fit.hpp"

#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>

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

int main() {

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
        k_msleep(10);
    }
}