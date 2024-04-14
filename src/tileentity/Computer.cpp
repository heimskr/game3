#include "entity/Player.h"
#include "game/ClientGame.h"
#include "net/LocalClient.h"
#include "packet/OpenModuleForAgentPacket.h"
#include "pipes/DataNetwork.h"
#include "realm/Realm.h"
#include "scripting/ObjectWrap.h"
#include "scripting/ScriptError.h"
#include "scripting/ScriptUtil.h"
#include "tileentity/Computer.h"
#include "ui/module/ComputerModule.h"
#include "util/Concepts.h"

namespace Game3 {
	Computer::Computer(Identifier tile_id, Position position_):
		TileEntity(std::move(tile_id), ID(), position_, true) {}

	Computer::Computer(Position position_):
		Computer("base:tile/computer"_id, position_) {}

	Computer::~Computer() {
		listeners.clear();
	}

	void Computer::init(Game &game) {
		TileEntity::init(game);
		context = std::make_shared<Context>(std::static_pointer_cast<Computer>(getSelf()));
		engine = std::make_unique<ScriptEngine>(game.shared_from_this(), [&](v8::Isolate *isolate, v8::Local<v8::ObjectTemplate> global) {
			tileEntityTemplate = makeTileEntityTemplate(isolate);
			global->Set(isolate, "TileEntity", tileEntityTemplate.Get(isolate));
		});

		v8::Isolate *isolate = engine->getIsolate();
		v8::Locker locker(isolate);
		v8::Isolate::Scope isolate_scope(isolate);
		v8::HandleScope handle_scope(isolate);

		v8::Local<v8::ObjectTemplate> instance = tileEntityTemplate.Get(isolate)->InstanceTemplate();

		instance->SetAccessor(engine->string("gid"), [](v8::Local<v8::Name>, const v8::PropertyCallbackInfo<v8::Value> &info) {
			auto &wrapper = WeakObjectWrap<TileEntity>::unwrap("TileEntity", info.This());
			auto locked = wrapper.object.lock();
			if (!locked) {
				info.GetReturnValue().SetNull();
			} else {
				info.GetReturnValue().Set(v8::BigInt::New(info.GetIsolate(), locked->getGID()));
			}
		});

		instance->SetAccessor(engine->string("realm"), [](v8::Local<v8::Name>, const v8::PropertyCallbackInfo<v8::Value> &info) {
			auto &wrapper = WeakObjectWrap<TileEntity>::unwrap("TileEntity", info.This());
			auto locked = wrapper.object.lock();
			if (!locked) {
				info.GetReturnValue().SetNull();
			} else {
				info.GetReturnValue().Set(v8::BigInt::New(info.GetIsolate(), locked->getRealm()->getID()));
			}
		});

		instance->SetAccessor(engine->string("name"), [](v8::Local<v8::Name>, const v8::PropertyCallbackInfo<v8::Value> &info) {
			auto &wrapper = WeakObjectWrap<TileEntity>::unwrap("TileEntity", info.This());
			auto locked = wrapper.object.lock();
			if (!locked) {
				info.GetReturnValue().SetNull();
			} else {
				std::string name = locked->getName();
				info.GetReturnValue().Set(v8::String::NewFromUtf8(info.GetIsolate(), name.c_str(), v8::NewStringType::kNormal, name.size()).ToLocalChecked());
			}
		});
	}

