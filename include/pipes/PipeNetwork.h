#pragma once

#include "Position.h"
#include "Types.h"
#include "threading/Lockable.h"
#include "util/PairHash.h"
#include "util/WeakSet.h"

#include <memory>
#include <unordered_set>
#include <utility>

namespace Game3 {
	class Pipe;
	class Realm;
	class TileEntity;

	class PipeNetwork: public std::enable_shared_from_this<PipeNetwork> {
		protected:
			using PairSet = std::unordered_set<std::pair<Position, Direction>, PairHash<Position, Direction>>;

			Lockable<WeakSet<Pipe>> members;
			size_t id = 0;
			std::weak_ptr<Realm> weakRealm;
			size_t lastTick = 0;

			Lockable<PairSet> extractions;
			Lockable<PairSet> insertions;


		public:
			PipeNetwork(size_t id_, const std::shared_ptr<Realm> &);

			virtual ~PipeNetwork() = default;

			static std::unique_ptr<PipeNetwork> create(PipeType, size_t id, const std::shared_ptr<Realm> &);

			void add(std::weak_ptr<Pipe>);
			void absorb(const std::shared_ptr<PipeNetwork> &);
			/** Cuts the network into two pieces by setting all pipes reachable from the given pipe to a new network. */
			void partition(const std::shared_ptr<Pipe> &);
			virtual void addExtraction(Position, Direction);
			virtual void addInsertion(Position, Direction);
			virtual bool removeExtraction(Position, Direction);
			virtual bool removeInsertion(Position, Direction);
			/** If there is no inventoried tile entity at the given position, all insertion points for the position are removed.
			 *  Otherwise, the realm is searched for pipe entities neighboring the position. For each direction to which a pipe
			 *  is attached, an insertion point is added; insertion points are removed from directions without attached pipes. */
			virtual void reconsiderInsertion(Position);
			virtual void removePipe(const std::shared_ptr<Pipe> &);
			virtual void lastPipeRemoved(Position) {}
			virtual bool canWorkWith(const std::shared_ptr<TileEntity> &) const { return false; }

			inline const auto & getExtractions() const { return extractions; }
			inline const auto & getInsertions()  const { return insertions; }

			inline auto getID() const { return id; }

			virtual PipeType getType() const = 0;
			virtual void tick(Tick);
			bool canTick(Tick);
	};
}
