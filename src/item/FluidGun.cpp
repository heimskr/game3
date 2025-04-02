#include "util/Log.h"
#include "entity/Player.h"
#include "entity/FluidParticle.h"
#include "game/ClientGame.h"
#include "game/Inventory.h"
#include "graphics/RendererContext.h"
#include "graphics/Tileset.h"
#include "item/FluidGun.h"
#include "lib/JSON.h"
#include "packet/UseFluidGunPacket.h"
#include "packet/InventorySlotUpdatePacket.h"
#include "realm/Realm.h"
#include "threading/ThreadContext.h"
#include "tile/Tile.h"
#include "tileentity/FluidHoldingTileEntity.h"
#include "types/PackedTime.h"
#include "types/Position.h"
#include "ui/gl/widget/Hotbar.h"
#include "ui/gl/widget/ProgressBar.h"
#include "ui/gl/Constants.h"
#include "ui/Window.h"

#include <tuple>

namespace Game3 {
	constexpr static double velocityBase = 2;
	constexpr static double velocityVariance = 0.8;
	constexpr static double jitterScale = 0.2;
	constexpr static double sizeBase = 0.166;
	constexpr static double sizeVariance = 0.8;
	constexpr static double shotCostBase = 250; // adjusted based on the tick rate to be the amount per second
	constexpr static double capacity = shotCostBase * 60;

	static inline double getCost(Tick tick_frequency) {
		return shotCostBase / tick_frequency;
	}

	static std::tuple<FluidPtr, double, PackedTime> getFluidGunData(const GamePtr &game, const ConstItemStackPtr &stack) {
		FluidPtr fluid{};
		double amount{};
		PackedTime last_slurp{0};

		stack->data.withShared([&](const boost::json::value &data) {
			if (auto lookup_result = data.try_at("fluid")) {
				if (auto string_result = lookup_result->try_as_string()) {
					fluid = game->getFluid(Identifier(*string_result));
				}
			}

			if (auto lookup_result = data.try_at("level")) {
				amount = getNumber<FluidAmount>(*lookup_result);
			}

			if (auto lookup_result = data.try_at("lastSlurp")) {
				last_slurp = getUint64(*lookup_result);
			}
		});

		return {fluid, amount, last_slurp};
	}

	static void setFluidGunData(const PlayerPtr &player, Slot slot, const ItemStackPtr &stack, const FluidPtr &fluid, double amount, std::optional<PackedTime> last_slurp) {
		stack->data.withUnique([&](boost::json::value &json) {
			boost::json::object *object = json.if_object();
			if (!object) {
				object = &json.emplace_object();
			}

			(*object)["fluid"] = boost::json::value_from(fluid->identifier);
			(*object)["level"] = boost::json::value_from(amount);
			if (last_slurp) {
				(*object)["lastSlurp"] = boost::json::value_from(last_slurp->milliseconds);
			}
		});

		player->send(make<InventorySlotUpdatePacket>(slot, stack));
	}

	static auto makeParticle(const GamePtr &game, const FluidPtr &fluid, const Place &place, std::pair<float, float> offsets) {
		auto [x_offset, y_offset] = offsets;
		PlayerPtr player = place.player;
		const Vector3 player_offset = player->getOffset();
		const Position relative = place.position - player->getPosition();
		const auto x_jitter = threadContext.random(-jitterScale, jitterScale);
		const auto y_jitter = threadContext.random(-jitterScale, jitterScale);
		const Vector3 jitter(x_jitter, y_jitter, 0);
		Vector3 velocity(relative.column + (0.5 - x_offset), relative.row + (0.5 - y_offset), 16.0);
		velocity -= player_offset * Vector3(1, 1, 0) - jitter;
		const double velocity_scale = threadContext.random(velocityBase * velocityVariance, velocityBase / velocityVariance);
		velocity.x *= velocity_scale;
		velocity.y *= velocity_scale;
		velocity.z /= velocity_scale;
		const double size = threadContext.random(sizeBase * sizeVariance, sizeBase / sizeVariance);
		auto entity = FluidParticle::create(game, fluid->registryID, velocity, size, fluid->color, 0, 2);
		entity->spawning = true;
		entity->setRealm(place.realm);
		entity->offset = player_offset - jitter;
		entity->setThrower(player);
		return entity;
	}

	std::string FluidGun::getTooltip(const ConstItemStackPtr &stack) {
		if (!stack->hasGame()) {
			return name;
		}

		auto [fluid, amount, last_slurp] = getFluidGunData(stack->getGame(), stack);

		if (!fluid || amount == 0) {
			return name;
		}

		return std::format("{} ({} x {:.2f})", name, fluid->name, amount);
	}

