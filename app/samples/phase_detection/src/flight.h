#include <array>
#include <cstdint>
#include <f_core/flight/CPhaseController.h>

enum Sources : uint8_t {
    IMU1 = 0,
    IMU2 = 1,
    BAROM1 = 2,
    BAROM2 = 3,
    GNSS = 4,
    TIMERS = 5,
    NumSources = 6,
};

constexpr std::array<const char *, Sources::NumSources> sourceNames = {
    "IMU 1 (LSM6DSL)", "IMU 2 (ADXL375)", "Barom 1 (BME280)", "Barom 2 (MS5611)", "GNSS", "Timer",
};

enum Events : uint8_t {
    PadReady = 0,
    Boost = 1,
    Coast = 2,
    Noseover = 3,
    MainChute = 4,
    GroundHit = 5,
    FlightOver = 6,
    NumEvents = 7
};

constexpr std::array<const char *, Events::NumEvents> eventNames = {
    "PadReady", "Boost", "Coast", "Noseover", "MainChute", "GroundHit", "FlightOver",
};
using Controller = PhaseController<Events, Events::NumEvents, Sources, Sources::NumSources>;

constexpr std::array<Controller::DecisionFunc, Events::NumEvents> deciders = [] {
    std::array<Controller::DecisionFunc, Events::NumEvents> arr = {nullptr};
    // Ready to go
    arr[Events::PadReady] = [](Controller::SourceStates states) -> bool {
        return states[Sources::IMU1] && states[Sources::IMU2] && states[Sources::BAROM1] && states[Sources::BAROM2] &&
               states[Sources::GNSS];
    };
    // Boosting
    arr[Events::Boost] = [](Controller::SourceStates states) -> bool {
        return (states[Sources::IMU1] && states[Sources::IMU2]) || states[Sources::BAROM1] || states[Sources::BAROM2] ||
               states[Sources::GNSS];
    };
    // Coasting
    arr[Events::Coast] = [](Controller::SourceStates states) -> bool {
        return states[Sources::IMU1] || states[Sources::IMU2];
    };
    // Noseover
    arr[Events::Noseover] = [](Controller::SourceStates states) -> bool {
        return states[Sources::BAROM1] || states[Sources::BAROM2];
    };
    // Main
    arr[Events::MainChute] = [](Controller::SourceStates states) -> bool {
        return states[Sources::BAROM1] || states[Sources::BAROM2] || states[Sources::TIMERS];
    };

    // On the ground
    arr[Events::GroundHit] = [](Controller::SourceStates states) -> bool {
        return states[Sources::BAROM1] || states[Sources::BAROM2] || states[Sources::TIMERS];
    };
    // After finishing stuff up
    arr[Events::GroundHit] = [](Controller::SourceStates states) -> bool { return states[Sources::TIMERS]; };

    return arr;
}();

Controller controller{sourceNames, eventNames, deciders};
