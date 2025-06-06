#include "util/Log.h"
#include "entity/Player.h"
#include "game/ClientGame.h"
#include "game/Crop.h"
#include "game/EnergyContainer.h"
#include "game/InventorySpan.h"
#include "game/ServerInventory.h"
#include "graphics/SpriteRenderer.h"
#include "graphics/Tileset.h"
#include "item/Plantable.h"
#include "packet/OpenModuleForAgentPacket.h"
#include "realm/Realm.h"
#include "tile/CropTile.h"
#include "tileentity/Autofarmer.h"
#include "ui/gl/module/MultiModule.h"

namespace Game3 {
	namespace {
		constexpr EnergyAmount ENERGY_CAPACITY = 16'000;
		constexpr EnergyAmount ENERGY_PER_OPERATION = 50;
		constexpr ItemCount INPUT_CAPACITY  = 5;
		constexpr ItemCount OUTPUT_CAPACITY = 20;
		constexpr Index DIAMETER = 5;
		constexpr std::chrono::microseconds PERIOD{int64_t(4e6 / (DIAMETER * DIAMETER))};
	}

	Autofarmer::Autofarmer():
		EnergeticTileEntity(ENERGY_CAPACITY) {}

	Autofarmer::Autofarmer(Identifier tile_id, Position position_):
		TileEntity(std::move(tile_id), ID(), position_, true), EnergeticTileEntity(ENERGY_CAPACITY) {}

	Autofarmer::Autofarmer(Position position_):
		Autofarmer("base:tile/autofarmer_s"_id, position_) {}

	void Autofarmer::init(Game &game) {
		TileEntity::init(game);
		HasInventory::setInventory(Inventory::create(shared_from_this(), INPUT_CAPACITY + OUTPUT_CAPACITY), 0);
	}

	void Autofarmer::tick(const TickArgs &args) {
		RealmPtr realm = weakRealm.lock();
		if (!realm || realm->getSide() != Side::Server)
			return;

		Ticker ticker{*this, args};

		const EnergyAmount to_consume = ENERGY_PER_OPERATION * autofarm();
		auto energy_lock = energyContainer->uniqueLock();
		energyContainer->remove(to_consume, true);
		enqueueTick(PERIOD);
	}

	void Autofarmer::toJSON(boost::json::value &json) const {
		TileEntity::toJSON(json);
		InventoriedTileEntity::toJSON(json);
		EnergeticTileEntity::toJSON(json);
		DirectedTileEntity::toJSON(json);
	}

	bool Autofarmer::onInteractNextTo(const PlayerPtr &player, Modifiers modifiers, const ItemStackPtr &, Hand) {
		RealmPtr realm = getRealm();

		if (modifiers.onlyAlt()) {
			realm->queueDestruction(getSelf());
			player->give(ItemStack::create(realm->getGame(), "base:item/autofarmer"_id));
			return true;
		}

		if (modifiers.onlyCtrl()) {
			rotateClockwise();
			return true;
		}

		player->send(make<OpenModuleForAgentPacket>(MultiModule<Substance::Item, Substance::Energy>::ID(), getGID()));
		EnergeticTileEntity::addObserver(player, true);
		InventoriedTileEntity::addObserver(player, true);

		return true;
	}

	void Autofarmer::absorbJSON(const GamePtr &game, const boost::json::value &json) {
		TileEntity::absorbJSON(game, json);
		InventoriedTileEntity::absorbJSON(game, json);
		EnergeticTileEntity::absorbJSON(game, json);
		DirectedTileEntity::absorbJSON(game, json);
	}

	void Autofarmer::encode(Game &game, Buffer &buffer) {
		TileEntity::encode(game, buffer);
		InventoriedTileEntity::encode(game, buffer);
		EnergeticTileEntity::encode(game, buffer);
		DirectedTileEntity::encode(game, buffer);
	}

	void Autofarmer::decode(Game &game, Buffer &buffer) {
		TileEntity::decode(game, buffer);
		InventoriedTileEntity::decode(game, buffer);
		EnergeticTileEntity::decode(game, buffer);
		DirectedTileEntity::decode(game, buffer);
	}

	void Autofarmer::broadcast(bool force) {
		assert(getSide() == Side::Server);

		if (force) {
			TileEntity::broadcast(true);
			return;
		}

		const auto packet = make<TileEntityPacket>(getSelf());

		auto energetic_lock = EnergeticTileEntity::observers.uniqueLock();

		std::erase_if(EnergeticTileEntity::observers, [&](const std::weak_ptr<Player> &weak_player) {
			if (auto player = weak_player.lock()) {
				player->send(packet);
				return false;
			}

			return true;
		});

		auto inventoried_lock = InventoriedTileEntity::observers.uniqueLock();

		std::erase_if(InventoriedTileEntity::observers, [&](const std::weak_ptr<Player> &weak_player) {
			if (auto player = weak_player.lock()) {
				if (!EnergeticTileEntity::observers.contains(player))
					player->send(packet);
				return false;
			}

			return true;
		});
	}

