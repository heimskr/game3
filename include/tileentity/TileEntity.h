#pragma once

#include "game/Agent.h"
#include "game/Tickable.h"
#include "net/Broadcastable.h"
#include "threading/Lockable.h"
#include "types/Position.h"
#include "types/TickArgs.h"
#include "types/Types.h"
#include "ui/Modifiers.h"

#include <boost/json.hpp>

#include <memory>
#include <random>

namespace Game3 {
	class Buffer;
	class Entity;
	class Game;
	class Player;
	class Realm;
	class GenericClient;
	class SpriteRenderer;
	struct Place;
	struct RendererContext;

	class TileEntity: public Agent, public Broadcastable, public Tickable {
		public:
			RealmID realmID = 0;
			std::weak_ptr<Realm> weakRealm;
			Identifier tileID;
			Identifier tileEntityID;
			Lockable<Position> position{-1, -1};
			bool solid = false;
			boost::json::value extraData;

			~TileEntity() override = default;
			virtual void destroy();

			static std::shared_ptr<TileEntity> fromJSON(const std::shared_ptr<Game> &, const boost::json::value &);

			static std::string getSQL();

			virtual void init(Game &);
			virtual void tick(const TickArgs &);
			virtual void onSpawn();
			/** Called after the tile entity is loaded from disk. */
			virtual void onLoad() {}
			virtual void onRemove();
			virtual void onNeighborUpdated(Position /* offset */) {}
			/** Returns the TileEntity ID. This is not the tile ID, which corresponds to a tile in the tileset. */
			inline Identifier getID() const { return tileEntityID; }
			virtual void render(SpriteRenderer &);
			virtual void renderUpper(SpriteRenderer &);
			virtual void renderLighting(const RendererContext &);
			/** Handles when an entity steps on this tile entity's position. */
			virtual void onOverlap(const EntityPtr &);
			/** Handles when an entity stops being on this tile entity's position.
			 *  Note that this will be called even if there are other entities still standing on the tile entity's position. */
			virtual void onOverlapEnd(const EntityPtr &);
			void setRealm(const std::shared_ptr<Realm> &);
			std::shared_ptr<Realm> getRealm() const override;
			Position getPosition() const override { return position.copyBase(); }
			void updateNeighbors() const;
			bool isVisible() const;
			bool isVisible(const Position &offset) const;
			Side getSide() const final;
			Type getAgentType() const final { return Agent::Type::TileEntity; }
			ChunkPosition getChunk() const;
			/** Called when the TileEntity is destroyed violently, e.g. by a bomb. Returns false if the TileEntity should survive the destruction. */
			virtual bool kill() { return false; }
			inline bool is(const Identifier &check) const { return getID() == check; }
			std::string getName() const override;
			std::shared_ptr<Game> getGame() const override;
			std::shared_ptr<TileEntity> getSelf();
			std::weak_ptr<TileEntity> getWeakSelf();
			void queueDestruction();
			virtual bool mouseOver();
			virtual void mouseOut();
			void setTileID(Identifier);
			void resetTileCache();

			void handleMessage(const std::shared_ptr<Agent> &source, const std::string &name, std::any &) override;

			virtual void encode(Game &, Buffer &);
			/** More work needs to be done after this to initialize weakRealm. */
			virtual void decode(Game &, Buffer &);

			void sendTo(GenericClient &, UpdateCounter threshold = 0);
			using Broadcastable::queueBroadcast;
			void queueBroadcast(bool force) override;
			void tryBroadcast();
			virtual void broadcast(bool force);

			virtual void toJSON(boost::json::value &) const;

			template <typename T, typename... Args>
			static std::shared_ptr<T> create(Args &&...args) {
				return std::shared_ptr<T>(new T(std::forward<Args>(args)...));
			}

			template <typename T, typename... Args>
			static std::shared_ptr<T> spawn(const Place &place, Args &&...args) {
				auto out = create<T>(std::forward<Args>(args)...);
				if (out->spawnIn(place)) {
					return out;
				}
				return {};
			}

			template <typename T, typename... Args>
			static std::shared_ptr<T> spawn(const std::shared_ptr<Realm> &realm, Args &&...args) {
				auto out = create<T>(std::forward<Args>(args)...);
				if (out->spawnIn(Place(out->getPosition(), realm))) {
					return out;
				}
				return {};
			}

		protected:
			TileID cachedTile = -1;
			TileID cachedUpperTile = -1;
			bool tileLookupFailed = false;

			TileEntity() = default;
			TileEntity(Identifier tileID, Identifier tileEntityID, Position position, bool solid);

			std::function<void(const TickArgs &)> getTickFunction();

			template <Duration D>
			requires (!std::is_same_v<D, std::chrono::nanoseconds>)
			Tick enqueueTick(D delay) {
				return enqueueTick(std::chrono::duration_cast<std::chrono::nanoseconds>(delay));
			}

			Tick enqueueTick(std::chrono::nanoseconds);
			Tick enqueueTick() override;

			virtual void absorbJSON(const std::shared_ptr<Game> &, const boost::json::value &);

			friend void tag_invoke(boost::json::value_from_tag, boost::json::value &, const TileEntity &);

		private:
			bool spawnIn(const Place &);

		public:
			struct Ticker {
				TileEntity &parent;
				TickArgs args;
				~Ticker() { parent.TileEntity::tick(args); }
			};
	};

	using TileEntityPtr = std::shared_ptr<TileEntity>;
	using WeakTileEntityPtr = std::weak_ptr<TileEntity>;
}
