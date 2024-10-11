// F-Core Includes
#include "f_core/util/debouncer.hpp"

#include <f_core/device/sensor/c_accelerometer.h>
#include <f_core/device/sensor/c_barometer.h>
#include <f_core/device/sensor/c_gyroscope.h>
#include <f_core/device/sensor/c_magnetometer.h>

// Zephyr Includes
#include "f_core/flight/CPhaseController.h"

#include <array>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>
void imu_thread(void *controller_vp, void *, void *) {
    while (true) {
    }
}
void barom_thread(void *, void *, void *) {}

int main() {
    CAccelerometer imu_accelerometer(*DEVICE_DT_GET_ONE(openrocket_imu));
    CGyroscope imu_gyroscope(*DEVICE_DT_GET_ONE(openrocket_imu));
    CBarometer barometer(*DEVICE_DT_GET_ONE(openrocket_barometer));
    CMagnetometer magnetometer(*DEVICE_DT_GET_ONE(openrocket_magnetometer));

    CSensorDevice *sensors[] = {&imu_accelerometer, &imu_gyroscope, &barometer, &magnetometer};

    while (1) {
        printk("Hello");
        k_msleep(10000);
    }

    return 0;
}
