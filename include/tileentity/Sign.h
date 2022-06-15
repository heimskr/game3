#pragma once

#include "tileentity/TileEntity.h"

namespace Game3 {
	class Sign: public TileEntity {
		public:
			std::string text;
			std::string name;

			Sign(const Sign &) = delete;
			Sign(Sign &&) = default;
			~Sign() override = default;

			Sign & operator=(const Sign &) = delete;
			Sign & operator=(Sign &&) = default;

			TileEntityID getID() const override { return TileEntity::SIGN; }

			void toJSON(nlohmann::json &) const override;
			bool onInteractNextTo(const std::shared_ptr<Player> &) override;
			void absorbJSON(const nlohmann::json &) override;
			// void render(SpriteRenderer &) const override;

		protected:
			Sign() = default;
			Sign(TileID id_, const Position &position_, const std::string &text_, const std::string &name_):
				TileEntity(id_, TileEntity::SIGN, position_, false), text(text_), name(name_) {}

			friend class TileEntity;
	};
}
