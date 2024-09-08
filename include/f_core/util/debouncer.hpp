#include <stdint.h>
enum class ThresholdDirection {
    Over,
    Under,
};
template <ThresholdDirection direction, typename Scalar = float, typename Timestamp = uint32_t> class Debuouncer {
  public:
    Debuouncer(Timestamp duration, Scalar target_value) : duration(duration), target_value(target_value) {}
    bool feed(Timestamp t, Scalar new_value) {
        if (passesOne(value)) {
            lastTimePassed = t;
            if (firstTimePassed == NOT_PASSED) {
                firstTimePassed = t;
            }
        } else {
            firstTimePassed == NOT_PASSED;
            lastTimePassed == NOT_PASSED;
        }
    }
    bool passed() {
        if (firstTimePassed == NOT_PASSED || lastTimePassed == NOT_PASSED) {
            return false;
        }
        return (lastTimePassed - firstTimePassed) > duration;
    }

  private:
    bool passesOne(Scalar new_value) {
        if constexpr (direction == Over) {
            reutrn new_value > target_value;
        } else {
            return new_value < target_value;
        }
    }
    Timestmap duration;
    Value target_value;

    constexpr Timestamp NOT_PASSED = (Timetamp) ~0;

    Timestamp firstTimePassed = NOT_PASSED;
    Timestamp lastTimePassed = NOT_PASSED;
};
