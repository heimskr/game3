#include "util/Log.h"
#include "chemistry/DissolverResults.h"
#include "item/Item.h"
#include "lib/JSON.h"
#include "threading/ThreadContext.h"
#include "util/Util.h"

namespace Game3 {
	std::vector<ItemStackPtr> DissolverResult::getResult(const std::shared_ptr<Game> &game) {
		std::vector<ItemStackPtr> out;
		add(game, out);
		return out;
	}

	std::vector<ItemStackPtr> DissolverResult::getResult(const std::shared_ptr<Game> &game, const boost::json::value &json) {
		auto result = fromJSON(json);
		assert(result);
		std::vector<ItemStackPtr> out;
		result->add(game, out);
		return out;
	}

	std::unique_ptr<DissolverResult> DissolverResult::fromJSON(const boost::json::value &json) {
		if (json.is_string()) {
			return std::make_unique<ChemicalResult>(json);
		}

		if (json.is_null()) {
			throw std::runtime_error("Null JSON object encountered in DissolverResult::getResult");
		}

		if (!json.is_array()) {
			throw std::runtime_error("Expected an array in DissolverResult::getResult");
		}

		const auto &array = json.as_array();

		if (array.at(0).is_number()) {
			return std::make_unique<MultiChemicalResult>(json);
		}

		std::string type(array.at(0).as_string());

		if (type == "union" || type == "+") {
			return std::make_unique<UnionDissolverResult>(json);
		}

		if (type == "weighted" || type == "*") {
			return std::make_unique<WeightedDissolverResult>(json);
		}

		if (type == "random" || type == "?") {
			return std::make_unique<RandomDissolverResult>(json);
		}

		throw std::invalid_argument("Invalid DissolverResult JSON");
	}

	void tag_invoke(boost::json::value_from_tag, boost::json::value &json, const DissolverResult &result) {
		result.toJSON(json);
	}

	UnionDissolverResult::UnionDissolverResult(const boost::json::value &json) {
		const auto &array = json.as_array();

		for (size_t i = 1; i < array.size(); ++i) {
			members.push_back(DissolverResult::fromJSON(array.at(i)));
		}
	}

	void UnionDissolverResult::add(const std::shared_ptr<Game> &game, std::vector<ItemStackPtr> &stacks) {
		for (const std::unique_ptr<DissolverResult> &member: members)
			member->add(game, stacks);
	}

	void UnionDissolverResult::toJSON(boost::json::value &json) const {
		auto &array = json.emplace_array();
		array.emplace_back("+");
		for (const auto &member: members) {
			boost::json::value subjson;
			member->toJSON(subjson);
			array.push_back(std::move(subjson));
		}
	}

	WeightedDissolverResult::WeightedDissolverResult(const boost::json::value &json) {
		const auto &array = json.as_array();
		for (size_t i = 1; i < array.size(); ++i) {
			const auto &item = array.at(i).as_array();
			members.emplace_back(getDouble(item.at(0)), DissolverResult::fromJSON(item.at(1)));
		}
	}

	void WeightedDissolverResult::add(const std::shared_ptr<Game> &game, std::vector<ItemStackPtr> &stacks) {
		if (members.empty())
			return;

		double sum = 0.;

		for (const auto &[weight, result]: members)
			sum += weight;

		const double choice = std::uniform_real_distribution(0., sum)(threadContext.rng);
		double accumulator = 0.;

		for (const auto &[weight, result]: members) {
			accumulator += weight;

			if (choice < accumulator) {
				result->add(game, stacks);
				return;
			}
		}

		throw std::logic_error("Impossible condition reached");
	}

	void WeightedDissolverResult::toJSON(boost::json::value &json) const {
		auto &array = json.emplace_array();
		array.push_back("*");
		for (const auto &[weight, result]: members) {
			boost::json::array subjson{weight, nullptr};
			result->toJSON(subjson[1]);
			array.push_back(std::move(subjson));
		}
	}

	RandomDissolverResult::RandomDissolverResult(const boost::json::value &json) {
		const auto &array = json.as_array();
		for (size_t i = 1; i < array.size(); ++i) {
			members.push_back(DissolverResult::fromJSON(array.at(i)));
		}
	}

	void RandomDissolverResult::add(const std::shared_ptr<Game> &game, std::vector<ItemStackPtr> &stacks) {
		if (!members.empty()) {
			choose(members)->add(game, stacks);
		}
	}

	void RandomDissolverResult::toJSON(boost::json::value &json) const {
		auto &array = json.emplace_array();
		array.emplace_back("?");
		for (const auto &member: members) {
			boost::json::value subjson;
			member->toJSON(subjson);
			array.push_back(std::move(subjson));
		}
	}

	ChemicalResult::ChemicalResult(const boost::json::value &json):
		formula(json.is_null()? "" : json.get_string()) {}

	void ChemicalResult::add(const std::shared_ptr<Game> &game, std::vector<ItemStackPtr> &stacks) {
		if (formula.empty()) {
			return;
		}

		if (formula.find(':') == std::string::npos) {
			stacks.push_back(ItemStack::create(game, "base:item/chemical", 1, boost::json::value{{"formula", formula}}));
		} else {
			stacks.push_back(ItemStack::create(game, Identifier(formula), 1));
		}
	}

	void ChemicalResult::toJSON(boost::json::value &json) const {
		json = formula;
	}

	MultiChemicalResult::MultiChemicalResult(const boost::json::value &json):
		result(DissolverResult::fromJSON(json.at(1))), count(boost::json::value_to<decltype(count)>(json.at(0))) {}

	void MultiChemicalResult::add(const std::shared_ptr<Game> &game, std::vector<ItemStackPtr> &stacks) {
		for (size_t i = 0; i < count; ++i) {
			result->add(game, stacks);
		}
	}

	void MultiChemicalResult::toJSON(boost::json::value &json) const {
		auto &array = json.emplace_array();
		array.emplace_back(count);
		boost::json::value subjson;
		result->toJSON(subjson);
		array.push_back(std::move(subjson));
	}
}
