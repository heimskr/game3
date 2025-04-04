#include "entity/ClientPlayer.h"
#include "game/ClientGame.h"
#include "game/Inventory.h"
#include "graphics/RectangleRenderer.h"
#include "graphics/RendererContext.h"
#include "graphics/TextRenderer.h"
#include "net/LocalClient.h"
#include "packet/AgentMessagePacket.h"
#include "packet/ChunkRequestPacket.h"
#include "packet/ContinuousInteractionPacket.h"
#include "packet/EntityRequestPacket.h"
#include "packet/JumpPacket.h"
#include "packet/MovePlayerPacket.h"
#include "packet/TileEntityRequestPacket.h"
#include "threading/ThreadContext.h"
#include "tile/Tile.h"
#include "ui/Window.h"

#include <cmath>

namespace Game3 {
	namespace {
		inline Tick getMaxMessageAge(ClientGame &game) {
			return 7 * game.getWindow()->settings.tickFrequency;
		}
	}

	ClientPlayer::ClientPlayer():
		Entity(ID()), Player() {}

	std::shared_ptr<ClientPlayer> ClientPlayer::create(const std::shared_ptr<Game> &) {
		return Entity::create<ClientPlayer>();
	}

	void ClientPlayer::tick(const TickArgs &args) {
		Player::tick(args);

		if (lastMessageAge < std::numeric_limits<decltype(lastMessageAge)::value_type>::max())
			++lastMessageAge;

		Direction final_direction = direction;

		if (movingLeft && !movingRight)
			final_direction = Direction::Left;

		if (movingRight && !movingLeft)
			final_direction = Direction::Right;

		if (movingUp && !movingDown)
			final_direction = Direction::Up;

		if (movingDown && !movingUp)
			final_direction = Direction::Down;

		const MovementContext context{
			.clearOffset = false,
			.facingDirection = final_direction,
		};

		if (movingLeft && !movingRight)
			move(Direction::Left, context);

		if (movingRight && !movingLeft)
			move(Direction::Right, context);

		if (movingUp && !movingDown)
			move(Direction::Up, context);

		if (movingDown && !movingUp)
			move(Direction::Down, context);

		direction = final_direction;

		if (firing) {
			if (InventoryPtr inventory = getInventory(0)) {
				if (ItemStackPtr active = inventory->getActive()) {
					Place place = getPlace();
					ClientGamePtr game = getGame()->toClientPointer();
					WindowPtr window = game->getWindow();
					auto [x, y] = window->getMouseCoordinates<double>();
					double x_offset{}, y_offset{};
					place.position = game->translateCanvasCoordinates(x, y, &x_offset, &y_offset);
					active->item->fire(inventory->activeSlot, active, place, window->getModifiers(), {x_offset, y_offset});
				}
			}
		}
	}