	bool FluidGun::use(Slot slot, const ItemStackPtr &stack, const Place &place, Modifiers modifiers, std::pair<float, float>) {
		if (!modifiers.shift) {
			return false;
		}

		assert(stack != nullptr);
		GamePtr game = place.getGame();
		assert(game->getSide() == Side::Server);

		RealmPtr realm = place.realm;
		auto [fluid, amount, last_slurp] = getFluidGunData(game, stack);

		if (last_slurp.timeSince() < std::chrono::milliseconds(100)) {
			return true;
		}

		std::optional<FluidTile> fluid_tile = place.getFluid();
		bool can_set = true;

		if (!fluid_tile || fluid_tile->level == 0) {
			if (auto tile_entity = std::dynamic_pointer_cast<FluidHoldingTileEntity>(place.getTileEntity())) {
				if (std::optional<FluidStack> fluid_stack = tile_entity->extractFluid(Direction::Down, true, FluidTile::FULL)) {
					fluid_tile.emplace(fluid_stack->id, fluid_stack->amount);
					can_set = false;
				}
			}
		}

		if (!fluid_tile || fluid_tile->level == 0) {
			if (TilePtr tile = place.getTile(Layer::Terrain)) {
				fluid_tile = tile->yieldFluid(place);
				can_set = false;
			}
		}

		if (fluid_tile) {
			if (fluid_tile->isInfinite()) {
				const double to_slurp = std::min<double>(FluidTile::FULL, capacity - amount);
				if (fluid && fluid->registryID == fluid_tile->id) {
					amount += to_slurp;
				} else {
					fluid = game->getFluid(fluid_tile->id);
					amount = FluidTile::FULL;
				}
			} else {
				const double to_slurp = std::min<double>(fluid_tile->level, capacity - amount);
				if (fluid && fluid->registryID == fluid_tile->id) {
					amount += to_slurp;
				} else {
					fluid = game->getFluid(fluid_tile->id);
					amount = fluid_tile->level;
				}
				fluid_tile->level -= to_slurp;
				if (can_set) {
					place.setFluid(*fluid_tile);
				}
			}

			setFluidGunData(place.player, slot, stack, fluid, amount, PackedTime::now());
		}

		return true;
	}

	bool FluidGun::drag(Slot slot, const ItemStackPtr &stack, const Place &place, Modifiers modifiers, std::pair<float, float> offsets) {
		return use(slot, stack, place, modifiers, offsets);
	}

	bool FluidGun::fire(Slot, const ItemStackPtr &stack, const Place &place, Modifiers modifiers, std::pair<float, float> offsets) {
		if (modifiers.shift) {
			return false;
		}

		assert(stack != nullptr);
		GamePtr game = place.getGame();
		assert(game->getSide() == Side::Client);

		auto [fluid, amount, last_slurp] = getFluidGunData(game, stack);

		auto tick_frequency = static_cast<uint16_t>(game->toClient().getWindow()->settings.withShared([](const ClientSettings &settings) {
			return settings.tickFrequency;
		}));

		game->toClient().send(make<UseFluidGunPacket>(place.position, offsets.first, offsets.second, modifiers, tick_frequency));

		const double cost = getCost(tick_frequency);

		if (!fluid || amount < cost) {
			return false;
		}

		auto entity = makeParticle(game, fluid, place, offsets);
		place.realm->queueEntityInit(std::move(entity), place.player->getPosition());

		static std::chrono::system_clock::time_point last_play{};
		auto now = std::chrono::system_clock::now();
		auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_play).count();
		if (diff > 90) {
			last_play = now;
			place.realm->playSound(place.position, "base:sound/hit", threadContext.getPitch(1.25f));
		}

		return true;
	}

	void FluidGun::renderEffects(Window &window, const RendererContext &renderers, const Position &, Modifiers, const ItemStackPtr &stack) const {
		const auto [fluid, amount, last_slurp] = getFluidGunData(window.game, stack);
		UIContext &ui = window.uiContext;
		Rectangle rectangle = ui.getHotbar()->getLastRectangle();
		rectangle.y -= rectangle.height + 8;
		constexpr static double shrinkage = 3.0;
		rectangle.y += rectangle.height * (1.0 - 1.0 / shrinkage);
		rectangle.height /= shrinkage;
		Color color = fluid? fluid->color : Color{};
		ProgressBar(ui, 1, color, amount / capacity).render(renderers, rectangle);
	}

	bool FluidGun::fireGun(Slot slot, const ItemStackPtr &stack, const Place &place, Modifiers modifiers, std::pair<float, float> offsets, uint16_t tick_frequency) {
		if (modifiers.shift) {
			return false;
		}

		assert(stack != nullptr);
		GamePtr game = place.getGame();
		assert(game->getSide() == Side::Server);

		auto [fluid, amount, last_slurp] = getFluidGunData(game, stack);
		const double cost = getCost(tick_frequency);

		if (!fluid || amount < cost) {
			return false;
		}

		amount -= cost;
		setFluidGunData(place.player, slot, stack, fluid, amount, last_slurp);

		auto entity = makeParticle(game, fluid, place, offsets);
		entity->excludedPlayer = place.player;
		place.realm->queueEntityInit(std::move(entity), place.player->getPosition());
		return true;
	}
}
