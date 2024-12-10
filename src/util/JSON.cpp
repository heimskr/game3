#include "util/JSON.h"
#include "util/Util.h"

#include <boost/json.hpp>

#include <string>

namespace Game3 {
	namespace {
		void stringifyWithBigInt(const boost::json::value &json, std::string &string) {
			if (const auto *object = json.if_object()) {
				string += '{';
				bool first = true;
				for (const auto &[key, value]: *object) {
					if (first) {
						first = false;
					} else {
						string += ',';
					}
					string += '"';
					string += escape(key);
					string += "\":";
					stringifyWithBigInt(value, string);
				}
				string += '}';
			} else if (const auto *array = json.if_array()) {
				string += '[';
				bool first = true;
				for (const auto &value: *array) {
					if (first) {
						first = false;
					} else {
						string += ',';
					}
					stringifyWithBigInt(value, string);
				}
				string += ']';
			} else if (const auto *value = json.if_string()) {
				string += '"';
				string += escape(*value);
				string += '"';
			} else if (const bool *boolean = json.if_bool()) {
				string += *boolean? "true" : "false";
			} else if (const double *value = json.if_double()) {
				string += std::to_string(*value);
			} else if (const uint64_t *value = json.if_uint64()) {
				string += std::to_string(*value);
				if (INT_MAX < *value) {
					string += 'n';
				}
			} else if (const int64_t *value = json.if_int64()) {
				string += std::to_string(*value);
				if (INT_MAX < *value) {
					string += 'n';
				}
			} else if (json.is_null()) {
				string += "null";
			} else {
				string += boost::json::serialize(json);
			}
		}
	}

	std::string stringifyWithBigInt(const boost::json::value &json) {
		std::string out;
		stringifyWithBigInt(json, out);
		return out;
	}

	boost::json::value * resolveJSON(boost::json::value &value, const boost::json::array &path) {
		if (path.empty()) {
			return nullptr;
		}

		boost::json::value *current = &value;

		std::optional<uint64_t> index;

		for (size_t i = 0, max = path.size(); i < max; ++i) {
			if (current == nullptr) {
				return nullptr;
			}

			const auto &item = path[i];

			if (const auto *string = item.if_string()) {
				if (auto *object = current->if_object()) {
					current = object->if_contains(*string);
					continue;
				}

				return nullptr;
			}

			if (const int64_t *index_ptr = item.if_int64()) {
				if (*index_ptr < 0) {
					return nullptr;
				}

				index.emplace(*index_ptr);
			} else if (const uint64_t *index_ptr = item.if_uint64()) {
				index.emplace(*index_ptr);
			}

			if (index) {
				if (auto *array = current->if_array()) {
					current = array->if_contains(*index);
					continue;
				}
			}

			return nullptr;
		}

		return current;
	}

	bool patchJSON(boost::json::value &json, const boost::json::array &patch) {
		for (const auto &item: patch) {
			const auto &object = item.as_object();
			std::string_view op(object.at("op").as_string());

			if (op == "test") {
				if (auto *value = resolveJSON(json, object.at("path").as_array())) {
					if (*value == object.at("value")) {
						continue;
					}
				}
				return false;
			}

			if (op == "replace") {
				if (auto *value = resolveJSON(json, object.at("path").as_array())) {
					*value = object.at("value");
					continue;
				}
				return false;
			}

			return false;
		}

		return true;
	}
}