	size_t Autofarmer::autofarm() {
		Position center = position + toPosition(tileDirection) * updiv(DIAMETER, 2);

		const Index odd = DIAMETER % 2;

		// If the diameter is even and the autofarmer is facing east or south, we need to adjust the center
		// of the farmed area by one tile so that the left column or top row doesn't include the autofarmer.
		if (!odd) {
			if (tileDirection == Direction::Right)
				++center.column;
			else
				++center.row;
		}

		bool input_empty = false;
		size_t operations = 0;

		EnergyAmount current_energy{};

		{
			auto lock = energyContainer->sharedLock();
			current_energy = energyContainer->energy;
		}

		if (ENERGY_PER_OPERATION <= current_energy)
			if (autofarm(center + centerOffset, input_empty))
				++operations;

		if (++centerOffset.column == DIAMETER / 2 + odd) {
			centerOffset.column = -DIAMETER / 2;
			if (++centerOffset.row == DIAMETER / 2 + odd)
				centerOffset.row = -DIAMETER / 2;
		}

		return operations;
	}

	bool Autofarmer::autofarm(Position where, bool &input_empty) {
		InventoryPtr inventory = getInventory(0);
		if (!inventory)
			return false;

		RealmPtr realm = getRealm();
		Tileset &tileset = realm->getTileset();
		const Place place(where, realm);

		TileID tile_id{};

		if (std::optional<TileID> maybe = place.get(Layer::Soil); maybe && *maybe != 0) {
			tile_id = *maybe;
		} else {
			return false;
		}

		bool operated = false;

		bool is_farmland = tileset.isInCategory(tile_id, "base:category/farmland");

		if (!is_farmland && tileset.isInCategory(tile_id, "base:category/tillable")) {
			const auto &farmlands = tileset.getCategoryIDs("base:category/farmland");

			if (farmlands.empty()) {
				static bool warned = false;

				if (!warned) {
					warned = true;
					WARN("No tiles found in category base:category/farmland");
				}

				return false;
			}

			place.set(Layer::Soil, *farmlands.begin());
			is_farmland = true;
			operated = true;
		}

		if (!is_farmland)
			return false;

		bool submerged_empty = true;
		TileID submerged{};

		if (std::optional<TileID> maybe = place.get(Layer::Submerged); maybe) {
			submerged = *maybe;
			submerged_empty = *maybe == 0;
		}

		auto input_span = std::make_shared<InventorySpan>(inventory, 0, INPUT_CAPACITY - 1);

		if (!submerged_empty) {
			// Try to harvest.
			GamePtr game = realm->getGame();
			auto crop_tile = std::dynamic_pointer_cast<CropTile>(game->getTile(tileset[submerged]));
			if (!crop_tile || !crop_tile->isRipe(tileset[submerged]))
				return operated;

			InventorySpan output_span(inventory, INPUT_CAPACITY, inventory->getSlotCount() - 1);

			bool any_added = false;

			std::vector<ItemStackPtr> inputs, outputs;

			for (const ItemStackPtr &stack: crop_tile->crop->products.getStacks()) {
				if (stack->hasAttribute("base:attribute/plantable"))
					inputs.push_back(stack);
				else
					outputs.push_back(stack);
			}

			for (const ItemStackPtr &input: inputs) {
				if (ItemStackPtr leftover = input_span->add(input)) {
					if (leftover->count < input->count)
						any_added = true;
					break;
				} else {
					any_added = true;
				}
			}

			for (const ItemStackPtr &output: outputs) {
				if (ItemStackPtr leftover = output_span.add(output)) {
					if (leftover->count < output->count)
						any_added = true;
					break;
				} else {
					any_added = true;
				}
			}

			if (any_added) {
				place.set(Layer::Submerged, 0);
				submerged_empty = true;
			}
		}

		if (submerged_empty) {
			// Try to plant.
			if (input_empty)
				return operated;

			std::shared_ptr<Plantable> plantable;
			Slot slot{};

			ItemStackPtr stack = input_span->firstItem(&slot, [&](const ItemStackPtr &stack, Slot) -> bool {
				plantable = std::dynamic_pointer_cast<Plantable>(stack->item);
				return plantable != nullptr;
			});

			if (!stack) {
				input_empty = true;
				return operated;
			}

			plantable->plant(input_span, slot, stack, place, Layer::Submerged);
			return true;
		}

		return true;
	}
}
