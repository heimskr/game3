#pragma once

#include "tileentity/TileEntity.h"

namespace Game3 {
	class Sign: public TileEntity {
		public:
			static Identifier ID() { return {"base", "te/sign"}; }
			std::string text;
			std::string name;

			std::string getName() const override { return name.empty()? "Sign" : name; }

			void toJSON(boost::json::value &) const override;
			bool onInteractNextTo(const std::shared_ptr<Player> &, Modifiers, const ItemStackPtr &, Hand) override;
			void absorbJSON(const std::shared_ptr<Game> &, const boost::json::value &) override;
			bool setField(uint32_t field_name, Buffer &field_value, const PlayerPtr &updater) override;

			void encode(Game &, Buffer &) override;
			void decode(Game &, Buffer &) override;

		protected:
			Sign() = default;
			Sign(Identifier tilename, Position position_, std::string text_, std::string name_);

		friend class TileEntity;
	};
}
