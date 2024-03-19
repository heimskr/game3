#pragma once

#include "biology/Gene.h"
#include "entity/Animal.h"

namespace Game3 {
	class Building;
	class Recolor;
	class RenderOptions;

	class Sheep: public Animal {
		public:
			CircularGene hue;
			FloatGene saturation;
			FloatGene valueMultiplier;
			StringGene species;

			static Identifier ID() { return {"base", "entity/sheep"}; }

			static std::shared_ptr<Sheep> create(const std::shared_ptr<Game> &) {
				return Entity::create<Sheep>();
			}

			static std::shared_ptr<Sheep> fromJSON(const std::shared_ptr<Game> &game, const nlohmann::json &json) {
				auto out = Entity::create<Sheep>();
				out->absorbJSON(game, json);
				return out;
			}

			std::string getName() const override { return "Sheep"; }

			void render(const RendererContext &) override;
			Identifier getMilk() const override { return {"base", "fluid/milk"}; }

			bool canAbsorbGenes(const nlohmann::json &) const override;
			void absorbGenes(const nlohmann::json &) override;
			void iterateGenes(const std::function<void(Gene &)> &) override;
			void iterateGenes(const std::function<void(const Gene &)> &) const override;

			void encode(Buffer &) override;
			void decode(Buffer &) override;

			std::vector<ItemStackPtr> getDrops() override {
				std::vector<ItemStackPtr> out = Animal::getDrops();
				out.push_back(ItemStack::create(getGame(), "base:item/raw_meat"));
				return out;
			}

			friend class Entity;

		protected:
			Sheep();

		private:
			std::shared_ptr<Texture> mask;

			void renderBody(const RendererContext &, const RenderOptions &);
			static float sample();
	};
}
