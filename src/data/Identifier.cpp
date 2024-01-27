#include "data/Identifier.h"
#include "net/Buffer.h"

#include <nlohmann/json.hpp>

namespace Game3 {
	Identifier::Identifier(std::string_view combined) {
		const size_t colon = combined.find(':');
		if (colon == std::string_view::npos)
			throw std::invalid_argument("Not a valid identifier: " + std::string(combined));
		space = std::string(combined.substr(0, colon));
		name  = std::string(combined.substr(colon + 1));
	}

	Identifier::Identifier(const char *combined):
		Identifier(std::string_view(combined)) {}

	std::string Identifier::getPath() const {
		const size_t slash = name.find_last_of('/');
		if (slash == std::string::npos)
			return "";
		return name.substr(0, slash);
	}

	std::string Identifier::getPathStart() const {
		const size_t slash = name.find('/');
		if (slash == std::string::npos)
			return "";
		return name.substr(0, slash);
	}

	std::string Identifier::getPostPath() const {
		const size_t slash = name.find_last_of('/');
		if (slash == std::string::npos)
			return name;
		return name.substr(slash + 1);
	}

	bool Identifier::operator==(const char *combined) const {
		return *this == std::string_view(combined);
	}

	bool Identifier::operator==(std::string_view combined) const {
		const size_t colon = combined.find(':');
		if (colon == std::string_view::npos)
			return false;
		return space == combined.substr(0, colon) && name == combined.substr(colon + 1);
	}

	bool Identifier::operator==(const Identifier &other) const {
		return this == &other || (space == other.space && name == other.name);
	}

	bool Identifier::operator<(const Identifier &other) const {
		if (this == &other)
			return false;

		if (space < other.space)
			return true;

		if (space > other.space)
			return false;

		return name < other.name;
	}

	void from_json(const nlohmann::json &json, Identifier &identifier) {
		identifier = std::string_view(json.get<std::string>());
	}

	void to_json(nlohmann::json &json, const Identifier &identifier) {
		json = identifier.str();
	}

	Identifier operator""_id(const char *string, size_t) {
		return {string};
	}

	template <>
	Identifier popBuffer<Identifier>(Buffer &buffer) {
		return {popBuffer<std::string>(buffer)};
	}

	template <>
	std::string Buffer::getType<Identifier>(const Identifier &) {
		return getType(std::string{});
	}

	Buffer & operator+=(Buffer &buffer, const Identifier &identifier) {
		return buffer += identifier.str();
	}

	Buffer & operator<<(Buffer &buffer, const Identifier &identifier) {
		return buffer << identifier.str();
	}

	Buffer & operator>>(Buffer &buffer, Identifier &identifier) {
		std::string str;
		buffer >> str;
		identifier = std::string_view(str);
		return buffer;
	}
}
