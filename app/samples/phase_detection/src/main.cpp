// F-Core Includes
#include "f_core/util/debouncer.hpp"

#include <f_core/device/sensor/c_accelerometer.h>
#include <f_core/device/sensor/c_barometer.h>
#include <f_core/device/sensor/c_gyroscope.h>
#include <f_core/device/sensor/c_magnetometer.h>
#include <zephyr/kernel.h>

// Zephyr Includes
#include "flight.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main, CONFIG_APP_PHASE_DETECT_LOG_LEVEL);

#include <array>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>

K_TIMER_DEFINE(imu_timer, NULL, NULL);
K_TIMER_DEFINE(barom_timer, NULL, NULL);

void imu_thread_f(void *, void *, void *) {
    CAccelerometer acc(*DEVICE_DT_GET_ONE(openrocket_imu));
    if (!acc.IsReady()) {
        LOG_WRN("Accelerometer not ready");
    }
    controller.SubmitEvent(Sources::IMU1, Events::PadReady);
    controller.WaitUntilEvent(Events::PadReady);

    while (true) {
        k_timer_status_sync(&imu_timer);
        bool good = acc.UpdateSensorValue();
        if (!good) {
            LOG_ERR("Failure reading imu");
        }
    }
}
K_THREAD_DEFINE(imu_thread, 1024, imu_thread_f, NULL, NULL, NULL, 0, 0, 0);

void barom_thread_f(void *, void *, void *) {
    CBarometer barometer(*DEVICE_DT_GET_ONE(openrocket_barometer));
    if (!barometer.IsReady()) {
        LOG_WRN("Accelerometer not ready");
    }
    controller.SubmitEvent(Sources::BAROM1, Events::PadReady);
    controller.WaitUntilEvent(Events::PadReady);

    while (true) {
        k_timer_status_sync(&barom_timer);
        bool good = barometer.UpdateSensorValue();
        if (!good) {
            LOG_ERR("Failure reading barometer");
        }
    }
}
K_THREAD_DEFINE(barom_thread, 1024, barom_thread_f, NULL, NULL, NULL, 0, 0, 0);

// TODO, allow for timers between states to be included in Controller
// StartEvent, Time, FinishedEvent
// At time of StartEvent + Time, send Finished Event signal
// mabye add Source to that so you can name and differentiate things that way
// coast only happens after boost with lockout of 1second
// Timer for boost ending
constexpr k_timeout_t boost_to_coast_time = K_SECONDS(3);
void boost_to_coast_timer_expire(struct k_timer *) { controller.SubmitEvent(Sources::TIMERS, Events::Coast); }
K_TIMER_DEFINE(boost_to_coast_timer, boost_to_coast_timer_expire, NULL);

// Timer for noseover to main
constexpr k_timeout_t noseover_to_main_time = K_SECONDS(5);
void noseover_to_main_timer_expire(struct k_timer *) { controller.SubmitEvent(Sources::TIMERS, Events::MainChute); }
K_TIMER_DEFINE(noseover_to_main_timer, noseover_to_main_timer_expire, NULL);

// Full Flight Time
constexpr k_timeout_t full_flight_time = K_SECONDS(10);
void full_flight_timer_expire(struct k_timer *) { controller.SubmitEvent(Sources::TIMERS, Events::GroundHit); }
K_TIMER_DEFINE(full_flight_timer, full_flight_timer_expire, NULL);

// Extra Camera Time
constexpr k_timeout_t extra_camera_time = K_SECONDS(2);
void extra_camera_timer_expire(struct k_timer *) { controller.SubmitEvent(Sources::TIMERS, Events::FlightOver); }
K_TIMER_DEFINE(extra_camera_timer, extra_camera_timer_expire, NULL);

int main() {
    LOG_DBG("Waiting until everyone ready");
    controller.SubmitEvent(Sources::IMU2, Events::PadReady); // mocking this cuz i cant be bothered to double it

    controller.WaitUntilEvent(Events::PadReady);
    LOG_DBG("System ready");

    k_timer_start(&imu_timer, K_MSEC(1), K_MSEC(1));
    k_timer_start(&barom_timer, K_MSEC(10), K_MSEC(10));

    // Mocked Boost Detect
    {
        controller.SubmitEvent(Sources::IMU1, Events::Boost);
        controller.SubmitEvent(Sources::IMU2, Events::Boost);
        controller.WaitUntilEvent(Events::Boost);
    }

    LOG_DBG("Boost detected");
    k_timer_start(&full_flight_timer, full_flight_time, K_NO_WAIT);
    k_timer_start(&boost_to_coast_timer, boost_to_coast_time, K_NO_WAIT);

    controller.WaitUntilEvent(Events::Coast);
    LOG_DBG("Coast detected");

    // IMU can chill out
    k_timer_start(&imu_timer, K_MSEC(10), K_MSEC(10));

    // Mock Noseover
    controller.SubmitEvent(Sources::BAROM1, Events::Noseover);

    controller.WaitUntilEvent(Events::Noseover);
    k_timer_start(&noseover_to_main_timer, noseover_to_main_time, K_NO_WAIT);
    LOG_DBG("Noseover detected");

    k_timer_start(&noseover_to_main_timer, noseover_to_main_time, K_NO_WAIT);

    controller.WaitUntilEvent(Events::MainChute);
    LOG_DBG("Main Chute Deploy");

    controller.WaitUntilEvent(Events::GroundHit);
    LOG_DBG("Hit The ground");
    k_timer_start(&extra_camera_timer, extra_camera_time, K_NO_WAIT);

    // Can stop recording
    k_timer_stop(&imu_timer);
    k_timer_stop(&barom_timer);

    controller.WaitUntilEvent(Events::FlightOver);
    LOG_DBG("Flight over ");
    return 0;
}
