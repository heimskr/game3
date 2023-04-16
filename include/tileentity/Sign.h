#pragma once

#include "tileentity/TileEntity.h"

namespace Game3 {
	class Sign: public TileEntity {
		public:
			static Identifier ID() { return {"base", "te/sign"}; }
			std::string text;
			std::string name;

			Sign(const Sign &) = delete;
			Sign(Sign &&) = default;
			~Sign() override = default;

			Sign & operator=(const Sign &) = delete;
			Sign & operator=(Sign &&) = default;

			void toJSON(nlohmann::json &) const override;
			bool onInteractNextTo(const std::shared_ptr<Player> &) override;
			void absorbJSON(Game &, const nlohmann::json &) override;
			// void render(SpriteRenderer &) const override;

		protected:
			Sign() = default;
			Sign(Identifier tilename, Position position_, std::string text_, std::string name_);

			friend class TileEntity;
	};
}
