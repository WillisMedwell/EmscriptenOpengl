#pragma once

#include <concepts>
#include <ranges>

namespace Concept {

template <typename Range, typename Underlying>
concept IsContiguousRangeWithUnderlyingType = std::ranges::contiguous_range<Range> && requires(Range range, Underlying underlying) {
    {
        std::remove_reference_t<decltype(*range.begin())> {}
    } -> std::same_as<Underlying>;
};

template <typename Range, typename Underlying>
concept IsRangeWithUnderlyingType = std::ranges::range<Range> && requires(Range range, Underlying underlying) {
    {
        std::remove_reference_t<decltype(*range.begin())> {}
    } -> std::same_as<Underlying>;
};

}
