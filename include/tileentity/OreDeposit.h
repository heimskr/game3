#pragma once

#include "item/Item.h"
#include "threading/Atomic.h"
#include "tileentity/TileEntity.h"

namespace Game3 {
	struct Ore: NamedRegisterable {
		ItemStackPtr stack;
		Identifier tilename;
		Identifier regenTilename;
		float tooldownMultiplier;
		uint32_t maxUses;
		float cooldown;

		Ore(Identifier, ItemStackPtr, Identifier tilename_, Identifier regen_tilename, float tooldown_multiplier, uint32_t max_uses, float cooldown_);

		static Ore fromJSON(const std::shared_ptr<Game> &, const nlohmann::json &);
	};

	class OreDeposit: public TileEntity {
		public:
			static Identifier ID() { return {"base", "te/ore_deposit"}; }
			Lockable<Identifier> oreType;
			Atomic<bool> ready = true;
			Atomic<uint32_t> uses = 0;

			std::string getName() const override { return "Ore Deposit"; }

			void toJSON(nlohmann::json &) const override;
			void absorbJSON(const std::shared_ptr<Game> &, const nlohmann::json &) override;
			void tick(const TickArgs &) override;
			bool onInteractNextTo(const std::shared_ptr<Player> &, Modifiers, const ItemStackPtr &, Hand) override;
			void render(SpriteRenderer &) override;
			const Ore & getOre(const Game &) const;

			void encode(Game &, Buffer &) override;
			void decode(Game &, Buffer &) override;

		protected:
			OreDeposit() = default;
			OreDeposit(const Ore &ore, const Position &position_, uint32_t uses_ = 0);

		friend class TileEntity;
	};
}
