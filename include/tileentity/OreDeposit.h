#pragma once

#include "item/Item.h"
#include "tileentity/TileEntity.h"

namespace Game3 {
	class OreDeposit: public TileEntity {
		public:
			Ore type;
			float timeRemaining = 0.f;
			unsigned uses = 0;

			const float tooldownMultiplier;
			const unsigned maxUses;
			const float cooldown;

			OreDeposit(const OreDeposit &) = delete;
			OreDeposit(OreDeposit &&) = default;
			~OreDeposit() override = default;

			OreDeposit & operator=(const OreDeposit &) = delete;
			OreDeposit & operator=(OreDeposit &&) = default;

			TileEntityID getID() const override { return TileEntity::ORE_DEPOSIT; }

			void init() override {}
			void toJSON(nlohmann::json &) const override;
			void absorbJSON(const nlohmann::json &) override;
			void tick(Game &, float) override;
			bool onInteractNextTo(const std::shared_ptr<Player> &) override;
			void render(SpriteRenderer &) override;
			ItemStack getOreStack(ItemCount count = 1);

			static ItemStack getOreStack(Ore, ItemCount count = 1);
			static TileID getID(Ore);
			static TileID getRegenID(Ore);
			static float getTooldownMultiplier(Ore);
			static unsigned getMaxUses(Ore);
			static float getCooldown(Ore);

		protected:
			OreDeposit() = delete;

			OreDeposit(Ore ore):
				TileEntity(), type(ore), tooldownMultiplier(getTooldownMultiplier(ore)), maxUses(getMaxUses(ore)), cooldown(getCooldown(ore)) {}

			OreDeposit(Ore ore, const Position &position_, float time_remaining = 0.f, unsigned uses_ = 0):
				TileEntity(getID(ore), TileEntity::ORE_DEPOSIT, position_, true), type(ore), timeRemaining(time_remaining), uses(uses_), tooldownMultiplier(getTooldownMultiplier(ore)),
				maxUses(getMaxUses(ore)), cooldown(getCooldown(ore)) {}

			friend class TileEntity;
	};
}
