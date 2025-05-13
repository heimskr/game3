#pragma once

#include "lib/JSON.h"

namespace Game3 {
	std::string stringifyWithBigInt(const boost::json::value &);

	boost::json::value * resolveJSON(boost::json::value &, const boost::json::string &path_string);
	bool patchJSON(boost::json::value &json, const boost::json::array &patch);

	template <template <typename...> typename M, typename K, typename V>
	static M<K, V> loadKeyValuePairs(const boost::json::value &json) {
		M<K, V> out;
		for (const boost::json::value &value: json.as_array()) {
			out.emplace(boost::json::value_to<K>(value.at(0)), boost::json::value_to<V>(value.at(1)));
		}
		return out;
	}
}
