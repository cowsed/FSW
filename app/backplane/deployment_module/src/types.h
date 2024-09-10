#include <cstdint>

enum class DeploymentChannel { Main = 1, Drogue = 2 };
enum class MixStrategy { Or = 1, And = 2 };
enum class BoostDetectMix { Accel = 1, Altimeter = 2, Both = 3, Either = 4 };

struct altim_telem {
    float temp;
    float press;
};

template <typename Value, typename TimestampT = uint32_t> struct Timestamped {
    using value_type = Value;
    using timestamp_type = TimestampT;

    timestamp_type time;
    value_type value;
};

struct Line {
    float m;
    float b;
};
