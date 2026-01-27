// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <ranges>

namespace SlHelpers::Views {

/**
 * @brief Mimic c++23's std::views::adjacent<2>
 */
template<std::ranges::view V>
class PairwiseView : public std::ranges::view_interface<PairwiseView<V>> {
public:
	PairwiseView() : m_base(V()) {}
	/// @brief Construct a new PairwiseView containing \p base
	explicit constexpr PairwiseView(V base) : m_base(std::move(base)) {}

	/// @brief Get the beginning of the range
	auto begin() { return Iterator{std::ranges::begin(m_base), std::ranges::end(m_base)}; }
	/// @brief Get the end of the range
	auto end() { return Sentinel{std::ranges::end(m_base)}; }

	/**
	 * @brief The Sentinel for PairwiseView's Iterator
	 */
	struct Sentinel {
		/// @brief The actual end of the range
		std::ranges::iterator_t<V> m_end;
	};

	/**
	 * @brief The Iterator for PairwiseView
	 */
	struct Iterator {
	private:
		std::ranges::iterator_t<V> m_cur;
		std::ranges::iterator_t<V> m_next;
		std::ranges::iterator_t<V> m_end;
	public:
		/// @brief Type of the stored pair in this Iterator
		using ValueType = std::pair<decltype(*m_cur), decltype(*m_cur)>;

		/// @brief Construct a new Iterator
		Iterator(std::ranges::iterator_t<V> begin, std::ranges::iterator_t<V> end)
			: m_cur(begin), m_next(begin), m_end(end) {
			if (m_next != m_end)
				++m_next;
		}

		/// @brief Dereference this Iterator -- obtain the pair
		ValueType operator*() const { return { *m_cur, *m_next }; }

		/// @brief Pre-increment this Iterator
		Iterator &operator++() {
			++m_cur;
			++m_next;
			return *this;
		}

		/// @brief Compare to Sentinel
		bool operator!=(Sentinel s) const { return m_cur != s.m_end && m_next != s.m_end; }

	};
private:
	V m_base;
};

template<class R>
PairwiseView(R&&) -> PairwiseView<std::views::all_t<R>>;

/**
 * @brief Support struct for PairwiseView
 */
struct PairwiseFn {
	/// @brief Support for pairwise(view)
	template<std::ranges::viewable_range R>
	auto operator()(R &&r) const {
		return PairwiseView(std::views::all(std::forward<R>(r)));
	}

	/// @brief Support for view | pairwise
	template<std::ranges::viewable_range R>
	friend auto operator|(R &&r, const PairwiseFn &fn) {
		return fn(std::forward<R>(r));
	}
};

inline constexpr PairwiseFn pairwise{};

} // namespace
