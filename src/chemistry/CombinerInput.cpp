#include "chemistry/CombinerInput.h"
#include "item/Item.h"

#include <nlohmann/json.hpp>

namespace Game3 {
	CombinerInput CombinerInput::fromJSON(const nlohmann::json &json, ItemCount *output_count_out) {
		CombinerInput out;
		if (!json.is_array())
			throw std::invalid_argument("Invalid JSON for CombinerInput");

		auto iter = json.begin();

		if (output_count_out)
			*output_count_out = *iter++;
		else
			++iter;

		if (iter == json.end())
			throw std::runtime_error("No input items listed in CombinerInput JSON");

		do {
			ItemCount count = 1;
			std::string string;

			if (iter->is_string()) {
				string = *iter;
			} else {
				count = iter->at(0);
				string = iter->at(1);
			}

			out.inputs.emplace_back(count, std::move(string));
		} while (++iter != json.end());

		return out;
	}

	std::vector<ItemStack> CombinerInput::getStacks(Game &game) {
		std::vector<ItemStack> out;

		for (const auto &[count, string]: inputs) {
			if (string.find(':') == std::string::npos)
				out.emplace_back(game, "base:item/chemical", count, nlohmann::json{{"formula", string}});
			else
				out.emplace_back(game, Identifier(string), count);
		}

		return out;
	}
}
