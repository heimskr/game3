#include <charconv>
#include <cstring>
#include <sstream>

#include "util/Util.h"

namespace Game3 {
	std::default_random_engine utilRNG;

	template <>
	std::vector<std::string_view> split<std::string_view>(std::string_view str, std::string_view delimiter, bool condense) {
		if (str.empty())
			return {};

		size_t next = str.find(delimiter);
		if (next == std::string::npos)
			return {str};

		std::vector<std::string_view> out;
		const size_t delimiter_length = delimiter.size();
		size_t last = 0;

		out.push_back(str.substr(0, next));

		while (next != std::string::npos) {
			last = next;
			next = str.find(delimiter, last + delimiter_length);
			std::string_view sub = str.substr(last + delimiter_length, next - last - delimiter_length);
			if (!sub.empty() || !condense)
				out.push_back(sub);
		}

		return out;
	}

	template <>
	std::vector<std::string> split<std::string>(std::string_view str, std::string_view delimiter, bool condense) {
		const auto pieces = split<std::string_view>(str, delimiter, condense);
		std::vector<std::string> out;
		out.reserve(pieces.size());
		for (const auto &piece: pieces)
			out.emplace_back(piece);
		return out;
	}

	std::string_view trimLeft(std::string_view str, std::string_view to_remove) {
		while (!str.empty() && str.find_first_of(to_remove) == 0)
			str.remove_prefix(1);

		return str;
	}

	std::string_view trimRight(std::string_view str, std::string_view to_remove) {
		while (!str.empty() && str.find_last_of(to_remove) == str.size() - 1)
			str.remove_suffix(1);

		return str;
	}

	std::string_view trim(std::string_view str, std::string_view to_remove) {
		return trimLeft(trimRight(str, to_remove), to_remove);
	}

	long parseLong(const std::string &str, int base) {
		const char *c_str = str.c_str();
		char *end = nullptr;
		const long parsed = strtol(c_str, &end, base);
		if (c_str + str.length() != end)
			throw std::invalid_argument("Not an integer: \"" + str + "\"");
		return parsed;
	}

	long parseLong(const char *str, int base) {
		char *end = nullptr;
		const long parsed = strtol(str, &end, base);
		if (str + strlen(str) != end)
			throw std::invalid_argument("Not an integer: \"" + std::string(str) + "\"");
		return parsed;
	}

	long parseLong(std::string_view view, int base) {
		long out = 0;
		auto result = std::from_chars(view.begin(), view.end(), out, base);
		if (result.ec == std::errc::invalid_argument)
			throw std::invalid_argument("Not an integer: \"" + std::string(view) + "\"");
		return out;
	}

	unsigned long parseUlong(const std::string &str, int base) {
		const char *c_str = str.c_str();
		char *end = nullptr;
		const unsigned long parsed = strtoul(c_str, &end, base);
		if (c_str + str.length() != end)
			throw std::invalid_argument("Not an integer: \"" + str + "\"");
		return parsed;
	}

	unsigned long parseUlong(const char *str, int base) {
		char *end = nullptr;
		const unsigned long parsed = strtoul(str, &end, base);
		if (str + strlen(str) != end)
			throw std::invalid_argument("Not an integer: \"" + std::string(str) + "\"");
		return parsed;
	}

	unsigned long parseUlong(std::string_view view, int base) {
		unsigned long out = 0;
		auto result = std::from_chars(view.begin(), view.end(), out, base);
		if (result.ec == std::errc::invalid_argument)
			throw std::invalid_argument("Not an integer: \"" + std::string(view) + "\"");
		return out;
	}
}
