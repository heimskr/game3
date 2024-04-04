#include "entity/Player.h"
#include "game/ClientGame.h"
#include "packet/OpenModuleForAgentPacket.h"
#include "realm/Realm.h"
#include "scripting/ScriptError.h"
#include "tileentity/Computer.h"
#include "ui/module/ComputerModule.h"

namespace Game3 {
	Computer::Computer(Identifier tile_id, Position position_):
		TileEntity(std::move(tile_id), ID(), position_, true) {}

	Computer::Computer(Position position_):
		Computer("base:tile/computer"_id, position_) {}

	void Computer::init(Game &game) {
		TileEntity::init(game);
	}

	void Computer::handleMessage(const std::shared_ptr<Agent> &source, const std::string &name, std::any &data) {
		if (name == "RunScript") {

			auto *buffer = std::any_cast<Buffer>(&data);
			assert(buffer);

			Token token = buffer->take<Token>();
			std::string javascript = buffer->take<std::string>();

			std::function<void(std::string_view)> print = [&](std::string_view text) {
				sendMessage(source, "ModuleMessage", ComputerModule::ID(), "ScriptPrint", token, text);
			};

			std::swap(print, engine.onPrint);

			try {
				auto result = engine.execute(javascript, true, [&](v8::Local<v8::Context>) {

				});

				if (result)
					sendMessage(source, "ModuleMessage", ComputerModule::ID(), "ScriptResult", token, engine.string(result.value()));
			} catch (const ScriptError &err) {
				sendMessage(source, "ModuleMessage", ComputerModule::ID(), "ScriptError", token, err.what(), err.line, err.column);
			}

			std::swap(print, engine.onPrint);

		} else {

			TileEntity::handleMessage(source, name, data);

		}
	}

	// void Computer::toJSON(nlohmann::json &json) const {
	// 	TileEntity::toJSON(json);
	// }

	bool Computer::onInteractNextTo(const PlayerPtr &player, Modifiers modifiers, const ItemStackPtr &, Hand) {
		if (modifiers.onlyAlt()) {
			RealmPtr realm = getRealm();
			realm->queueDestruction(getSelf());
			player->give(ItemStack::create(realm->getGame(), "base:item/computer"_id));
			return true;
		}

		player->send(OpenModuleForAgentPacket(ComputerModule::ID(), getGID()));
		// addObserver(player, true);

		return false;
	}

	// void Computer::absorbJSON(const GamePtr &game, const nlohmann::json &json) {
	// 	TileEntity::absorbJSON(game, json);
	// }

	void Computer::encode(Game &game, Buffer &buffer) {
		TileEntity::encode(game, buffer);
	}

	void Computer::decode(Game &game, Buffer &buffer) {
		TileEntity::decode(game, buffer);
	}

	// GamePtr Computer::getGame() const {
	// 	return TileEntity::getGame();
	// }
}
