#pragma once

#include "Position.h"
#include "Types.h"
#include "game/Agent.h"
#include "net/Broadcastable.h"
#include "threading/Lockable.h"
#include "ui/Modifiers.h"

#include <memory>
#include <random>

#include <nlohmann/json_fwd.hpp>

namespace Game3 {
	class Buffer;
	class Entity;
	class Game;
	class Player;
	class Realm;
	class RemoteClient;
	class SpriteRenderer;

	class TileEntity: public Agent, public Broadcastable {
		public:
			RealmID realmID = 0;
			std::weak_ptr<Realm> weakRealm;
			Identifier tileID;
			Identifier tileEntityID;
			Lockable<Position> position {-1, -1};
			bool solid = false;
			nlohmann::json extraData;

			template <typename T, typename... Args>
			static std::shared_ptr<T> create(Game &, Args && ...args) {
				return std::shared_ptr<T>(new T(std::forward<Args>(args)...));
			}

			~TileEntity() override = default;
			virtual void destroy();

			static std::shared_ptr<TileEntity> fromJSON(Game &, const nlohmann::json &);

			virtual void init(Game &);
			virtual void tick(Game &, float);
			virtual void onSpawn();
			/** Called after the tile entity is loaded from disk. */
			virtual void onLoad() {}
			virtual void onRemove();
			virtual void onNeighborUpdated(Position /* offset */) {}
			/** Returns the TileEntity ID. This is not the tile ID, which corresponds to a tile in the tileset. */
			inline Identifier getID() const { return tileEntityID; }
			virtual void render(SpriteRenderer &);
			/** Handles when the player interacts with the tile they're on and that tile contains this tile entity. Returns whether anything interesting happened. */
			virtual bool onInteractOn(const std::shared_ptr<Player> &, Modifiers) { return false; }
			/** Handles when the player interacts with the tile in front of them and that tile contains this tile entity. Returns whether anything interesting happened. */
			virtual bool onInteractNextTo(const std::shared_ptr<Player> &, Modifiers) { return false; }
			/** Handles when an entity steps on this tile entity's position. */
			virtual void onOverlap(const std::shared_ptr<Entity> &) {}
			void setRealm(const std::shared_ptr<Realm> &);
			std::shared_ptr<Realm> getRealm() const override;
			Position getPosition() const override { return position.copyBase(); }
			void updateNeighbors() const;
			bool isVisible() const;
			Side getSide() const final;
			Type getAgentType() const final { return Agent::Type::TileEntity; }
			ChunkPosition getChunk() const;
			/** Called when the TileEntity is destroyed violently, e.g. by a bomb. Returns false if the TileEntity should survive the destruction. */
			virtual bool kill() { return false; }
			inline bool is(const Identifier &check) const { return getID() == check; }
			std::string getName() override { return "Unknown TileEntity (" + std::string(tileEntityID) + ')'; }
			virtual Game & getGame() const;
			std::shared_ptr<TileEntity> getSelf();

			virtual void encode(Game &, Buffer &);
			/** More work needs to be done after this to initialize weakRealm. */
			virtual void decode(Game &, Buffer &);

			void sendTo(RemoteClient &, UpdateCounter threshold = 0);
			virtual void broadcast();

			virtual void toJSON(nlohmann::json &) const;

		protected:
			TileID cachedTile = -1;
			bool tileLookupFailed = false;

			TileEntity() = default;
			TileEntity(Identifier tile_id, Identifier tile_entity_id, Position position_, bool solid_):
				tileID(std::move(tile_id)), tileEntityID(std::move(tile_entity_id)), position(std::move(position_)), solid(solid_) {}

			virtual void absorbJSON(Game &, const nlohmann::json &);

			friend void to_json(nlohmann::json &, const TileEntity &);

		public:
			struct Ticker {
				TileEntity &parent;
				Game &game;
				float delta;
				~Ticker() { parent.TileEntity::tick(game, delta); }
			};
	};

	using TileEntityPtr = std::shared_ptr<TileEntity>;
}
