#pragma once

namespace wm {

template <typename T>
struct IsEqualTo {
    const T value;

    IsEqualTo(const T& val)
        : value(val)
    {
    }

    bool operator()(const auto& other) const
    {
        return value == other;
    }
};

template <typename T>
struct IsNotEqualTo {
    const T value;

    IsNotEqualTo(const T& val)
        : value(val)
    {
    }

    bool operator()(const auto& other) const
    {
        return value != other;
    }
};

template <typename T>
struct IsLessThan {
    const T value;

    IsLessThan(const T& val)
        : value(val)
    {
    }

    bool operator()(const auto& other) const
    {
        return other < value;
    }
};

template <typename T>
struct IsGreaterThan {
    const T value;

    IsGreaterThan(const T& val)
        : value(val)
    {
    }

    bool operator()(const auto& other) const
    {
        return other > value;
    }
};
}