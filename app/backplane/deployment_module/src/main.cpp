#include "linear_fit.hpp"
#include "types.h"

#include <f_core/util/debouncer.hpp>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/settings/settings.h>
#include <zephyr/sys/reboot.h>
#include <zephyr/zbus/zbus.h>
LOG_MODULE_DECLARE(dep_mod);

using SampleType = LinearFitSample<FeetPerSec>;

static constexpr std::size_t gl_window_size = 10;
using GLAvger = MovingAvg<Feet, gl_window_size>;

static constexpr std::size_t window_size = 10;
using SummerType = RollingSum<SampleType, window_size>;

using NoseoverDebouncerT = Debuouncer<ThresholdDirection::Under, Scalar>;
using MainHeightDebouncer = Debuouncer<ThresholdDirection::Under, Scalar>;
using GroundLevelDebouncer = Debuouncer<ThresholdDirection::Under, Scalar>;

Feet calc_alt(KiloPa press_kpa, Celsius temp_c) {
    Scalar pressure = press_kpa * 10;
    Scalar altitude = (1 - pow(pressure / 1'013.25, 0.190284)) * 145'366.45;
    return altitude;
}
static Phase current_phase = Phase::Pad;

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

static void do_state_transition(const struct zbus_channel *chan) { printk("who up transitioning they state"); }
ZBUS_LISTENER_DEFINE(state_transition_lis, do_state_transition);
bool event_validator(const void *msg, size_t msg_size) {
    auto ev = (Timestamped<Event> *) msg;
    printk("Sending State: %d from %d\n", (int) ev->value.typ, (int) ev->value.source);
    if (ev->value.typ == EventType::Noseover) {
        current_phase = Phase::UnderDrogue;
    } else if (ev->value.typ == EventType::Main) {
        current_phase = Phase::UnderMain;
    }

    return false;
}

ZBUS_CHAN_DEFINE(events_chan,                          /* Name */
                 Timestamped<Event>,                   /* Message type */
                 event_validator,                      /* Validator */
                 NULL,                                 /* User Data */
                 ZBUS_OBSERVERS(state_transition_lis), /* observers */
                 ZBUS_MSG_INIT(Timestamped<Event>{0,
                                                  Event{EventType::TurnOn, SensorType::Other}}) /* Initial value {0} */
);

const struct device *imu = DEVICE_DT_GET_ONE(openrocket_imu);
const struct device *barom = DEVICE_DT_GET(DT_ALIAS(barom));

struct AltimCBData {
    const char *label;
    SensorType self;

    GLAvger ground_level_avger;

    SummerType line_fitting_summer;
    NoseoverDebouncerT noseover_debouncer;
    int summer_samples = 0; // we need to get at least window_size samples before any of our data is valid

    MainHeightDebouncer main_debouncer;

    GroundLevelDebouncer end_of_flight_debouncer;
};

static void line_fitting_cb(const struct zbus_channel *chan) {
    const Timestamped<altim_telem> *telem = (Timestamped<altim_telem> *) zbus_chan_const_msg(chan);
    AltimCBData *dat = (AltimCBData *) chan->user_data;

    Milliseconds_u32 time_ms = telem->time;
    Scalar time = time_ms / 1000.0; // we want velocity in ft/s
    Scalar asl_alt = calc_alt(telem->value.press, telem->value.temp);
    Feet agl_alt = asl_alt - dat->ground_level_avger.avg();

    dat->line_fitting_summer.feed(SampleType{time, agl_alt});

    if (current_phase == Phase::Pad) {
        dat->ground_level_avger.feed(asl_alt);

        // do a check for boostin
        dat->end_of_flight_debouncer =
            GroundLevelDebouncer{5000, dat->ground_level_avger.avg() + 100}; // TODO use not magic numbers for this
        // ALSO TODO update debouncer to have in range and stable (value doesnt differ by range in time period)

        // also solves boost
        // boost is when altitude is not stable by 500ft in 2 seconds
        //

        return;
    }
    if (current_phase == Phase::UnderMain) {
        dat->main_debouncer.feed(time_ms, agl_alt);
        if (dat->main_debouncer.passed()) {
            Event ev = {.typ = EventType::Main, .source = dat->self};
            Timestamped<Event> tev = {.time = time_ms, .value = ev};
            int err = zbus_chan_pub(&events_chan, &tev, K_SECONDS(1));
            if (err != 0) {
                LOG_ERR("Failed to send main event");
            }
        }
        return;
    }
    // line fit is invalid until we have window_size samples
    dat->summer_samples++;
    if (dat->summer_samples < window_size + 1) {
        return;
    }

    Line l = find_line(dat->line_fitting_summer);
    FeetPerSec vel = l.m;

    dat->noseover_debouncer.feed(time_ms, vel);

    // we nosen over
    if (current_phase == Phase::Coasting && dat->noseover_debouncer.passed()) {
        Event ev;
        ev.source = dat->self;
        ev.typ = EventType::Noseover;
        Timestamped<Event> tev = {.time = time_ms, .value = ev};
        printk("here\n");

        int err = zbus_chan_pub(&events_chan, &tev, K_SECONDS(1));
        if (err != 0) {
            LOG_ERR("Failed to write noseover event");
        }
    }

    // LOG_INF("From listener -> telem p=%f, t=%f", (double) telem->value.press, (double) telem->value.temp);
}

ZBUS_LISTENER_DEFINE(bme_lis, line_fitting_cb);

AltimCBData bmecbd{
    .label = "bme280",
    .self = SensorType::BME280,
    .ground_level_avger = GLAvger{0.0},
    .line_fitting_summer = RollingSum<SampleType, window_size>{LinearFitSample<Scalar>{}},
    .noseover_debouncer = NoseoverDebouncerT{0, 0},
    .main_debouncer = MainHeightDebouncer{0, 0},
    .end_of_flight_debouncer = GroundLevelDebouncer{0, 0}, // TODO make these defaults
};

ZBUS_CHAN_DEFINE(bme_telem_chan,                                               /* Name */
                 Timestamped<altim_telem>,                                     /* Message type */
                 NULL,                                                         /* Validator */
                 &bmecbd,                                                      /* User Data */
                 ZBUS_OBSERVERS(bme_lis),                                      /* observers */
                 ZBUS_MSG_INIT(Timestamped<altim_telem>{0, altim_telem{0, 0}}) /* Initial value {0} */
);

int main() {
    Feet main_height = 500;
    Milliseconds_u32 main_time = 100;

    FeetPerSec noseover_velocity = 10;
    Milliseconds_u32 noseover_time = 100;

    bmecbd.noseover_debouncer = NoseoverDebouncerT{noseover_time, noseover_velocity};
    bmecbd.main_debouncer = MainHeightDebouncer{main_time, main_height};

    int i = 0;
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
        if (ms > 3860 && current_phase == Phase::Boostin) {
            current_phase = Phase::Coasting;
        } else if (ms > 2000 && current_phase == Phase::Pad) {
            current_phase = Phase::Boostin;
        }

        Timestamped<altim_telem> telem = {};
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