	void ClientPlayer::render(const RendererContext &renderers) {
		if (!isVisible()) {
			return;
		}

		TextRenderer &text = renderers.text;

		const auto [row, column] = getPosition();
		const auto [x, y, z] = offset.copyBase();

		GamePtr game = getGame();
		const bool show_message = lastMessageAge.load() < getMaxMessageAge(game->toClient());
		const float health_offset = canShowHealthBar()? -.5 : 0;
		const float name_offset = health_offset + (show_message? -1 : 0);

		LivingEntity::render(renderers);

		if (show_message) {
			text.drawOnMap(lastMessage, {
				.x = static_cast<float>(column) + x + .5,
				.y = static_cast<float>(row) + y - z + health_offset - .25,
				.scaleX = .75,
				.scaleY = .75,
				.align = TextAlign::Center,
			});
		}

#if 0
		constexpr double thickness = 5 / 16.;
		constexpr double shrink = 16 / 16.;
		ClientGamePtr client_game = game->toClientPointer();
		const auto [mouse_x, mouse_y] = client_game->getWindow()->getMouseCoordinates<double>();
		double x_offset{}, y_offset{};
		Position mouse_position = client_game->translateCanvasCoordinates(mouse_x, mouse_y, &x_offset, &y_offset);
		Vector2d target(mouse_position.column - x_offset + 1, mouse_position.row - y_offset + 1);
		Vector2d diff = Vector2d(getPosition()) + Vector2d(offset.x + 0.5, offset.y - offset.z + 0.5) - target;
		Vector2d midpoint = target + diff / 2.0;
		double degrees = diff.atan2() * 180 / M_PI;
		double distance = diff.magnitude();

		renderers.rectangle.drawOnMap(RenderOptions{
			.x = midpoint.x - distance / 2 + shrink / 2,
			.y = midpoint.y - thickness / 2,
			.sizeX = distance - shrink,
			.sizeY = thickness,
			.angle = degrees,
			.color = Color{"#fa4d10bd"},
		});

		renderers.singleSprite.drawOnMap(cacheTexture("resources/sticky_hand.png"), RenderOptions{
			.x = target.x - 0.5,
			.y = target.y - 0.5,
			.sizeX = -1,
			.sizeY = -1,
			.angle = degrees,
		});
#endif

		text.drawOnMap(displayName, {
			.x = float(column) + x + .5,
			.y = float(row) + y - z + name_offset,
			.align = TextAlign::Center,
		});

		ClientGamePtr client_game = getGame()->toClientPointer();

		if (client_game->getPlayer().get() == this) {
			if (InventoryPtr inventory = getInventory(0)) {
				if (ItemStackPtr active = inventory->getActive()) {
					auto window = client_game->getWindow();
					active->renderEffects(*window, renderers, window->getHoveredPosition(), window->getModifiers());
				}
			}
		}
	}

	void ClientPlayer::renderLighting(const RendererContext &renderers) {
		if (!isVisible()) {
			return;
		}

		const auto [row, column] = getPosition();
		const auto [x, y, z] = offset.copyBase();

		constexpr static float radius = 8;

		renderers.circle.drawOnMap(RenderOptions{
			.x = column + x + .5,
			.y = row + y - z + .5,
			.sizeX = radius,
			.sizeY = radius,
			.color = {1, 1, 1, 1},
		}, 0.5);
	}

	void ClientPlayer::stopContinuousInteraction() {
		send(make<ContinuousInteractionPacket>());
	}

	void ClientPlayer::setContinuousInteraction(bool on, Modifiers modifiers) {
		if (on != continuousInteraction) {
			continuousInteraction = on;
			if (on)
				send(make<ContinuousInteractionPacket>(modifiers));
			else
				send(make<ContinuousInteractionPacket>());
		}

		continuousInteractionModifiers = modifiers;
	}

	void ClientPlayer::jump() {
		if (getRidden()) {
			return;
		}

		double z{};
		{
			auto lock = offset.sharedLock();
			z = offset.z;
		}
		if (std::abs(z) <= 0.001) {
			{
				auto lock = velocity.uniqueLock();
				velocity.z = getJumpSpeed();
			}
			GamePtr game = getGame();
			game->toClient().playSound("base:sound/jump", threadContext.getPitch(1.111f));

			EntityPtr self = getSelf();

			if (TileEntityPtr tile_entity = getRealm()->tileEntityAt(getPosition())) {
				tile_entity->onOverlapEnd(self);
			}

			Place place = getPlace();

			for (Layer layer: allLayers) {
				if (TilePtr tile = place.getTile(layer)) {
					tile->jumpedFrom(self, place, layer);
				}
			}

			send(make<JumpPacket>());
		}
	}

	const std::unordered_set<Layer> & ClientPlayer::getVisibleLayers() const {
		static std::unordered_set<Layer> main_layers {Layer::Terrain, Layer::Submerged, Layer::Objects, Layer::Highest};
		return main_layers;
	}

	bool ClientPlayer::move(Direction direction, MovementContext context) {
		if (Entity::move(direction, context)) {
			send(make<MovePlayerPacket>(position, direction, context.facingDirection, offset));
			return true;
		}

		return false;
	}

