#include "chemistry/CombinerInput.h"
#include "item/Item.h"
#include "lib/JSON.h"

namespace Game3 {
	CombinerInput tag_invoke(boost::json::value_to_tag<CombinerInput>, const boost::json::value &json, ItemCount *output_count_out) {
		CombinerInput out;

		if (!json.is_array()) {
			throw std::invalid_argument("Invalid JSON for CombinerInput");
		}

		const auto &array = json.as_array();

		auto iter = array.begin();

		if (output_count_out) {
			*output_count_out = boost::json::value_to<ItemCount>(*iter++);
		} else {
			++iter;
		}

		if (iter == array.end()) {
			throw std::runtime_error("No input items listed in CombinerInput JSON");
		}

		do {
			ItemCount count = 1;
			std::string string;

			if (iter->is_string()) {
				string = iter->get_string();
			} else {
				count = boost::json::value_to<ItemCount>(iter->at(0));
				string = iter->at(1).get_string();
			}

			out.inputs.emplace_back(count, std::move(string));
		} while (++iter != array.end());

		return out;
	}

	std::vector<ItemStackPtr> CombinerInput::getStacks(const std::shared_ptr<Game> &game) {
		std::vector<ItemStackPtr> out;

		for (const auto &[count, string]: inputs) {
			if (string.find(':') == std::string::npos)
				out.push_back(ItemStack::create(game, "base:item/chemical", count, boost::json::value{{"formula", string}}));
			else
				out.push_back(ItemStack::create(game, Identifier(string), count));
		}

		return out;
	}
}
