// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <type_traits>

namespace SlHelpers {

/// @brief Helper class to iterate over enum values from Enum::First to Enum::Last
template <typename Enum>
class EnumRange {
public:
	/// @brief Type of the underlying enum values
	using Underlying = std::underlying_type_t<Enum>;

	/// @brief Iterator class to iterate over enum values
	struct iterator {
		/// @brief Current value of the iterator
		Underlying v;

		/// @brief Dereference operator to get the current enum value
		Enum operator*() const { return static_cast<Enum>(v); }

		/// @brief Pre-increment operator to move to the next enum value
		iterator &operator++() {
			++v;
			return *this;
		}

		/// @brief Equality operator to compare two iterators
		bool operator==(const iterator &other) const { return v == other.v; }
		/// @brief Inequality operator to compare two iterators
		bool operator!=(const iterator &other) const { return v != other.v; }
	};

	/// @brief Returns an iterator to the beginning of the enum range
	iterator begin() const { return { static_cast<Underlying>(Enum::First) }; }
	/// @brief Returns an iterator to the end of the enum range
	iterator end() const { return { static_cast<Underlying>(Enum::Last) + 1 }; }
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
