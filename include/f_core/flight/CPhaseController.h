#include <array>
#include <cstdint>
#include <cstdio>

void flight_log_init();
void flight_log_source_event(const char *source, const char *event) { printf("%-10s from %s\n", event, source); }
void flight_log_event_confirmed(const char *event, bool current_state) {
    printf("%-10s confirmed%s\n", event, current_state ? " but already happened. Not dispatching" : "");
}

template <typename EventID, std::size_t num_events, typename SourceID, std::size_t num_sources> class PhaseController {
  public:
    // States of all the sources for a certain event
    // Example:
    // Source: IMU1  Barom  GNSS
    // State:  true  true   false
    // Decision functions take this and come to a conclusion about whether or
    // not the event is actually achieved
    using SourceStates = std::array<bool, num_sources>;

    // Decision functions should be intterupt-safe
    // quick operations that don't do IO or sleep or anything spooky
    using DecisionFunc = bool (*)(SourceStates source_states);

    PhaseController(const std::array<const char *, num_sources> &sourceNames,
                    const std::array<const char *, num_events> &eventNames,
                    const std::array<DecisionFunc, num_events> &deciders)
        : sourceNames(sourceNames), eventNames(eventNames), deciders(deciders) {}

    void SubmitEvent(SourceID source, EventID event) {
        source_states[event][source] = true;
        flight_log_source_event(sourceNames[source], eventNames[event]);

        if (deciders[event](source_states[event])) {
            bool state = event_states[event];
            flight_log_event_confirmed(eventNames[event], state);
            if (!state) {
                // dispatch event
                event_states[event] = true;
            }
        }
    }

    // Dealing with state
    bool HasEventOccured(EventID event);
    void WaitUntilEvent(EventID);

    // Current State of the system
    std::array<SourceStates, num_events> source_states = {false};
    std::array<bool, num_events> event_states = {false};

    // consts for logging and deciding. These will not change after construction
    const std::array<const char *, num_sources> &sourceNames;
    const std::array<const char *, num_events> &eventNames;
    const std::array<DecisionFunc, num_events> &deciders;
};