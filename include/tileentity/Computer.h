#pragma once

#include "scripting/ScriptEngine.h"
#include "threading/Lockable.h"
#include "tileentity/TileEntity.h"

#include <v8.h>

#include <map>

namespace Game3 {
	class Computer: public TileEntity {
		public:
			struct Context {
				std::weak_ptr<Computer> computer;
			};

			std::shared_ptr<Context> context;

			static Identifier ID() { return {"base", "te/computer"}; }

			std::string getName() const override { return "Computer"; }
			void init(Game &) override;

			void handleMessage(const std::shared_ptr<Agent> &source, const std::string &name, std::any &data) override;
			bool onInteractNextTo(const std::shared_ptr<Player> &, Modifiers, const ItemStackPtr &, Hand) override;

		private:
			std::unique_ptr<ScriptEngine> engine;
			v8::Global<v8::FunctionTemplate> tileEntityTemplate;
			Lockable<std::multimap<std::string, v8::Global<v8::Function>>> listeners;

			Computer() = default;
			Computer(Identifier tile_id, Position);
			Computer(Position);

			/** Attempts to find a tile entity connected to the computer via a data cable. */
			TileEntityPtr searchFor(GlobalID);

			v8::Global<v8::FunctionTemplate> makeTileEntityTemplate(v8::Isolate *);

		friend class TileEntity;
	};

	using ComputerPtr = std::shared_ptr<Computer>;
}
