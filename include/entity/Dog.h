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

	class Dog: public Animal {
		public:
			StringGene species{"species", "base:entity/dog"};
			LongGene breed{"breed", 0, 1, 0};

			static Identifier ID() { return {"base", "entity/dog"}; }

			static std::shared_ptr<Dog> create(const std::shared_ptr<Game> &) {
				return Entity::create<Dog>();
			}

			static std::shared_ptr<Dog> fromJSON(const std::shared_ptr<Game> &game, const boost::json::value &json) {
				auto out = Entity::create<Dog>();
				out->absorbJSON(game, json);
				return out;
			}

			std::string getName() const override { return "Dog"; }

			/** You do not get to kill the dog. */
			HitPoints getMaxHealth() const override { return INVINCIBLE; }

			bool canAbsorbGenes(const boost::json::value &) const override;
			void absorbGenes(const boost::json::value &) override;
			void iterateGenes(const std::function<void(Gene &)> &) override;
			void iterateGenes(const std::function<void(const Gene &)> &) const override;

			void render(const RendererContext &) override;
			void encode(Buffer &) override;
			void decode(Buffer &) override;

			void setBreed(LongGene::ValueType);

		friend class Entity;

		protected:
			Dog():
				Entity(ID()), Animal() {}

			static std::vector<Identifier> breeds;
	};
}
