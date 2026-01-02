#ifndef LOOKUP_TABLE_H
#define LOOKUP_TABLE_H
#include <cinttypes>
#include <map>

template <typename T>
class LookupTable
{
private:
    std::map<T, T> table;

public:
    explicit LookupTable(std::map<T, T> input)
        : table(std::move(input))
    {
    }

    T get(T key) const
    {
        if (table.empty())
            return 0; // Or throw, depending on your error handling strategy

        auto first = table.begin();
        auto last = std::prev(table.end());

        // Clamp low
        if (key <= first->first)
            return first->second;

        if (key == last->first)
            return last->second;

        // Extrapolation
        if (key > last->first)
        {
            auto upper = last;
            auto lower = std::prev(upper);
            // printf("lower = %ld upper = %ld\n", lower->first, upper->first);
            return extrapolate(key, lower->first, lower->second, upper->first, upper->second);
        }

        // Interpolation: find first element greater than key
        auto upper = table.upper_bound(key);
        auto lower = std::prev(upper);

        return interpolate(key, lower->first, lower->second, upper->first, upper->second);
    }

private:
    static T interpolate(T x, T x0, T y0, T x1, T y1)
    {
        float ratio = static_cast<float>(x - x0) / (x1 - x0);
        return static_cast<T>(y0 + ratio * (y1 - y0));
    }

    static T extrapolate(T x, T x0, T y0, T x1, T y1)
    {
        float slope = static_cast<float>(y1 - y0) / (x1 - x0);
        return static_cast<T>(y1 + slope * (x - x1));
    }
};
#endif