#include "pipes/ItemNetwork.h"
#include "pipes/PipeNetwork.h"
#include "realm/Realm.h"
#include "tileentity/InventoriedTileEntity.h"
#include "tileentity/Pipe.h"
#include "util/PairHash.h"

#include <csignal>

namespace Game3 {
	PipeNetwork::PipeNetwork(size_t id_, const std::shared_ptr<Realm> &realm):
		id(id_), weakRealm(realm) {}

	std::unique_ptr<PipeNetwork> PipeNetwork::create(PipeType type, size_t id, const std::shared_ptr<Realm> &realm) {
		switch (type) {
			case PipeType::Item:
				return std::make_unique<ItemNetwork>(id, realm);
			default:
				throw std::invalid_argument("Can't create pipe network with type " + std::to_string(static_cast<int>(type)));
		}
	}

	void PipeNetwork::add(std::weak_ptr<Pipe> pipe) {
		if (auto locked = pipe.lock()) {
			locked->setNetwork(getType(), shared_from_this());
			members.insert(std::move(pipe));
			// Detect insertions
			locked->onNeighborUpdated(Position( 1,  0));
			locked->onNeighborUpdated(Position(-1,  0));
			locked->onNeighborUpdated(Position( 0,  1));
			locked->onNeighborUpdated(Position( 0, -1));
		} else
			throw std::runtime_error("Can't lock pipe in PipeNetwork::add");
	}

	void PipeNetwork::absorb(const std::shared_ptr<PipeNetwork> &other) {
		assert(other);

		const PipeType type = getType();
		assert(other->getType() == type);

		const auto shared = shared_from_this();

		for (const std::weak_ptr<Pipe> &member: other->members)
			if (auto locked = member.lock())
				add(locked);

		other->members.clear();
	}

	void PipeNetwork::partition(const std::shared_ptr<Pipe> &start) {
		auto realm = weakRealm.lock();
		assert(realm);

		const PipeType type = getType();

		std::shared_ptr<PipeNetwork> new_network = PipeNetwork::create(type, realm->pipeLoader.newID(), realm);

		std::unordered_set visited{start};
		std::vector queue{start};

		while (!queue.empty()) {
			auto pipe = queue.back();
			queue.pop_back();
			visited.insert(pipe);
			new_network->add(pipe);

			pipe->getDirections()[type].iterate([&](Direction direction) {
				if (auto neighbor = pipe->getConnected(type, direction); neighbor && !visited.contains(neighbor))
					queue.push_back(neighbor);
			});
		}
	}

	void PipeNetwork::addExtraction(Position position, Direction direction) {
		extractions.emplace(position, direction);
	}

	void PipeNetwork::addInsertion(Position position, Direction direction) {
		INFO("Adding insertion: " << position << ", " << direction);
		insertions.emplace(position, direction);
	}

	bool PipeNetwork::removeExtraction(Position position, Direction direction) {
		return 1 == extractions.erase(std::make_pair(position, direction));
	}

	bool PipeNetwork::removeInsertion(Position position, Direction direction) {
		return 1 == insertions.erase(std::make_pair(position, direction));
	}

	void PipeNetwork::reconsiderInsertion(Position position) {
		RealmPtr realm = weakRealm.lock();
		if (!realm)
			throw std::runtime_error("Couldn't lock realm");

		PipeType type = getType();
		std::unordered_set<Direction> found;

		if (!std::dynamic_pointer_cast<InventoriedTileEntity>(realm->tileEntityAt(position))) {
			for (const Direction direction: ALL_DIRECTIONS)
				removeInsertion(position, direction);
			return;
		}

		for (const Direction direction: ALL_DIRECTIONS) {
			if (auto pipe = std::dynamic_pointer_cast<Pipe>(realm->tileEntityAt(position + flipDirection(direction)))) {
				if (pipe->getDirections()[type][direction] && !pipe->getExtractors()[type][direction])
					addInsertion(position, flipDirection(direction));
				else
					removeInsertion(position, flipDirection(direction));
			}
		}
	}

	void PipeNetwork::tick(Tick tick) {
		lastTick = tick;
	}

	bool PipeNetwork::canTick(Tick tick) {
		return lastTick < tick;
	}
}
