#include "data/Identifier.h"

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
		if (slash == name.npos)
			return "";
		return name.substr(0, slash);
	}

	std::string Identifier::getPathStart() const {
		const auto slash = name.find('/');
		if (slash == name.npos)
			return "";
		return name.substr(0, slash);
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
		return Identifier(string);
	}
}

std::ostream & operator<<(std::ostream &os, const Game3::Identifier &id) {
	return os << id.str();
}
