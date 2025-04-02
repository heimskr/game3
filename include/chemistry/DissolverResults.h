#pragma once

#include "types/Types.h"

#include <boost/json/fwd.hpp>

#include <memory>
#include <vector>

namespace Game3 {
	class DissolverResult {
		public:
			virtual ~DissolverResult() = default;
			virtual void add(const GamePtr &, std::vector<ItemStackPtr> &) = 0;
			virtual void toJSON(boost::json::value &) const = 0;
			std::vector<ItemStackPtr> getResult(const GamePtr &);
			static std::vector<ItemStackPtr> getResult(const GamePtr &, const boost::json::value &);
			static std::unique_ptr<DissolverResult> fromJSON(const boost::json::value &);
	};

	void tag_invoke(boost::json::value_from_tag, boost::json::value &, const DissolverResult &);

	class UnionDissolverResult: public DissolverResult {
		public:
			UnionDissolverResult(const boost::json::value &);
			void add(const GamePtr &, std::vector<ItemStackPtr> &) override;
			void toJSON(boost::json::value &) const override;

		private:
			std::vector<std::unique_ptr<DissolverResult>> members;
	};

	class WeightedDissolverResult: public DissolverResult {
		public:
			WeightedDissolverResult(const boost::json::value &);
			void add(const GamePtr &, std::vector<ItemStackPtr> &) override;
			void toJSON(boost::json::value &) const override;

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
			RandomDissolverResult(const boost::json::value &);
			void add(const GamePtr &, std::vector<ItemStackPtr> &) override;
			void toJSON(boost::json::value &) const override;

		private:
			std::vector<std::unique_ptr<DissolverResult>> members;
	};

	class ChemicalResult: public DissolverResult {
		public:
			ChemicalResult(const boost::json::value &);
			void add(const GamePtr &, std::vector<ItemStackPtr> &) override;
			void toJSON(boost::json::value &) const override;

		private:
			std::string formula;
	};

	class MultiChemicalResult: public DissolverResult {
		public:
			MultiChemicalResult(const boost::json::value &);
			void add(const GamePtr &, std::vector<ItemStackPtr> &) override;
			void toJSON(boost::json::value &) const override;

		private:
			std::unique_ptr<DissolverResult> result;
			ItemCount count;
	};
}
