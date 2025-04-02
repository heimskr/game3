#pragma once

#include "tileentity/TileEntity.h"

namespace Game3 {
	class Lamp: public TileEntity {
		public:
			static Identifier ID() { return {"base", "te/lamp"}; }

			std::string getName() const override { return "Lamp"; }

			void toJSON(boost::json::value &) const override;
			void absorbJSON(const GamePtr &, const boost::json::value &) override;

			void setOn(bool);

			void handleMessage(const AgentPtr &source, const std::string &name, std::any &data) override;
			bool onInteractNextTo(const PlayerPtr &, Modifiers, const ItemStackPtr &, Hand) override;
			void encode(Game &, Buffer &) override;
			void decode(Game &, Buffer &) override;

		protected:
			bool on = false;

			Lamp() = default;
			Lamp(Identifier tilename, Position);
			Lamp(Position);

		friend class TileEntity;
	};
}
