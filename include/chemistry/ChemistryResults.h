#pragma once

#include "Types.h"

#include <memory>
#include <vector>

#include <nlohmann/json_fwd.hpp>

namespace Game3 {
	class Game;
	class ItemStack;

	class ChemistryResult {
		public:
			virtual void add(Game &, std::vector<ItemStack> &) = 0;
			std::vector<ItemStack> getResult(Game &);
			static std::vector<ItemStack> getResult(Game &, const nlohmann::json &);
			static std::unique_ptr<ChemistryResult> fromJSON(const nlohmann::json &);
	};

	class UnionChemistryResult: public ChemistryResult {
		public:
			UnionChemistryResult(const nlohmann::json &);
			void add(Game &, std::vector<ItemStack> &) override;

		private:
			std::vector<std::unique_ptr<ChemistryResult>> members;
	};

	class WeightedChemistryResult: public ChemistryResult {
		public:
			WeightedChemistryResult(const nlohmann::json &);
			void add(Game &, std::vector<ItemStack> &) override;

		private:
			struct Member {
				double weight;
				std::unique_ptr<ChemistryResult> result;
			};

			std::vector<Member> members;
	};

	class RandomChemistryResult: public ChemistryResult {
		public:
			RandomChemistryResult(const nlohmann::json &);
			void add(Game &, std::vector<ItemStack> &) override;

		private:
			std::vector<std::unique_ptr<ChemistryResult>> members;
	};

	class ChemicalResult: public ChemistryResult {
		public:
			ChemicalResult(const nlohmann::json &);
			void add(Game &, std::vector<ItemStack> &) override;

		private:
			std::string formula;
	};

	class MultiChemicalResult: public ChemistryResult {
		public:
			MultiChemicalResult(const nlohmann::json &);
			void add(Game &, std::vector<ItemStack> &) override;

		private:
			std::string formula;
			ItemCount count;
	};
}
