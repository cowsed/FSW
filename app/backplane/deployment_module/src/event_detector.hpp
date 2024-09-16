#pragma once
#include <stdint.h>

enum class Phase { NoChange, Pad, Boost, Coast, PostNoseover, PostMain, Ground, StopFlight };
struct PhaseEvent {
    Phase nextphase = Phase::NoChange;
    uint32_t offset_ms = 0;
};

class EventDetector {
    virtual PhaseEvent PadCallback() { return {}; }
    virtual PhaseEvent BoostCallback() { return {}; }
    virtual PhaseEvent CoastCallback() { return {}; }
    virtual PhaseEvent PostNoseoverCallback() { return {}; }
    virtual PhaseEvent PostMainCallback() { return {}; }
    virtual PhaseEvent GroundCallback() { return {}; }
};