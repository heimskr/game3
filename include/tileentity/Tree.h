#pragma once

#include "tileentity/TileEntity.h"

namespace Game3 {
	class Tree: public TileEntity {
		public:
			constexpr static float MATURITY = 30.f;

			TileID immatureID;
			float age = 0.f;

			Tree(const Tree &) = delete;
			Tree(Tree &&) = default;
			~Tree() override = default;

			Tree & operator=(const Tree &) = delete;
			Tree & operator=(Tree &&) = default;

			TileEntityID getID() const override { return TileEntity::TREE; }

			void toJSON(nlohmann::json &) const override;
			void absorbJSON(const nlohmann::json &) override;
			void tick(float) override;
			bool onInteractNextTo(const std::shared_ptr<Player> &) override;
			void render(SpriteRenderer &) override;

		protected:
			Tree() = default;
			Tree(TileID id_, TileID immature_id, const Position &position_, float age_):
				TileEntity(id_, TileEntity::TREE, position_, true), immatureID(immature_id), age(age_) {}

			friend class TileEntity;
	};
}
