#include "lib/JSON.h"

namespace Game3 {
	void tag_invoke(boost::json::value_from_tag, boost::json::value &json, const std::filesystem::path &path) {
		json = path.string();
	}

	std::filesystem::path tag_invoke(boost::json::value_to_tag<std::filesystem::path>, const boost::json::value &json) {
		return {std::string_view(json.as_string())};
	}

	boost::json::object & ensureObject(boost::json::value &json) {
		if (auto *object = json.if_object()) {
			return *object;
		}

		return json.emplace_object();
	}

	void serializeJSON(const JSON::value &json, std::ostream &stream) {
		boost::json::serializer serializer;
		serializer.reset(&json);
		char buffer[512];
		while (!serializer.done()) {
			stream << serializer.read(buffer);
		}
	}

	std::string getString(const JSON::value &json) {
		return std::string(json.as_string());
	}
}
