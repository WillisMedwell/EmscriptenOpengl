#pragma once

#include <algorithm>
#include <compare>
#include <ranges>
#include <span>
#include <tuple>
#include <vector>

#include "wm/Functors.hpp"

namespace wm {

template <std::ranges::contiguous_range Container, typename Element>
class SplitByElement {
    Container& _container;
    Element _delim;

    using ContainerElement = std::remove_reference_t<decltype(*_container.begin())>;
    using ContainerIterator = std::ranges::iterator_t<Container>;

    std::vector<std::span<ContainerElement>> index_lookup;

public:
    SplitByElement(Container& container, Element delim)
        : _container(container)
        , _delim(delim)
    {
        static_assert(std::is_convertible_v<std::remove_cvref_t<Element>, ContainerElement>);
    }

    struct Iterator {
        ContainerIterator sub_begin;
        ContainerIterator sub_end;
        ContainerIterator container_end;
        Element delim;

        Iterator(ContainerIterator sb, ContainerIterator se, ContainerIterator ce, Element d)
            : sub_begin(sb)
            , sub_end(se)
            , container_end(ce)
            , delim(d)
        {
            sub_begin = std::find_if(sub_begin, container_end, IsNotEqualTo(d));
            sub_end = std::find_if(sub_begin, container_end, IsEqualTo(d));
        }

        auto operator*()
        {
            return std::span<ContainerElement> { sub_begin, sub_end };
        }

        auto operator++() -> Iterator&
        {
            if (sub_end == container_end) {
                sub_begin = container_end;
                return *this;
            }

            ++sub_end;
            sub_end = std::find_if(sub_end, container_end, IsNotEqualTo(delim));
            sub_begin = sub_end;
            sub_end = std::find_if(sub_end, container_end, IsEqualTo(delim));
            return *this;
        }

        auto operator==(const Iterator& other) const -> bool
        {
            return sub_begin == other.sub_begin
                && sub_end == other.sub_end
                && container_end == other.container_end
                && delim == other.delim;
        }
        auto operator!=(const Iterator other) const -> bool
        {
            return !(*this == other);
        }
    };

    auto begin() const
    {
        return Iterator(_container.begin(), _container.begin(), _container.end(), _delim);
    }
    auto end() const
    {
        return Iterator(_container.end(), _container.end(), _container.end(), _delim);
    }

    auto operator[](size_t i) -> std::span<ContainerElement>
    {
        if (index_lookup.size() == 0) {
            for (auto sub : *this) {
                index_lookup.emplace_back(sub);
            }
        }
        return index_lookup[i];
    }

    auto evaluate() -> std::vector<std::span<ContainerElement>>
    {
        if (index_lookup.size() == 0) {
            for (auto sub : *this) {
                index_lookup.emplace_back(sub);
            }
        }
		return index_lookup;
    }
};

}