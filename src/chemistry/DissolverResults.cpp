#include "Log.h"
#include "chemistry/DissolverResults.h"
#include "item/Item.h"
#include "threading/ThreadContext.h"
#include "util/Util.h"

#include <random>

#include <nlohmann/json.hpp>

namespace Game3 {
	std::vector<ItemStack> DissolverResult::getResult(const std::shared_ptr<Game> &game) {
		std::vector<ItemStack> out;
		add(game, out);
		return out;
	}

	std::vector<ItemStack> DissolverResult::getResult(const std::shared_ptr<Game> &game, const nlohmann::json &json) {
		auto result = fromJSON(json);
		assert(result);
		std::vector<ItemStack> out;
		result->add(game, out);
		return out;
	}

	std::unique_ptr<DissolverResult> DissolverResult::fromJSON(const nlohmann::json &json) {
		if (json.is_string())
			return std::make_unique<ChemicalResult>(json);

		if (json.is_null())
			throw std::runtime_error("Null JSON object encountered in DissolverResult::getResult");

		if (json.at(0).is_number())
			return std::make_unique<MultiChemicalResult>(json);

		const std::string type = json.at(0);

		if (type == "union" || type == "+")
			return std::make_unique<UnionDissolverResult>(json);

		if (type == "weighted" || type == "*")
			return std::make_unique<WeightedDissolverResult>(json);

		if (type == "random" || type == "?")
			return std::make_unique<RandomDissolverResult>(json);

		throw std::invalid_argument("Invalid DissolverResult JSON");
	}

	void to_json(nlohmann::json &json, const DissolverResult &result) {
		result.toJSON(json);
	}

	UnionDissolverResult::UnionDissolverResult(const nlohmann::json &json) {
		for (size_t i = 1; i < json.size(); ++i)
			members.push_back(DissolverResult::fromJSON(json.at(i)));
	}

	void UnionDissolverResult::add(const std::shared_ptr<Game> &game, std::vector<ItemStack> &stacks) {
		for (const std::unique_ptr<DissolverResult> &member: members)
			member->add(game, stacks);
	}

	void UnionDissolverResult::toJSON(nlohmann::json &json) const {
		json.push_back("+");
		for (const auto &member: members) {
			nlohmann::json subjson;
			member->toJSON(subjson);
			json.push_back(std::move(subjson));
		}
	}

	WeightedDissolverResult::WeightedDissolverResult(const nlohmann::json &json) {
		for (size_t i = 1; i < json.size(); ++i) {
			const nlohmann::json &item = json.at(i);
			members.push_back(Member(item.at(0).get<double>(), DissolverResult::fromJSON(item.at(1)))); // clang moment
		}
	}

	void WeightedDissolverResult::add(const std::shared_ptr<Game> &game, std::vector<ItemStack> &stacks) {
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

	void WeightedDissolverResult::toJSON(nlohmann::json &json) const {
		json.push_back("*");
		for (const auto &[weight, result]: members) {
			nlohmann::json subjson{weight, nlohmann::json{}};
			result->toJSON(subjson.at(1));
			json.push_back(std::move(subjson));
		}
	}

	RandomDissolverResult::RandomDissolverResult(const nlohmann::json &json) {
		for (size_t i = 1; i < json.size(); ++i)
			members.push_back(DissolverResult::fromJSON(json.at(i)));
	}

	void RandomDissolverResult::add(const std::shared_ptr<Game> &game, std::vector<ItemStack> &stacks) {
		if (!members.empty())
			choose(members)->add(game, stacks);
	}

	void RandomDissolverResult::toJSON(nlohmann::json &json) const {
		json.push_back("?");
		for (const auto &member: members) {
			nlohmann::json subjson;
			member->toJSON(subjson);
			json.push_back(std::move(subjson));
		}
	}

	ChemicalResult::ChemicalResult(const nlohmann::json &json):
		formula(json.is_null()? "" : json) {}

	void ChemicalResult::add(const std::shared_ptr<Game> &game, std::vector<ItemStack> &stacks) {
		if (formula.empty())
			return;

		if (formula.find(':') == std::string::npos)
			stacks.emplace_back(game, "base:item/chemical", 1, nlohmann::json{{"formula", formula}});
		else
			stacks.emplace_back(game, Identifier(formula), 1);
	}

	void ChemicalResult::toJSON(nlohmann::json &json) const {
		json = formula;
	}

	MultiChemicalResult::MultiChemicalResult(const nlohmann::json &json):
		result(DissolverResult::fromJSON(json.at(1))), count(json.at(0)) {}

	void MultiChemicalResult::add(const std::shared_ptr<Game> &game, std::vector<ItemStack> &stacks) {
		for (size_t i = 0; i < count; ++i)
			result->add(game, stacks);
	}

	void MultiChemicalResult::toJSON(nlohmann::json &json) const {
		json.push_back(count);
		nlohmann::json subjson;
		result->toJSON(subjson);
		json.push_back(std::move(subjson));
	}
}
