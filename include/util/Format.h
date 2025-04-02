#pragma once

#include <format>
#include <sstream>
#include <vector>

template <typename T>
struct std::formatter<std::vector<T>> {
	std::string left;
	std::string right;
	std::string delimiter;

	constexpr auto parse(auto &ctx) {
		auto iter = ctx.begin();
		const std::ptrdiff_t size = ctx.end() - iter;
		if (size < 2) {
			delimiter = ", ";
			return iter;
		}

		left = {*iter++};
		right = {*iter++};

		while (iter != ctx.end() && *iter != '}') {
			delimiter.push_back(*iter++);
		}

		return iter;
	}

	auto format(const auto &vector, auto &ctx) const {
		for (bool start = true; const auto &item: vector) {
			if (start) {
				start = false;
				std::format_to(ctx.out(), "{}{}{}", left, item, right);
			} else {
				std::format_to(ctx.out(), "{}{}{}{}", delimiter, left, item, right);
			}
		}

		return ctx.out();
	}
};
