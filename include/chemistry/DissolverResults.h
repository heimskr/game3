#pragma once

#include "types/Types.h"

#include <memory>
#include <vector>

#include <nlohmann/json_fwd.hpp>

namespace Game3 {
	class Game;
	class ItemStack;

	class DissolverResult {
		public:
			virtual ~DissolverResult() = default;
			virtual void add(Game &, std::vector<ItemStack> &) = 0;
			virtual void toJSON(nlohmann::json &) const = 0;
			std::vector<ItemStack> getResult(Game &);
			static std::vector<ItemStack> getResult(Game &, const nlohmann::json &);
			static std::unique_ptr<DissolverResult> fromJSON(const nlohmann::json &);
	};

	void to_json(nlohmann::json &, const DissolverResult &);

	class UnionDissolverResult: public DissolverResult {
		public:
			UnionDissolverResult(const nlohmann::json &);
			void add(Game &, std::vector<ItemStack> &) override;
			void toJSON(nlohmann::json &) const override;

		private:
			std::vector<std::unique_ptr<DissolverResult>> members;
	};

	class WeightedDissolverResult: public DissolverResult {
		public:
			WeightedDissolverResult(const nlohmann::json &);
			void add(Game &, std::vector<ItemStack> &) override;
			void toJSON(nlohmann::json &) const override;

		private:
			struct Member {
				double weight;
				std::unique_ptr<DissolverResult> result;

				Member(double weight_, std::unique_ptr<DissolverResult> &&result_):
					weight(weight_), result(std::move(result_)) {}
			};

			std::vector<Member> members;
	};

	class RandomDissolverResult: public DissolverResult {
		public:
			RandomDissolverResult(const nlohmann::json &);
			void add(Game &, std::vector<ItemStack> &) override;
			void toJSON(nlohmann::json &) const override;

		private:
			std::vector<std::unique_ptr<DissolverResult>> members;
	};

	class ChemicalResult: public DissolverResult {
		public:
			ChemicalResult(const nlohmann::json &);
			void add(Game &, std::vector<ItemStack> &) override;
			void toJSON(nlohmann::json &) const override;

		private:
			std::string formula;
	};

	class MultiChemicalResult: public DissolverResult {
		public:
			MultiChemicalResult(const nlohmann::json &);
			void add(Game &, std::vector<ItemStack> &) override;
			void toJSON(nlohmann::json &) const override;

		private:
			std::unique_ptr<DissolverResult> result;
			ItemCount count;
	};
}
