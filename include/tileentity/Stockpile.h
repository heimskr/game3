#pragma once

#include "tileentity/Chest.h"

namespace Game3 {
	class Stockpile: public Chest {
		public:
			Stockpile(const Stockpile &) = delete;
			Stockpile(Stockpile &&) = default;
			~Stockpile() override = default;

			Stockpile & operator=(const Stockpile &) = delete;
			Stockpile & operator=(Stockpile &&) = default;

			TileEntityID getID() const override { return TileEntity::STOCKPILE; }

			void toJSON(nlohmann::json &) const override;
			bool onInteractNextTo(const std::shared_ptr<Player> &) override;
			void absorbJSON(const nlohmann::json &) override;

		protected:
			Stockpile() = default;
			Stockpile(TileID id_, const Position &position_, const Texture &texture_ = Chest::DEFAULT_TEXTURE);

			friend class TileEntity;
	};
}
