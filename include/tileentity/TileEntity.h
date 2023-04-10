#pragma once

#include <memory>
#include <random>

#include <nlohmann/json.hpp>

#include "Position.h"
#include "Types.h"
#include "game/Agent.h"

namespace Game3 {
	class Entity;
	class Game;
	class Player;
	class Realm;
	class SpriteRenderer;

	class TileEntity: public Agent, public std::enable_shared_from_this<TileEntity> {
		public:
			constexpr static TileEntityID BUILDING         = 1;
			constexpr static TileEntityID TELEPORTER       = 2;
			constexpr static TileEntityID SIGN             = 3;
			constexpr static TileEntityID CHEST            = 4;
			constexpr static TileEntityID STOCKPILE        = 5;
			constexpr static TileEntityID TREE             = 6;
			constexpr static TileEntityID CRAFTING_STATION = 7;
			constexpr static TileEntityID ORE_DEPOSIT      = 8;
			constexpr static TileEntityID GHOST            = 9;
			constexpr static TileEntityID ITEM_SPAWNER     = 10;

			RealmID realmID = 0;
			std::weak_ptr<Realm> weakRealm;
			Identifier tileID;
			Identifier tileEntityID;
			Position position {-1, -1};
			bool solid = false;
			nlohmann::json extraData;

			template <typename T, typename... Args>
			static std::shared_ptr<T> create(std::default_random_engine &rng, Args && ...args) {
				auto out = std::shared_ptr<T>(new T(std::forward<Args>(args)...));
				out->init(rng);
				return out;
			}

			template <typename T, typename... Args>
			static std::shared_ptr<T> create(Args && ...args) {
				auto out = std::shared_ptr<T>(new T(std::forward<Args>(args)...));
				out->init();
				return out;
			}

			~TileEntity() override = default;

			static std::shared_ptr<TileEntity> fromJSON(const nlohmann::json &);

			// At least one of the two init methods must be overridden to prevent an infinite loop!
			virtual void init(std::default_random_engine &);
			virtual void init();

			virtual void tick(Game &, float) {}
			virtual void onSpawn() {}
			virtual void onRemove() {}
			virtual void onNeighborUpdated(Index /* row_offset */, Index /* column_offset */) {}
			/** Returns the TileEntity ID. This is not the tile ID, which corresponds to a tile in the tileset. */
			inline TileEntityID getID() const { return tileEntityID; }
			virtual void render(SpriteRenderer &) {}
			/** Handles when the player interacts with the tile they're on and that tile contains this tile entity. Returns whether anything interesting happened. */
			virtual bool onInteractOn(const std::shared_ptr<Player> &) { return false; }
			/** Handles when the player interacts with the tile in front of them and that tile contains this tile entity. Returns whether anything interesting happened. */
			virtual bool onInteractNextTo(const std::shared_ptr<Player> &) { return false; }
			/** Handles when an entity steps on this tile entity's position. */
			virtual void onOverlap(const std::shared_ptr<Entity> &) {}
			void setRealm(const std::shared_ptr<Realm> &);
			std::shared_ptr<Realm> getRealm() const override;
			const Position & getPosition() const override { return position; }
			void updateNeighbors();
			bool isVisible() const;
			/** Called when the TileEntity is destroyed violently, e.g. by a bomb. Returns false if the TileEntity should survive the destruction. */
			virtual bool kill() { return false; }

		protected:
			TileEntity() = default;
			TileEntity(Identifier tile_id, Identifier tile_entity_id, const Position &position_, bool solid_):
				tileID(std::move(tile_id)), tileEntityID(std::move(tile_entity_id)), position(position_), solid(solid_) {}

			virtual void absorbJSON(const Game &, const nlohmann::json &);
			virtual void toJSON(nlohmann::json &) const;

			friend void to_json(nlohmann::json &, const TileEntity &);
	};

	using TileEntityPtr = std::shared_ptr<TileEntity>;
}
