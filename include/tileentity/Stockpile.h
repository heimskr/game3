#pragma once

#include "tileentity/Chest.h"

namespace Game3 {
	class Stockpile: public Chest {
		public:
			static Identifier ID() { return {"base", "te/stockpile"}; }
			Stockpile(const Stockpile &) = delete;
			Stockpile(Stockpile &&) = default;
			~Stockpile() override = default;

			Stockpile & operator=(const Stockpile &) = delete;
			Stockpile & operator=(Stockpile &&) = default;

			void toJSON(nlohmann::json &) const override;
			bool onInteractNextTo(const std::shared_ptr<Player> &, Modifiers) override;
			void absorbJSON(Game &, const nlohmann::json &) override;

		protected:
			Stockpile() = default;
			Stockpile(Identifier tilename, Position position_, std::shared_ptr<Texture> texture_ = cacheTexture(Chest::DEFAULT_TEXTURE_PATH));

			friend class TileEntity;
	};
}
