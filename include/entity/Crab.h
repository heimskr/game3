#pragma once

#include "entity/Animal.h"
#include "biology/Gene.h"
#include "data/Identifier.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace Game3 {
	class Building;

	class Crab: public Animal {
		public:
			StringGene species{"species", "base:entity/crab"};
			LongGene breed{"breed", 0, 10, sampleBreed()};

			static Identifier ID() { return {"base", "entity/crab"}; }

			static std::shared_ptr<Crab> create(const std::shared_ptr<Game> &) {
				return Entity::create<Crab>();
			}

			static std::shared_ptr<Crab> fromJSON(const std::shared_ptr<Game> &game, const nlohmann::json &json) {
				auto out = Entity::create<Crab>();
				out->absorbJSON(game, json);
				return out;
			}

			std::string getName() const override { return "Crab"; }

			bool canAbsorbGenes(const nlohmann::json &) const override;
			void absorbGenes(const nlohmann::json &) override;
			void iterateGenes(const std::function<void(Gene &)> &) override;
			void iterateGenes(const std::function<void(const Gene &)> &) const override;

			void render(const RendererContext &) override;
			bool wander() override;
			void encode(Buffer &) override;
			void decode(Buffer &) override;

			void setBreed(LongGene::ValueType);

		friend class Entity;

		protected:
			Crab():
				Entity(ID()), Animal() {}

			static std::map<LongGene::ValueType, Identifier> breeds;

			static LongGene::ValueType sampleBreed();
	};
}
