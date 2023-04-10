#pragma once

#include "item/Item.h"
#include "tileentity/TileEntity.h"

namespace Game3 {
	struct Ore: NamedRegisterable {
		ItemStack stack;
		Identifier tilename;
		Identifier regenTilename;
		float tooldownMultiplier;
		uint32_t maxUses;
		float cooldown;

		Ore(Identifier, ItemStack, Identifier tilename_, Identifier regen_tilename, float tooldown_multiplier, uint32_t max_uses, float cooldown_);

		static Ore fromJSON(const Game &, const nlohmann::json &);
	};

	class OreDeposit: public TileEntity {
		public:
			static Identifier ID() { return {"base", "te/ore_deposit"}; }
			Identifier oreType;
			float timeRemaining = 0.f;
			uint32_t uses = 0;

			OreDeposit(const OreDeposit &) = delete;
			OreDeposit(OreDeposit &&) = default;
			~OreDeposit() override = default;

			OreDeposit & operator=(const OreDeposit &) = delete;
			OreDeposit & operator=(OreDeposit &&) = default;

			void init(Game &) override {}
			void toJSON(nlohmann::json &) const override;
			void absorbJSON(Game &, const nlohmann::json &) override;
			void tick(Game &, float) override;
			bool onInteractNextTo(const std::shared_ptr<Player> &) override;
			void render(SpriteRenderer &) override;
			const Ore & getOre(const Game &) const;

		protected:
			OreDeposit() = delete;
			OreDeposit(const Ore &ore, const Position &position_, float time_remaining = 0.f, uint32_t uses_ = 0);

			friend class TileEntity;
	};
}
