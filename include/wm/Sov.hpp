#pragma once
// Structure of vectors implementation
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <iterator>
#include <memory>
#include <numeric>
#include <ranges>
#include <span>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <typeinfo>
#include <vector>

namespace wm {

template <typename... Types>
class Sov {
private:
    using FieldsValue = std::tuple<Types...>;
    using FieldsPtr = std::tuple<Types*...>;
    using FieldsRef = std::tuple<Types&...>;
    using FieldsConstRef = std::tuple<const Types&...>;
    using FieldsMove = std::tuple<Types&&...>;
    constexpr static size_t num_types = std::tuple_size<FieldsValue>();

    constexpr static size_t bytes_per_entry = sizeof(FieldsValue);

    // std::unique_ptr<uint8_t[], std::default_delete<uint8_t[]>> data;
    size_t entry_capacity = 0;
    size_t entry_count = 0;
    FieldsPtr beginnings;
    uint8_t* data;

private: // tuple iteration helpers
    template <size_t index = 0, typename Tuple, typename T, class Pred>
    constexpr static auto accumulateTuple(Tuple tuple, T init,
        Pred predicate)
    {
        if constexpr (index == std::tuple_size<Tuple>()) {
            return init;
        } else {
            auto& element = std::get<index>(tuple);
            init = predicate(init, element);
            return accumulateTuple<index + 1, Tuple, T, Pred>(tuple, init, predicate);
        }
    }

    template <size_t index = 0, typename Tuple, class Pred>
    constexpr static void forEachTuple(Tuple& tuple, Pred predicate)
    {
        if constexpr (index == std::tuple_size<Tuple>()) {
            return;
        } else {
            auto& element = std::get<index>(tuple);
            predicate(element);
            return forEachTuple<index + 1>(tuple, predicate);
        }
    }

    template <size_t index = 0>
    constexpr static void moveTuple(FieldsPtr& destination, FieldsMove& source,
        const size_t i)
    {
        if constexpr (index == num_types) {
            return;
        } else {
            using FieldType = std::tuple_element_t<index, FieldsValue>;
            using FieldPtr = std::tuple_element_t<index, FieldsPtr>;
            auto* ptr = reinterpret_cast<FieldPtr>(std::get<index>(destination) + i);
            std::construct_at(ptr, std::forward<FieldType>(std::get<index>(source)));
            return moveTuple<index + 1>(destination, source, i);
        }
    }

    template <size_t index = 0>
    constexpr static void emplaceTuple(FieldsPtr& destination, const FieldsConstRef& source, const size_t i)
    {
        if constexpr (index == num_types) {
            return;
        } else {
            using FieldPtr = std::tuple_element_t<index, FieldsPtr>;
            auto* ptr = reinterpret_cast<FieldPtr>(std::get<index>(destination) + i);
            std::construct_at(ptr, std::get<index>(source));
            return emplaceTuple<index + 1>(destination, source, i);
        }
    }

    template <size_t index = 0, typename T>
    constexpr static auto getElement(FieldsPtr& source, const size_t i,
        T zipped_element)
    {
        if constexpr (index == num_types) {
            return zipped_element;
        } else {
            auto& element = std::get<index>(source)[i];
            auto updated_zipped_element = std::tuple_cat(
                zipped_element, std::tuple<decltype(element)> { element });
            return getElement<index + 1>(source, i, updated_zipped_element);
        }
    }

    template <size_t index = 0>
    constexpr static auto destroyElement(FieldsPtr& source, const size_t i) -> void
    {
        if constexpr (index == num_types) {
            return;
        } else {
            using FieldPtr = std::tuple_element_t<index, FieldsPtr>;
            auto ptr = reinterpret_cast<FieldPtr>(std::get<index>(source) + i);
            std::destroy_at(ptr);
            return destroyElement<index + 1>(source, i);
        }
    }

    template <size_t index = 0>
    constexpr static void copyFields(FieldsPtr& destination,
        const FieldsPtr& source, size_t count)
    {
        if constexpr (index == num_types) {
            return;
        } else {
            auto src_set = std::span { std::get<index>(source), count };
            auto dest_set = std::span { std::get<index>(destination), count };
            std::ranges::uninitialized_copy(src_set, dest_set);
            return copyFields<index + 1>(destination, source, count);
        }
    }

    template <size_t index = 0>
    constexpr static void moveFields(FieldsPtr& destination,
        const FieldsPtr& source, size_t count)
    {
        if constexpr (index == num_types) {
            return;
        } else {
            auto old_set = std::span { std::get<index>(source), count };
            auto new_set = std::span { std::get<index>(destination), count };
            std::ranges::uninitialized_move(old_set, new_set);
            std::ranges::destroy(old_set);
            return moveFields<index + 1>(destination, source, count);
        }
    }

    template <typename... Ts>
    constexpr static bool isSameAlignment()
    {
        return (
            ... && (std::alignment_of_v<Ts> == std::alignment_of_v<std::tuple_element_t<0, std::tuple<Ts...>>>));
    }

    static constexpr bool is_same_alignment = isSameAlignment<Types...>();

public:
    constexpr Sov(size_t init_capacity = 20)
        : entry_capacity(init_capacity)
        , entry_count(0)
        , beginnings({})
        , data(new uint8_t[bytes_per_entry * init_capacity + 64])
    {
        static_assert(std::conjunction_v<std::is_default_constructible<Types>...>,
            "All types must be default-constructible");
        uint8_t* ptr = data;

        auto assign_beginning = [&](auto& begin) {
            using Ptr = std::remove_reference_t<decltype(begin)>;
            using Value = std::remove_pointer_t<Ptr>;
            // ensure proper memory alignment
            if constexpr (!is_same_alignment) {
                while (reinterpret_cast<std::uintptr_t>(ptr) % alignof(Value)) {
                    ++ptr;
                }
            }
            begin = reinterpret_cast<Ptr>(ptr);
            ptr += sizeof(Value) * entry_capacity;
        };
        forEachTuple(beginnings, assign_beginning);
    }

