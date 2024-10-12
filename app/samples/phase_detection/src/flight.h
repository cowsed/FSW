#pragma once
#include <array>
#include <cstdint>
#include <f_core/flight/CPhaseController.h>

/**
 * Flight Events for a fictional flight
 */
enum Events : uint8_t { PadReady, Boost, Coast, Noseover, MainChute, GroundHit, CamerasOff, NumEvents };
constexpr std::array<const char *, Events::NumEvents> eventNames = {
    "PadReady", "Boost", "Coast", "Noseover", "MainChute", "GroundHit", "CamerasOff",
};

/**
 * Sources of flight events
 */
enum Sources : uint8_t {
    IMU1,
    IMU2,
    Barom1,
    Boost2CoastTimer,
    Noseover2MainTimer,
    FullFlightTimer,
    ExtraCameraTimer,
    NumSources
};
constexpr std::array<const char *, Sources::NumSources> sourceNames = {
    "IMU 1 (LSM6DSL)",     "IMU 2 (ADXL375)",   "Barom 1 (BME280)",  "Boost2Coast Timer",
    "Noseover2Main Timer", "Full Flight Timer", "ExtraCamera Timer",
};

constexpr std::size_t num_timer_events = 4;
using Controller = PhaseController<Events, Events::NumEvents, Sources, Sources::NumSources, num_timer_events>;

/**
 * Special events triggered not by sensors but by timers between phases
 */
constexpr std::array<Controller::TimerEvent, num_timer_events> timer_events = {
    Controller::TimerEvent{
        .start = Events::Boost,
        .event = Events::Coast,
        .time = K_SECONDS(3),
        .source = Sources::Boost2CoastTimer,
    },
    Controller::TimerEvent{
        .start = Events::Noseover,
        .event = Events::MainChute,
        .time = K_SECONDS(5),
        .source = Sources::Noseover2MainTimer,
    },
    Controller::TimerEvent{
        .start = Events::Boost,
        .event = Events::GroundHit,
        .time = K_SECONDS(10),
        .source = Sources::FullFlightTimer,
    },
    Controller::TimerEvent{
        .start = Events::GroundHit,
        .event = Events::CamerasOff,
        .time = K_SECONDS(2),
        .source = Sources::ExtraCameraTimer,
    },
};

/**
 * Functions for merging the different sources of events
 * 
 * silly immediatly invoked lambda because c++ doesnt support designtaed array initializers :(
 * still constexpr tho which is nice
 */
constexpr std::array<Controller::DecisionFunc, Events::NumEvents> deciders = [] {
    std::array<Controller::DecisionFunc, Events::NumEvents> arr = {nullptr};
    // Ready to go
    arr[Events::PadReady] = [](Controller::SourceStates states) -> bool {
        return states[Sources::IMU1] && states[Sources::IMU2] && states[Sources::Barom1];
    };
    // Boosting
    arr[Events::Boost] = [](Controller::SourceStates states) -> bool {
        return (states[Sources::IMU1] && states[Sources::IMU2]) || states[Sources::Barom1];
    };
    // Coasting
    arr[Events::Coast] = [](Controller::SourceStates states) -> bool {
        return states[Sources::Boost2CoastTimer] || states[Sources::IMU1] || states[Sources::IMU2];
    };
    // Noseover
    arr[Events::Noseover] = [](Controller::SourceStates states) -> bool { return states[Sources::Barom1]; };

    // Main
    arr[Events::MainChute] = [](Controller::SourceStates states) -> bool {
        return states[Sources::Barom1] || states[Sources::Noseover2MainTimer];
    };

    // On the ground
    arr[Events::GroundHit] = [](Controller::SourceStates states) -> bool {
        return states[Sources::Barom1] || states[Sources::FullFlightTimer];
    };
    // After finishing stuff up (what grim did to make sure camera SD cards fully saved)
    arr[Events::CamerasOff] = [](Controller::SourceStates states) -> bool { return states[Sources::ExtraCameraTimer]; };

    return arr;
}();

Controller controller{sourceNames, eventNames, timer_events, deciders};
