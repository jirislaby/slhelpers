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

}