    Sov(const Sov& other)
        : Sov(other.entry_capacity)
    {
        copyFields(beginnings, other.beginnings, other.entry_count);
        entry_count = other.entry_count;
    }

    Sov(Sov&& other)
        : entry_capacity(other.entry_capacity)
        , entry_count(other.entry_count)
        , data(other.data)
        , beginnings(other.beginnings)
    {
        other.entry_capacity = 0;
        other.entry_count = 0;
        other.beginnings = {};
    }

    ~Sov()
    {
        auto deleteField = [&](auto& ptr) {
            auto field = std::span { ptr, entry_count };
            std::ranges::destroy(field);
            ptr = nullptr;
        };
        forEachTuple(beginnings, deleteField);
        delete[] data;
    }

    auto pushBack(const Types&... value) -> void
    {
        if (entry_count == entry_capacity) {
            grow(entry_capacity * 2);
        }
        emplaceTuple(beginnings, FieldsConstRef(value...), entry_count);
        ++entry_count;
    }
    
    auto pushBack(const FieldsConstRef value) -> void
    {
        if (entry_count == entry_capacity) {
            grow(entry_capacity * 2);
        }
        emplaceTuple(beginnings, value, entry_count);
        ++entry_count;
    }

    auto pushBack(Types&&... value) -> void
    {
        if (entry_count == entry_capacity) {
            grow(entry_capacity * 2);
        }
        auto fields = FieldsMove(std::forward<Types>(value)...);
        moveTuple(beginnings, fields, entry_count);
        ++entry_count;
    }

    inline auto popBack() -> void
    {
        if (entry_count != 0) {
            --entry_count;
            destroyElement(beginnings, entry_count);
        }
    }

    auto grow(size_t new_entry_capacity)
    {
        assert(new_entry_capacity > entry_capacity);
        assert(new_entry_capacity > 0);

        auto new_data = new uint8_t[bytes_per_entry * new_entry_capacity + 64];
        uint8_t* ptr = new_data;
        FieldsPtr new_beginnings;

        auto assign_beginning = [&](auto& begin) {
            using Ptr = std::remove_reference_t<decltype(begin)>;
            using Value = std::remove_pointer_t<Ptr>;
            if constexpr (!is_same_alignment) {
                // ensure proper memory alignment
                while (reinterpret_cast<std::uintptr_t>(ptr) % alignof(Value)) {
                    ++ptr;
                }
            }
            begin = reinterpret_cast<Ptr>(ptr);
            ptr += sizeof(Value) * new_entry_capacity;
        };
        forEachTuple(new_beginnings, assign_beginning);
        assert(ptr < new_data + (bytes_per_entry * new_entry_capacity + 64));

        moveFields(new_beginnings, beginnings, entry_count);

        std::swap(data, new_data);
        entry_capacity = new_entry_capacity;
        beginnings = new_beginnings;
        delete[] new_data;
    }

    template <size_t i>
    [[nodiscard]] inline auto field()
    {
        static_assert(i < num_types, "Trying to access inaccessible field");
        return std::span { std::get<i>(beginnings), entry_count };
    }

    template <typename T>
    [[nodiscard]] inline auto fieldData() -> T*
    {
        return std::get<T*>(beginnings);
    }

    template <typename T>
    [[nodiscard]] inline auto field() -> std::span<T>
    {
        return std::span { std::get<T*>(beginnings), entry_count };
    }

    template <typename Field>
    [[nodiscard]] inline auto field() const -> const std::span<const Field>
    {
        return { std::get<Field*>(beginnings), entry_count };
    }

    template <typename Field>
    consteval static bool hasField(Field field = {})
    {
        auto containsField = [&](bool has_field, auto t) -> bool {
            return (has_field || std::is_same_v<decltype(t), Field>);
        };
        bool result = accumulateTuple(FieldsValue {}, false, containsField);
        return result;
    }

    [[nodiscard]] inline auto operator[](size_t index) -> FieldsRef
    {
        return getElement(beginnings, index, std::tuple<>());
    }

    [[nodiscard]] inline auto at(int index) -> FieldsRef
    {
        if (index >= entry_count || index < 0) {
            throw std::out_of_range { "Sov::at() is out of range" };
        }
        return getElement(beginnings, index, std::tuple<>());
    }

    class Iterator {
    private:
        Sov& sov;
        size_t index = 0;

    public:
        Iterator(Sov& s, size_t i) noexcept
            : sov(s)
            , index(i)
        {
        }
        inline auto operator++() noexcept -> Iterator&
        {
            ++index;
            return *this;
        }
        inline auto operator++(int) noexcept -> Iterator
        {
            auto cpy = *this;
            ++cpy;
            return cpy;
        }
        [[nodiscard]] inline auto operator==(const Iterator& other) const noexcept -> bool
        {
            return this->index == other.index;
        }
        [[nodiscard]] inline auto operator!=(const Iterator& other) const noexcept -> bool
        {
            return this->index != other.index;
        }
        [[nodiscard]] inline auto operator*() noexcept { return sov[index]; }
    };
    [[nodiscard]] inline auto begin() noexcept -> Iterator { return Iterator(*this, 0); }
    [[nodiscard]] inline auto end() noexcept -> Iterator { return Iterator(*this, entry_count); }
    [[nodiscard]] inline auto size() const noexcept -> size_t { return entry_count; }
};

}