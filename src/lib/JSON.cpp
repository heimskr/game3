#include "lib/JSON.h"

namespace Game3 {
	void tag_invoke(boost::json::value_from_tag, boost::json::value &json, const std::filesystem::path &path) {
		json = path.string();
	}

	std::filesystem::path tag_invoke(boost::json::value_to_tag<std::filesystem::path>, const boost::json::value &json) {
		return {std::string_view(json.as_string())};
	}
}