	void Computer::handleMessage(const std::shared_ptr<Agent> &source, const std::string &name, std::any &data) {
		if (Buffer *buffer = std::any_cast<Buffer>(&data)) {
			auto lock = listeners.sharedLock();
			auto [iter, end] = listeners.equal_range(name);

			if (iter != end) {
				v8::Isolate *isolate = engine->getIsolate();
				v8::Locker locker(isolate);
				v8::Isolate::Scope isolate_scope(isolate);
				v8::HandleScope handle_scope(isolate);
				v8::Local<v8::Context> context = engine->getContext();

				v8::Local<v8::Object> agent_object;
				v8::Local<v8::Value> argument;

				if (auto tile_entity = std::dynamic_pointer_cast<TileEntity>(source)) {
					v8::Local<v8::Value> tile_entity_args[] {v8::BigInt::New(isolate, int64_t(tile_entity->getGID()))};
					agent_object = tileEntityTemplate.Get(isolate)->GetFunction(context).ToLocalChecked()->CallAsConstructor(context, 1, tile_entity_args).ToLocalChecked().As<v8::Object>();
					auto *wrapper = new ObjectWrap<TileEntity>(tile_entity);
					wrapper->wrap(isolate, "TileEntity", agent_object);
					argument = agent_object;
				} else {
					argument = v8::Null(isolate);
				}

				for (; iter != end; ++iter) {
					v8::Local<v8::Object> new_buffer = engine->getBufferTemplate()->GetFunction(context).ToLocalChecked()->CallAsConstructor(context, 0, nullptr).ToLocalChecked().As<v8::Object>();
					auto &new_wrapper = ObjectWrap<Buffer>::unwrap("Buffer", new_buffer);
					*new_wrapper.object = *buffer;
					v8::Local<v8::Value> args[] {argument, new_buffer};
					std::ignore = iter->second.Get(isolate)->Call(context, v8::Null(isolate), 2, args);
				}
			}
		}

		if (name == "RunScript") {

			Buffer *buffer = std::any_cast<Buffer>(&data);
			if (!buffer)
				return;

			Token token = buffer->take<Token>();
			std::string javascript = buffer->take<std::string>();

			std::function<void(std::string_view)> print = [&](std::string_view text) {
				sendMessage(source, "ModuleMessage", ComputerModule::ID(), "ScriptPrint", token, text);
			};

			assert(engine);

			std::swap(print, engine->onPrint);

			try {
				auto result = engine->execute(javascript, true, [&](v8::Local<v8::Context> script_context) {
					v8::Local<v8::Object> g3 = engine->object({
						{"findAll", engine->makeValue(+[](const v8::FunctionCallbackInfo<v8::Value> &info) {
							auto &context = getExternal<std::shared_ptr<Context>>(info);
							ComputerPtr computer = context->computer.lock();
							if (!computer) {
								info.GetIsolate()->ThrowError("Computer pointer expired");
								return;
							}
							assert(computer->engine);
							ScriptEngine &engine = *computer->engine;
							v8::Local<v8::Array> found = v8::Array::New(engine.getIsolate());
							std::unordered_set<TileEntityPtr> tile_entities;
							RealmPtr realm = computer->getRealm();
							uint32_t index = 0;

							std::unordered_set<GlobalID> gids;

							auto templ = computer->tileEntityTemplate.Get(info.GetIsolate());
							v8::Local<v8::Context> engine_context = engine.getContext();
							v8::Local<v8::Function> function = templ->GetFunction(engine_context).ToLocalChecked();

							std::function<bool(const TileEntityPtr &)> filter;
							if (info.Length() == 1 && info[0]->IsString()) {
								filter = [name = engine.string(info[0])](const TileEntityPtr &tile_entity) {
									return tile_entity->getName() == name;
								};
							} else {
								filter = [](const TileEntityPtr &) { return true; };
							}

							DataNetwork::visitNetworks(computer->getPlace(), [&](DataNetworkPtr network) {
								DataNetwork::visitNetwork(network, [&](const TileEntityPtr &member)  {
									GlobalID gid = member->getGID();
									if (!gids.contains(gid) && filter(member)) {
										v8::Local<v8::BigInt> gid_bigint = v8::BigInt::New(engine.getIsolate(), static_cast<int64_t>(gid));
										v8::Local<v8::Value> argv[] {gid_bigint};
										found->Set(engine.getContext(), index++, function->CallAsConstructor(engine_context, 1, argv).ToLocalChecked()).Check();
										gids.insert(gid);
									}
								});
							});

							info.GetReturnValue().Set(found);
						}, engine->wrap(&context))},

						{"listen", engine->makeValue(+[](const v8::FunctionCallbackInfo<v8::Value> &info) {
							if (info.Length() < 2) {
								info.GetIsolate()->ThrowError("Expected two arguments");
								return;
							}

							if (!info[0]->IsString()) {
								info.GetIsolate()->ThrowError("Expected a string as the first argument");
								return;
							}

							if (!info[1]->IsFunction()) {
								info.GetIsolate()->ThrowError("Expected a function as the second argument");
								return;
							}

							auto &context = getExternal<std::shared_ptr<Context>>(info);
							ComputerPtr computer = context->computer.lock();
							if (!computer) {
								info.GetIsolate()->ThrowError("Computer pointer expired");
								return;
							}
							assert(computer->engine);
							ScriptEngine &engine = *computer->engine;

							auto lock = computer->listeners.uniqueLock();
							computer->listeners.emplace(engine.string(info[0]), v8::Global<v8::Function>(info.GetIsolate(), info[1].As<v8::Function>()));
						}, engine->wrap(&context))},

						{"unlisten", engine->makeValue(+[](const v8::FunctionCallbackInfo<v8::Value> &info) {
							auto &context = getExternal<std::shared_ptr<Context>>(info);
							ComputerPtr computer = context->computer.lock();
							if (!computer) {
								info.GetIsolate()->ThrowError("Computer pointer expired");
								return;
							}
							assert(computer->engine);
							ScriptEngine &engine = *computer->engine;

							if (info.Length() < 1 || !info[0]->IsString()) {
								info.GetIsolate()->ThrowError("Expected a string as the first argument");
								return;
							}

							std::string name = engine.string(info[0]);

							auto &listeners = computer->listeners;
							auto lock = listeners.uniqueLock();

							if (info.Length() == 1) {
								listeners.erase(name);
								return;
							}

							if (!info[1]->IsFunction()) {
								info.GetIsolate()->ThrowError("Expected a function as the second argument");
								return;
							}

							v8::Local<v8::Function> function = info[1].As<v8::Function>();
							std::vector<std::decay_t<decltype(listeners)>::iterator> to_erase;

							auto [iter, end] = listeners.equal_range(name);

							for (; iter != end; ++iter) {
								if (iter->second == function)
									to_erase.push_back(iter);
							}

							for (const auto &iterator: to_erase)
								listeners.erase(iterator);
						}, engine->wrap(&context))},
					});

					script_context->Global()->Set(script_context, engine->string("g3"), g3).Check();
				});

				if (result) {
					std::string result_string = engine->string(result.value());

					constexpr static size_t MAX_LENGTH = LocalClient::MAX_PACKET_SIZE - 512;
					if (MAX_LENGTH < result_string.size()) {
						result_string.erase(result_string.begin() + MAX_LENGTH, result_string.end());
						result_string += "... (truncated)";
					}

					sendMessage(source, "ModuleMessage", ComputerModule::ID(), "ScriptResult", token, result_string);
				}
			} catch (const ScriptError &err) {
				sendMessage(source, "ModuleMessage", ComputerModule::ID(), "ScriptError", token, err.what(), err.line, err.column);
			}

			std::swap(print, engine->onPrint);

		} else if (name == "Echo") {

			return;

		} else {
			TileEntity::handleMessage(source, name, data);
		}
	}

