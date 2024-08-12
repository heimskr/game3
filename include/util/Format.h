#pragma once

#include <format>
#include <sstream>
#include <vector>

template <typename T>
struct std::formatter<std::vector<T>> {
	constexpr auto parse(auto &ctx) {
		return ctx.begin();
	}

	auto format(const auto &vector, auto &ctx) const {
		std::stringstream ss;
		bool first = true;
		for (const auto &item: vector) {
			if (first)
				first = false;
			else
				ss << ", ";
			ss << std::format("{}", item);
		}
		return std::format_to(ctx.out(), "[{}]", ss.str());
	}
};
