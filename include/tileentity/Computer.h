#pragma once

#include "scripting/ScriptEngine.h"
#include "tileentity/TileEntity.h"

namespace Game3 {
	class Computer: public TileEntity {
		public:
			static Identifier ID() { return {"base", "te/computer"}; }

			std::string getName() const override { return "Computer"; }
			void init(Game &) override;

			void handleMessage(const std::shared_ptr<Agent> &source, const std::string &name, std::any &data) override;
			// void toJSON(nlohmann::json &) const override;
			bool onInteractNextTo(const std::shared_ptr<Player> &, Modifiers, const ItemStackPtr &, Hand) override;
			// void absorbJSON(const std::shared_ptr<Game> &, const nlohmann::json &) override;

			void encode(Game &, Buffer &) override;
			void decode(Game &, Buffer &) override;

		private:
			Computer() = default;
			Computer(Identifier tile_id, Position);
			Computer(Position);

			ScriptEngine engine;

			/** Attempts to find a tile entity connected to the computer via a data cable. */
			TileEntityPtr searchFor(GlobalID);

		friend class TileEntity;
	};

	using ComputerPtr = std::shared_ptr<Computer>;
}