	bool Computer::onInteractNextTo(const PlayerPtr &player, Modifiers modifiers, const ItemStackPtr &, Hand) {
		if (modifiers.onlyAlt()) {
			RealmPtr realm = getRealm();
			realm->queueDestruction(getSelf());
			player->give(ItemStack::create(realm->getGame(), "base:item/computer"_id));
			return true;
		}

		player->send(OpenModuleForAgentPacket(ComputerModule::ID(), getGID()));

		return false;
	}

	v8::Global<v8::FunctionTemplate> Computer::makeTileEntityTemplate(v8::Isolate *isolate) {
		v8::Locker locker(isolate);
		v8::Isolate::Scope isolate_scope(isolate);
		v8::HandleScope handle_scope(isolate);

		v8::Local<v8::External> external = v8::External::New(isolate, this);

		v8::Local<v8::FunctionTemplate> templ = v8::FunctionTemplate::New(isolate, [](const v8::FunctionCallbackInfo<v8::Value> &info) {
			v8::Isolate *isolate = info.GetIsolate();

			if (info.Length() != 1 || !info[0]->IsBigInt()) {
				isolate->ThrowError("Expected a BigInt argument");
				return;
			}

			const GlobalID gid = info[0].As<v8::BigInt>()->Uint64Value();
			Computer &computer = getExternal<Computer>(info);
			ScriptEngine &engine = *computer.engine;
			GamePtr game = engine.game.lock();

			TileEntityPtr tile_entity = game->getAgent<TileEntity>(gid);
			if (!tile_entity) {
				isolate->ThrowError("Tile entity not found");
				return;
			}

			v8::Local<v8::Object> this_obj = info.This();

			auto *wrapper = new WeakObjectWrap<TileEntity>(tile_entity);
			wrapper->wrap(engine.getIsolate(), "TileEntity", this_obj);
		}, external);

		v8::Local<v8::ObjectTemplate> instance = templ->InstanceTemplate();

		instance->SetInternalFieldCount(2);

		instance->Set(isolate, "tell", v8::FunctionTemplate::New(isolate, [](const v8::FunctionCallbackInfo<v8::Value> &info) {
			auto &wrapper = WeakObjectWrap<TileEntity>::unwrap("TileEntity", info.This());

			Computer &computer = getExternal<Computer>(info);
			ScriptEngine &engine = *computer.engine;

			TileEntityPtr tile_entity = wrapper.object.lock();
			if (!tile_entity) {
				info.GetIsolate()->ThrowError("Tile entity pointer expired");
				return;
			}

			if (info.Length() != 1 && info.Length() != 2) {
				info.GetReturnValue().Set(engine.string("Invalid number of arguments"));
				return;
			}

			v8::Local<v8::Value> message_name = info[0];
			const GlobalID gid = tile_entity->getGID();

			tile_entity = computer.searchFor(gid);
			if (!tile_entity) {
				info.GetIsolate()->ThrowError(engine.string("Couldn't find connected tile entity with GID " + std::to_string(gid)));
				return;
			}

			v8::Local<v8::Context> script_context = engine.getContext();
			Buffer buffer;

			if (info.Length() == 2) {
				if (info[1]->IsString()) {
					buffer << engine.string(info[1]);
				} else {
					v8::MaybeLocal<v8::Object> maybe_object = info[1]->ToObject(script_context);
					if (maybe_object.IsEmpty()) {
						engine.getIsolate()->ThrowError("Third argument isn't a Buffer object");
						return;
					}

					v8::Local<v8::Object> object = maybe_object.ToLocalChecked();

					if (object->InternalFieldCount() == 2) {
						v8::Local<v8::Data> internal = object->GetInternalField(0);
						if (!internal->IsValue() || engine.string(internal.As<v8::Value>()) != "Buffer") {
							engine.getIsolate()->ThrowError("Third argument isn't a Buffer object");
							return;
						}
					}

					buffer = *ObjectWrap<Buffer>::unwrap("Buffer", object).object;
				}
			}

			std::any any(std::move(buffer));
			computer.sendMessage(tile_entity, engine.string(message_name), any);

			if (Buffer *new_buffer = std::any_cast<Buffer>(&any)) {
				v8::Local<v8::Object> retval;
				if (engine.getBufferTemplate()->GetFunction(script_context).ToLocalChecked()->NewInstance(script_context).ToLocal(&retval)) {
					auto *wrapper = ObjectWrap<Buffer>::make(std::move(*new_buffer));
					wrapper->object->context = computer.getGame();
					wrapper->wrap(engine.getIsolate(), "Buffer", retval);
					info.GetReturnValue().Set(retval);
				} else {
					info.GetReturnValue().SetNull();
				}
			}
		}, external));

		return v8::Global<v8::FunctionTemplate>(isolate, templ);
	}

	TileEntityPtr Computer::searchFor(GlobalID gid) {
		TileEntityPtr out;

		DataNetwork::visitNetworks(getPlace(), [&](const DataNetworkPtr &network) {
			return DataNetwork::visitNetwork(network, [&](const TileEntityPtr &member) {
				if (member->getGID() == gid) {
					out = member;
					return true;
				}

				return false;
			});
		});

		return out;
	}
}
