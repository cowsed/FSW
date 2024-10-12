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
    controller.SubmitEvent(Sources::Barom1, Events::PadReady);
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

int main() {
    LOG_DBG("Waiting until everyone ready");
    controller.SubmitEvent(Sources::IMU2, Events::PadReady); // mocking this cuz i cant be bothered to double it

    controller.WaitUntilEvent(Events::PadReady);
    LOG_DBG("System ready:\n\tstart boost detecting");

    // Start sensing
    k_timer_start(&imu_timer, K_MSEC(1), K_MSEC(1));
    k_timer_start(&barom_timer, K_MSEC(10), K_MSEC(10));

    // Mocked Boost Detect
    {
        controller.SubmitEvent(Sources::IMU1, Events::Boost);
        controller.SubmitEvent(Sources::IMU2, Events::Boost);
        controller.SubmitEvent(Sources::Barom1, Events::Boost);
    }

    controller.WaitUntilEvent(Events::Boost);
    LOG_DBG("Boost detected:\n\ttell your friends (engineering cams)");

    controller.WaitUntilEvent(Events::Coast);
    LOG_DBG("Coast detected:\n\tturn down IMU data rate");

    // IMU can chill out
    k_timer_start(&imu_timer, K_MSEC(10), K_MSEC(10));

    // Mock Noseover
    controller.SubmitEvent(Sources::Barom1, Events::Noseover);

    controller.WaitUntilEvent(Events::Noseover);
    LOG_DBG("Noseover detected:\n\tdeploy charges");

    controller.WaitUntilEvent(Events::MainChute);
    LOG_DBG("Main Chute Deploy:\n\tdeploy more charges");

    controller.WaitUntilEvent(Events::GroundHit);
    LOG_DBG("Hit The ground: Stop\n\trecording data");

    // Stop recording
    k_timer_stop(&imu_timer);
    k_timer_stop(&barom_timer);

    controller.WaitUntilEvent(Events::CamerasOff);
    LOG_DBG("Flight over:\n\tTurn off cameras");
    return 0;
}
