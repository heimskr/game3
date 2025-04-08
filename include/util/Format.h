#pragma once

#include <filesystem>
#include <format>
#include <source_location>
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

template <>
struct std::formatter<std::filesystem::path> {
	constexpr auto parse(auto &ctx) {
		return ctx.begin();
	}

	auto format(const auto &path, auto &ctx) const {
#ifdef __MINGW32__
		// Windows users are punished with an extra allocation.
		return std::format_to(ctx.out(), "{}", path.string().c_str());
#else
		return std::format_to(ctx.out(), "{}", path.c_str());
#endif
	}
};

template <>
struct std::formatter<std::source_location> {
	constexpr auto parse(auto &ctx) {
		return ctx.begin();
	}

	auto format(std::source_location location, auto &ctx) const {
		return std::format_to(ctx.out(), "[{}:{} {}]", location.file_name(), location.line(), location.function_name());
	}
};