	void ClientPlayer::addMoney(MoneyCount to_add) {
		setMoney(money + to_add);
	}

	bool ClientPlayer::removeMoney(MoneyCount to_remove) {
		if (money < to_remove)
			return false;

		setMoney(money - to_remove);
		return true;
	}

	void ClientPlayer::setMoney(MoneyCount new_value) {
		Entity::setMoney(new_value);
		auto shared = getShared();
		GamePtr game = getGame();
		ClientGame &client_game = game->toClient();
		if (client_game.getPlayer() == shared) {
			client_game.signalPlayerMoneyUpdate(getShared());
		}
	}

	void ClientPlayer::movedToNewChunk(const std::optional<ChunkPosition> &old_position) {
		if (auto realm = weakRealm.lock()) {
			std::set<ChunkPosition> chunk_requests;
			std::vector<EntityRequest> entity_requests;
			std::vector<TileEntityRequest> tile_entity_requests;

			auto process_chunk = [&](ChunkPosition chunk_position) {
				chunk_requests.insert(chunk_position);

				if (auto entities = realm->getEntities(chunk_position)) {
					auto lock = entities->sharedLock();
					for (const WeakEntityPtr &weak_entity: *entities)
						if (EntityPtr entity = weak_entity.lock())
							entity_requests.emplace_back(*entity);
				}

				if (auto tile_entities = realm->getTileEntities(chunk_position)) {
					auto lock = tile_entities->sharedLock();
					for (const auto &tile_entity: *tile_entities)
						tile_entity_requests.emplace_back(*tile_entity);
				}
			};

			if (old_position) {
				const ChunkRange old_range(*old_position);
				ChunkRange(getChunk()).iterate([&process_chunk, old_range](ChunkPosition chunk_position) {
					if (!old_range.contains(chunk_position))
						process_chunk(chunk_position);
				});
			} else {
				ChunkRange(getChunk()).iterate(process_chunk);
			}

			if (!chunk_requests.empty())
				send(make<ChunkRequestPacket>(*realm, chunk_requests));

			if (!entity_requests.empty())
				send(make<EntityRequestPacket>(realm->id, std::move(entity_requests)));

			if (!tile_entity_requests.empty())
				send(make<TileEntityRequestPacket>(realm->id, std::move(tile_entity_requests)));
		}

		Entity::movedToNewChunk(old_position);
	}

	void ClientPlayer::handleMessage(const std::shared_ptr<Agent> &source, const std::string &name, std::any &data) {
		if (name == "ModuleMessage") {
			auto *buffer = std::any_cast<Buffer>(&data);
			assert(buffer != nullptr);
			const auto module_name = buffer->take<Identifier>();
			const auto message_name = buffer->take<std::string>();
			GamePtr game = getGame();
			game->toClient().getWindow()->moduleMessageBuffer(module_name, source, message_name, std::move(*buffer));
		}
	}

	void ClientPlayer::sendMessage(const std::shared_ptr<Agent> &destination, const std::string &name, std::any &data) {
		assert(destination);
		if (auto *buffer = std::any_cast<Buffer>(&data)) {
			GamePtr game = getGame();
			game->toClient().getClient()->send(make<AgentMessagePacket>(destination->getGID(), name, *buffer));
		} else
			throw std::runtime_error("Expected data to be a Buffer in ClientPlayer::sendMessage");
	}

	void ClientPlayer::setLastMessage(std::string message) {
		lastMessage = std::move(message);
		lastMessageAge = 0;
	}

	void ClientPlayer::face(Direction new_direction) {
		if (direction.exchange(new_direction) == new_direction)
			return;

		send(make<MovePlayerPacket>(position, new_direction, new_direction));
	}

	void ClientPlayer::showText(const UString &text, const UString &name) {
		(void) name;
		getGame()->toClient().getWindow()->openModule("base:module/text", std::any(text.raw()));
	}
}
