#pragma once

#include "tileentity/TileEntity.h"

namespace Game3 {
	class Tree: public TileEntity {
		public:
			constexpr static float MATURITY = 30.f;
			constexpr static float HIVE_MATURITY = 60.f;
			constexpr static double CHAR_CHANCE = 0.314159265358979323;

			TileID immatureID;
			float age = 0.f;
			float hiveAge = -1.f; // < 0 for no hive

			Tree(const Tree &) = delete;
			Tree(Tree &&) = default;
			~Tree() override = default;

			Tree & operator=(const Tree &) = delete;
			Tree & operator=(Tree &&) = default;

			void toJSON(nlohmann::json &) const override;
			void absorbJSON(const nlohmann::json &) override;
			void init(std::default_random_engine &) override;
			using TileEntity::init;
			void tick(Game &, float) override;
			bool onInteractNextTo(const PlayerPtr &) override;
			bool hasHive() const;
			bool kill() override;
			void render(SpriteRenderer &) override;

		protected:
			Tree() = default;
			Tree(TileID id_, TileID immature_id, const Position &position_, float age_):
				TileEntity(id_, TileEntity::TREE, position_, true), immatureID(immature_id), age(age_) {}

			friend class TileEntity;
	};
}
