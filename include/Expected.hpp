#pragma once

#include <concepts>
#include <optional>
#include <utility>
#include <variant>

template <typename T>
concept DefaultMoveCopyLike = std::default_initializable<T> && std::copy_constructible<T> && std::move_constructible<T>;

template <typename ValueType, DefaultMoveCopyLike ErrorType>
class Expected {
    std::variant<ValueType, ErrorType> m_value;

public:
    [[nodiscard]] bool HasError() const noexcept
    {
        return std::holds_alternative<ErrorType>(m_value);
    }

    [[nodiscard]] bool HasValue() const noexcept
    {
        return std::holds_alternative<ValueType>(m_value);
    }

    Expected()
        : m_value(ValueType{})
    {
    }
    Expected(const ValueType& value)
        : m_value(value)
    {
    }
    Expected(ValueType&& value)
        : m_value(std::forward<ValueType>(value))
    {
    }
    Expected(const ErrorType& error)
        : m_value(error)
    {
    }
    Expected(ErrorType&& error)
        : m_value(std::forward<ErrorType>(error))
    {
    }
    template <typename Pred>
    auto OnError(Pred pred) noexcept -> Expected&
    {
        if (HasError()) {
            pred(std::get<ErrorType>(m_value));
        }
        return *this;
    }
    template <typename Pred>
    auto OnValue(Pred pred) noexcept -> Expected&
    {
        if (HasValue()) {
            pred(std::get<ValueType>(m_value));
        }
        return *this;
    }

    [[nodiscard]] auto Value() -> ValueType&
    {
        return std::get<ValueType>(m_value);
    }

    [[nodiscard]] auto Error() -> ErrorType&
    {
        return std::get<ErrorType>(m_value);
    }
};

template <DefaultMoveCopyLike ErrorType>
class Expected<void, ErrorType> {
    std::optional<ErrorType> m_value;

public:
    [[nodiscard]] bool HasError() const noexcept
    {
        return m_value.has_value();
    }

    [[nodiscard]] bool HasValue() const noexcept
    {
        return !m_value.has_value();
    }

    Expected()
        : m_value(std::nullopt)
    {
    }
    Expected(const ErrorType& error)
        : m_value(error)
    {
    }
    Expected(ErrorType&& error)
        : m_value(std::forward<ErrorType>(error))
    {
    }
    template <typename Pred>
    auto OnError(Pred pred) noexcept -> Expected&
    {
        if (HasError()) {
            pred(m_value.value());
        }
        return *this;
    }

    [[nodiscard]] auto Error() -> ErrorType&
    {
        return m_value.value();
    }
};