#include "data/Identifier.h"
#include "net/Buffer.h"

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
		const auto slash = name.find_last_of('/');
		if (slash == std::string::npos)
			return "";
		return name.substr(0, slash);
	}

	std::string Identifier::getPathStart() const {
		const auto slash = name.find('/');
		if (slash == std::string::npos)
			return "";
		return name.substr(0, slash);
	}

	std::string Identifier::getPostPath() const {
		const auto slash = name.find_last_of('/');
		if (slash == std::string::npos)
			return name;
		return name.substr(slash + 1);
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

	std::ostream & operator<<(std::ostream &os, const Identifier &id) {
		return os << id.str();
	}
}

