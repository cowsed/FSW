#include "f_core/util/circular_buffer.hpp"

#include <array>
#include <cstddef>

template <typename T, std::size_t len> class RollingSum {
  public:
    static_assert(len > 0, "What is the sum of 0 elements?");

    using value_type = T;
    static constexpr std::size_t size_ = len;

    RollingSum(T start) : buf(start) { setup(start); }

    constexpr std::size_t size() const { return size_; }

    void setup(value_type start) {
        buf.reset(start);
        total = buf.oldest_sample();
        for (std::size_t i = 1; i < buf.size(); i++) {
            total = total + buf[1];
        }
    }

    void feed(value_type value) {
        value_type oldest = buf.oldest_sample();
        total = total - oldest;
        total = total + value;
        buf.add_sample(value);
    }
    value_type sum() const { return total; }

  private:
    value_type total;
    CircularBuffer<value_type, size_> buf;
};

template <typename T, std::size_t len> class MovingAvg {
  public:
    using value_type = T;
    static constexpr std::size_t length = len;

    MovingAvg(value_type start) : summer(start) {}

    void feed(value_type value) { summer.feed(value); }
    value_type avg() { return summer.sum() / (value_type) len; }
    void setup(T start) { summer.setup(start); }

  private:
    RollingSum<value_type, len> summer;
};

template <typename T> class LinearFitSample {

  public:
    using SampleT = T;
    LinearFitSample() : LinearFitSample(0, 0) {}
    LinearFitSample(T x, T y) : x(x), y(y), xx(x * x), xy(x * y) {}
    LinearFitSample(T x, T y, T xx, T xy) : x(x), y(y), xx(xx), xy(xy) {}

    LinearFitSample operator+(const LinearFitSample &o) const {
        return LinearFitSample{x + o.x, y + o.y, xx + o.xx, xy + o.xy};
    }
    LinearFitSample operator-(const LinearFitSample &o) const {
        return LinearFitSample{x - o.x, y - o.y, xx - o.xx, xy - o.xy};
    }

    T x, y, xx, xy;
};
