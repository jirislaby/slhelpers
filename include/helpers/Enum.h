// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <iterator>
#include <type_traits>

namespace SlHelpers {

/// @brief Helper class to iterate over enum values from Enum::First to Enum::Last
template <typename Enum>
requires std::is_enum_v<Enum>
class EnumRange {
public:
	/// @brief Type of the underlying enum values
	using Underlying = std::underlying_type_t<Enum>;

	/// @brief Constructs an EnumRange from Enum::First to Enum::Last
	EnumRange() requires requires { Enum::First; Enum::Last; }
		: EnumRange(Enum::First, Enum::Last) {}

	/// @brief Constructs an EnumRange from the specified \p first and \p last enum values
	EnumRange(Enum first, Enum last)
		: m_first(static_cast<Underlying>(first)),
		  m_last(static_cast<Underlying>(last)) {}

	/// @brief Iterator class to iterate over enum values
	struct iterator {
		using iterator_category	= std::forward_iterator_tag;
		using difference_type	= std::ptrdiff_t;
		using value_type	= Enum;
		using reference		= Enum;
		using pointer		= Enum *;

		/// @brief Current value of the iterator
		Underlying v;

		/// @brief Dereference operator to get the current enum value
		Enum operator*() const { return static_cast<Enum>(v); }

		/// @brief Pre-increment operator to move to the next enum value
		iterator &operator++() {
			++v;
			return *this;
		}

		/// @brief Post-increment operator to move to the next enum value
		iterator operator++(int) {
			iterator temp = *this;
			++(*this);
			return temp;
		}

		/// @brief Equality operator to compare two iterators
		bool operator==(const iterator &other) const { return v == other.v; }
		/// @brief Inequality operator to compare two iterators
		bool operator!=(const iterator &other) const { return v != other.v; }
	};

	using value_type	= Enum;
	using const_iterator	= iterator;

	/// @brief Returns an iterator to the beginning of the enum range
	iterator begin() const { return { m_first }; }
	/// @brief Returns an iterator to the end of the enum range
	iterator end() const { return { m_last + 1 }; }
private:
	Underlying m_first;
	Underlying m_last;
};

template<typename E>
struct hasBitmaskOperators : std::false_type {};

#define ENABLE_BITMASK_OPERATORS(E) \
	template<> struct SlHelpers::hasBitmaskOperators<E> : std::true_type {}

template<typename E>
concept BitmaskEnum = std::is_enum_v<E> && hasBitmaskOperators<E>::value;

} // namespace

template<SlHelpers::BitmaskEnum E>
constexpr E operator~(E lhs)
{
	using underlying = std::underlying_type_t<E>;
	return static_cast<E>(~static_cast<underlying>(lhs));
}

template<SlHelpers::BitmaskEnum E>
constexpr E operator|(E lhs, E rhs)
{
	using underlying = std::underlying_type_t<E>;
	return static_cast<E>(static_cast<underlying>(lhs) | static_cast<underlying>(rhs));
}

template<SlHelpers::BitmaskEnum E>
constexpr E operator&(E lhs, E rhs)
{
	using underlying = std::underlying_type_t<E>;
	return static_cast<E>(static_cast<underlying>(lhs) & static_cast<underlying>(rhs));
}

template<SlHelpers::BitmaskEnum E>
constexpr E &operator|=(E &lhs, E rhs)
{
	return lhs = lhs | rhs;
}

template<SlHelpers::BitmaskEnum E>
constexpr E &operator&=(E &lhs, E rhs)
{
	return lhs = lhs & rhs;
}

template<SlHelpers::BitmaskEnum E>
constexpr bool hasFlag(E flags, E flag)
{
	return (flags & flag) != E::NONE;
}
