#include "util/JSON.h"
#include "util/Util.h"

#include <nlohmann/json.hpp>

namespace Game3 {
	namespace {
		void stringifyWithBigInt(const nlohmann::json &json, std::string &string) {
			if (json.is_object()) {
				string += '{';
				bool first = true;
				for (const auto &[key, value]: json.items()) {
					if (first)
						first = false;
					else
						string += ',';
					string += '"';
					string += escape(key);
					string += "\":";
					stringifyWithBigInt(value, string);
				}
				string += '}';
			} else if (json.is_array()) {
				string += '[';
				bool first = true;
				for (const auto &value: json) {
					if (first)
						first = false;
					else
						string += ',';
					stringifyWithBigInt(value, string);
				}
				string += ']';
			} else if (json.is_string()) {
				string += '"';
				string += escape(json.get<std::string_view>());
				string += '"';
			} else if (json.is_boolean()) {
				string += json.get<bool>()? "true" : "false";
			} else if (json.is_number_float()) {
				string += std::to_string(json.get<double>());
			} else if (json.is_number_unsigned()) {
				uint64_t num(json);
				string += std::to_string(num);
				if (INT_MAX < num)
					string += 'n';
			} else if (json.is_number_integer()) {
				int64_t num(json);
				string += std::to_string(num);
				if (INT_MAX < num)
					string += 'n';
			} else if (json.is_null()) {
				string += "null";
			} else {
				string += json.dump();
			}
		}
	}

	std::string stringifyWithBigInt(const nlohmann::json &json) {
		std::string out;
		stringifyWithBigInt(json, out);
		return out;
	}
}
