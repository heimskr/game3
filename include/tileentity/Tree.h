#pragma once

#include "tileentity/TileEntity.h"

namespace Game3 {
	class Tree: public TileEntity {
		public:
			Tree(const Tree &) = delete;
			Tree(Tree &&) = default;
			~Tree() override = default;

			Tree & operator=(const Tree &) = delete;
			Tree & operator=(Tree &&) = default;

			TileEntityID getID() const override { return TileEntity::TREE; }

			// void toJSON(nlohmann::json &) const override;
			// void absorbJSON(const nlohmann::json &) override;
			void render(SpriteRenderer &) override;

		protected:
			Tree() = default;
			Tree(TileID id_, const Position &position_):
				TileEntity(id_, TileEntity::TREE, position_, true) {}

			friend class TileEntity;
	};
}
