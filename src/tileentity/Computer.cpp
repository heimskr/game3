#include "entity/Player.h"
#include "game/ClientGame.h"
#include "packet/OpenModuleForAgentPacket.h"
#include "pipes/DataNetwork.h"
#include "realm/Realm.h"
#include "scripting/ScriptError.h"
#include "scripting/ScriptUtil.h"
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

	namespace {
		template <typename Fn>
		void visitNetworks(const Place &place, Fn &&visitor) {
			std::unordered_set<DataNetworkPtr> visited_networks;

			for (const Direction direction: ALL_DIRECTIONS) {
				auto network = std::static_pointer_cast<DataNetwork>(PipeNetwork::findAt(place + direction, Substance::Data));
				if (!network || visited_networks.contains(network))
					continue;

				visited_networks.insert(network);
				visitor(network);
			}
		}
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
				using Context = struct {
					ComputerPtr computer;
					ScriptEngine *engine;
				};

				Context context{std::static_pointer_cast<Computer>(getSelf()), &engine};

				auto result = engine.execute(javascript, true, [&](v8::Local<v8::Context> script_context) {
					v8::Local<v8::Object> foo = engine.object({
						{"findAll", engine.makeValue(+[](const v8::FunctionCallbackInfo<v8::Value> &info) {
							auto &context = getExternal<Context>(info);
							ComputerPtr computer = context.computer;
							ScriptEngine &engine = *context.engine;

							v8::Local<v8::Array> found = v8::Array::New(engine.getIsolate());

							std::unordered_set<TileEntityPtr> tile_entities;

							RealmPtr realm = computer->getRealm();

							uint32_t index = 0;

							auto visit = [&](const auto &set) {
								auto lock = set.sharedLock();
								for (const auto &[position, direction]: set) {
									TileEntityPtr member = realm->tileEntityAt(position);
									if (!member || tile_entities.contains(member))
										continue;
									tile_entities.insert(member);
									found->Set(engine.getContext(), index++, v8::BigInt::New(engine.getIsolate(), static_cast<int64_t>(member->getGID()))).Check();
								}
							};

							visitNetworks(computer->getPlace(), [&](DataNetworkPtr network) {
								visit(network->getInsertions());
								visit(network->getExtractions());
							});

							info.GetReturnValue().Set(found);
						}, engine.wrap(&context))},
					});

					script_context->Global()->Set(script_context, engine.string("g3"), foo).Check();
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
