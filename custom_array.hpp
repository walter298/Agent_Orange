#pragma once

#include <concepts>
#include <stdexcept>
#include <utility>

#include <nlohmann/json.hpp>

namespace merck {
	template<typename T, std::integral idx_type, idx_type elem_count>
		requires(elem_count > 0) //can't have negative/0 # of elements
	class custom_array {
	private:
		T m_data[elem_count];
	public:
		constexpr int size() const {
			return elem_count;
		}

		T& operator[](idx_type idx) {
			return m_data[idx];
		}
		const T& operator[](idx_type idx) const {
			return m_data[idx];
		}

		auto begin() {
			return std::begin(m_data);
		}
		auto end() {
			return std::end(m_data);
		}
		auto cbegin() {
			return std::cbegin(m_data);
		}
		auto cend() {
			return std::cend(m_data);
		}
	};

	template<typename T, std::integral idx_type, idx_type size>
	void to_json(nlohmann::json& j, 
		const custom_array<T, idx_type, size>& arr) 
	{
		for (int i = 0; i < arr.size(); i++) {
			j[i] = arr[i];
		}
	}

	template<typename T, std::integral idx_type, idx_type size>
	void from_json(const nlohmann::json& j, 
		custom_array<T, idx_type, size>& arr)
	{
		for (int i = 0; i < arr.size(); i++) {
			arr[i] = j[i];
		}
	}
}