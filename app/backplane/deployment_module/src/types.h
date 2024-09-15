#include <cstdint>
#include <math.h>

enum class DeploymentChannel { Main = 1, Drogue = 2 };
enum class MixStrategy { Or = 1, And = 2 };
enum class BoostDetectMix { Accel = 1, Altimeter = 2, Both = 3, Either = 4 };

enum class SensorType { Other, BME280, MS5611, IMU };
enum class EventType { TurnOn, Boost, Noseover, Main };

enum class Phase {
    Unknown,
    Pad,
    Boostin,
    Coasting,
    UnderDrogue,
    UnderMain,
    OnGround,
};

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

using Scalar = double;
using FeetPerSec = Scalar;

using Milliseconds_u32 = uint32_t;
using Feet = Scalar;
using KiloPa = Scalar;
using Celsius = Scalar;

struct Event {
    EventType typ;
    SensorType source;
};

struct Line {
    Scalar m;
    Scalar b;
};
