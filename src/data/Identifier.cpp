#include "data/Identifier.h"
#include "net/Buffer.h"

#include <boost/json.hpp>

namespace Game3 {
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

	std::ostream & operator<<(std::ostream &os, const Identifier &identifier) {
		return os << identifier.space << ':' << identifier.name;
	}

	Identifier tag_invoke(boost::json::value_to_tag<Identifier>, const boost::json::value &json) {
		return {static_cast<std::string_view>(json.get_string())};
	}

	void tag_invoke(boost::json::value_from_tag, boost::json::value &json, const Identifier &identifier) {
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
	std::string Buffer::getType<Identifier>(const Identifier &identifier, bool in_container) {
		if (in_container)
			return getType(std::string{}, true);
		return getType(identifier.str(), false);
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
