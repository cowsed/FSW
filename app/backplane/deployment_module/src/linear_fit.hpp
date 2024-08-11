#include <array>
#include <cstddef>

template <typename T, std::size_t len> class RollingSum {
  public:
    using Type = T;
    static constexpr std::size_t length = len;

    RollingSum() {}
    RollingSum(T start) { setup(start); }
    void setup(T start) {
        underlying.fill(start);
        total = underlying[0];
        for (std::size_t i = 1; i < underlying.size(); i++) {
            total += underlying[1];
        }
        write_head = 0;
    }

    void feed(Type value) {
        Type oldest = underlying[write_head];
        total = total - oldest;
        total = total + value;
        underlying[write_head] = value;
        write_head++;
        if (write_head >= length) {
            write_head = 0;
        }
    }
    Type sum() const { return total; }
    std::size_t size() const { return length; }

  private:
    std::size_t write_head = 0;
    Type total = {};
    std::array<Type, length> underlying;
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
