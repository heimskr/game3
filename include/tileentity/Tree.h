#pragma once

#include <optional>

#include "tileentity/TileEntity.h"

namespace Game3 {
	class Tree: public TileEntity {
		public:
			static Identifier ID() { return {"base", "te/tree"}; }
			constexpr static float MATURITY = 30.f;
			constexpr static float HIVE_MATURITY = 60.f;
			constexpr static double CHAR_CHANCE = 0.314159265358979323;

			float age = 0.f;
			float hiveAge = -1.f; // < 0 for no hive

			Tree(const Tree &) = delete;
			Tree(Tree &&) = default;
			~Tree() override = default;

			Tree & operator=(const Tree &) = delete;
			Tree & operator=(Tree &&) = default;

			void onRemove() override;
			void toJSON(nlohmann::json &) const override;
			void absorbJSON(Game &, const nlohmann::json &) override;
			void onSpawn() override;
			void tick(Game &, float) override;
			bool onInteractNextTo(const PlayerPtr &, Modifiers) override;
			bool hasHive() const;
			bool kill() override;
			void render(SpriteRenderer &) override;

			void encode(Game &, Buffer &) override;
			void decode(Game &, Buffer &) override;

		protected:
			Tree() = default;
			Tree(Identifier tilename, Identifier immature_tilename, Position position_, float age_);

			friend class TileEntity;

		private:
			Identifier immatureTilename;
			std::optional<TileID> immatureTileID;
			TileID getImmatureTileID(const Tileset &);
	};
}
