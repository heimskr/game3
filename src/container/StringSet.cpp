#include "container/StringSet.h"
#include "lib/JSON.h"

namespace Game3 {
	void tag_invoke(boost::json::value_from_tag, boost::json::value &json, const StringSet &set) {
		auto &array = json.emplace_array();
		array.reserve(set.size());

		for (const std::string &string: set) {
			array.emplace_back(string);
		}
	}

	StringSet tag_invoke(boost::json::value_to_tag<StringSet>, const boost::json::value &json) {
		const auto &array = json.as_array();
		StringSet out;
		out.reserve(array.size());

		for (const auto &item: array) {
			out.emplace(item.as_string());
		}

		return out;
	}
}
