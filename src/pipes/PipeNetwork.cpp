#include "pipes/PipeNetwork.h"
#include "tileentity/Pipe.h"
#include "util/PairHash.h"

namespace Game3 {
	PipeNetwork::PipeNetwork(size_t id_, const std::shared_ptr<Realm> &realm):
		id(id_), weakRealm(realm) {}

	void PipeNetwork::add(std::weak_ptr<Pipe> pipe) {
		if (auto locked = pipe.lock()) {
			locked->setNetwork(getType(), shared_from_this());
			members.insert(std::move(pipe));
		} else
			throw std::runtime_error("Can't lock pipe in PipeNetwork::add");
	}

	void PipeNetwork::absorb(const std::shared_ptr<PipeNetwork> &other) {
		assert(other);

		const PipeType type = getType();
		assert(other->getType() == type);

		const auto shared = shared_from_this();

		for (const std::weak_ptr<Pipe> &member: other->members) {
			if (auto locked = member.lock()) {
				locked->setNetwork(type, shared);
				members.insert(member);
			}
		}

		other->members.clear();
	}

	void PipeNetwork::addExtraction(Position position, Direction direction) {
		extractions.emplace(position, direction);
	}

	void PipeNetwork::addInsertion(Position position, Direction direction) {
		insertions.emplace(position, direction);
	}

	bool PipeNetwork::removeExtraction(Position position, Direction direction) {
		return 1 == extractions.erase(std::make_pair(position, direction));
	}

	bool PipeNetwork::removeInsertion(Position position, Direction direction) {
		return 1 == insertions.erase(std::make_pair(position, direction));
	}

	void PipeNetwork::tick(Tick tick) {
		lastTick = tick;
	}

	bool PipeNetwork::canTick(Tick tick) {
		return lastTick < tick;
	}
}
