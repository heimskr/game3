#pragma once

#include <memory>
#include <random>

#include <nlohmann/json.hpp>

#include "Position.h"
#include "Types.h"
#include "game/Agent.h"

namespace Game3 {
	class Buffer;
	class Entity;
	class Game;
	class Player;
	class Realm;
	class RemoteClient;
	class SpriteRenderer;

	class TileEntity: public Agent, public std::enable_shared_from_this<TileEntity> {
		public:
			RealmID realmID = 0;
			std::weak_ptr<Realm> weakRealm;
			Identifier tileID;
			Identifier tileEntityID;
			Position position {-1, -1};
			bool solid = false;
			nlohmann::json extraData;

			template <typename T, typename... Args>
			static std::shared_ptr<T> create(Game &game, Args && ...args) {
				auto out = std::shared_ptr<T>(new T(std::forward<Args>(args)...));
				out->init(game);
				return out;
			}

			~TileEntity() override = default;
			virtual void destroy();

			static std::shared_ptr<TileEntity> fromJSON(Game &, const nlohmann::json &);

			virtual void init(Game &) {}
			virtual void tick(Game &, float) {}
			virtual void onSpawn() {}
			virtual void onRemove() {}
			virtual void onNeighborUpdated(Index /* row_offset */, Index /* column_offset */) {}
			/** Returns the TileEntity ID. This is not the tile ID, which corresponds to a tile in the tileset. */
			inline Identifier getID() const { return tileEntityID; }
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
			Side getSide() const override final;
			Type getAgentType() const override final { return Agent::Type::TileEntity; }
			ChunkPosition getChunk() const;
			/** Called when the TileEntity is destroyed violently, e.g. by a bomb. Returns false if the TileEntity should survive the destruction. */
			virtual bool kill() { return false; }
			inline bool is(const Identifier &check) const { return getID() == check; }

			virtual void encode(Game &, Buffer &);
			/** More work needs to be done after this to initialize weakRealm. */
			virtual void decode(Game &, Buffer &);

			void sendTo(RemoteClient &, UpdateCounter threshold = 0);

		protected:
			TileEntity() = default;
			TileEntity(Identifier tile_id, Identifier tile_entity_id, Position position_, bool solid_):
				tileID(std::move(tile_id)), tileEntityID(std::move(tile_entity_id)), position(std::move(position_)), solid(solid_) {}

			virtual void absorbJSON(Game &, const nlohmann::json &);
			virtual void toJSON(nlohmann::json &) const;

			friend void to_json(nlohmann::json &, const TileEntity &);
	};

	using TileEntityPtr = std::shared_ptr<TileEntity>;
}
