#pragma once

#include "biology/Gene.h"
#include "entity/Animal.h"

namespace Game3 {
	class Building;

	class Pig: public Animal {
		public:
			StringGene species;
			LongGene variant;

			static Identifier ID() { return {"base", "entity/pig"}; }

			static std::shared_ptr<Pig> create(const std::shared_ptr<Game> &) {
				return Entity::create<Pig>();
			}

			static std::shared_ptr<Pig> fromJSON(const std::shared_ptr<Game> &game, const boost::json::value &json) {
				auto out = Entity::create<Pig>();
				out->absorbJSON(game, json);
				return out;
			}

			std::string getName() const override { return "Pig"; }

			std::vector<ItemStackPtr> getDrops() override;
			bool canAbsorbGenes(const boost::json::value &) const override;
			void absorbGenes(const boost::json::value &) override;
			void iterateGenes(const std::function<void(Gene &)> &) override;
			void iterateGenes(const std::function<void(const Gene &)> &) const override;

			void render(const RendererContext &) override;
			void encode(Buffer &) override;
			void decode(Buffer &) override;

		friend class Entity;

		protected:
			Pig();

			static std::vector<Identifier> variants;
	};
}
