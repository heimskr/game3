#include "container/FriendSet.h"
#include "game/Game.h"
#include "pipes/EnergyNetwork.h"
#include "pipes/FluidNetwork.h"
#include "pipes/ItemNetwork.h"
#include "pipes/PipeNetwork.h"
#include "realm/Realm.h"
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
			case PipeType::Fluid:
				return std::make_unique<FluidNetwork>(id, realm);
			case PipeType::Energy:
				return std::make_unique<EnergyNetwork>(id, realm);
			default:
				throw std::invalid_argument("Can't create pipe network with type " + std::to_string(static_cast<int>(type)));
		}
	}

	void PipeNetwork::add(std::weak_ptr<Pipe> pipe) {
		if (std::shared_ptr<Pipe> locked = pipe.lock()) {
			std::shared_ptr<PipeNetwork> shared = shared_from_this();

			if (locked->getNetwork(getType()) == shared)
				return;

			locked->setNetwork(getType(), shared);
			{
				auto lock = members.uniqueLock();
				members.insert(std::move(pipe));
			}
			// Detect insertions and extractions
			locked->onNeighborUpdated(Position( 1,  0));
			locked->onNeighborUpdated(Position(-1,  0));
			locked->onNeighborUpdated(Position( 0,  1));
			locked->onNeighborUpdated(Position( 0, -1));
		} else
			throw std::runtime_error("Can't lock pipe in PipeNetwork::add");
	}

	void PipeNetwork::absorb(std::shared_ptr<PipeNetwork> other) {
		if (this == other.get())
			return;

		// Absorbing while either network is ticking would be unadvisable.
		auto this_lock = uniqueLock();
		auto other_lock = other->uniqueLock();

		assert(other);

		const PipeType type = getType();
		assert(other->getType() == type);

		const std::shared_ptr<PipeNetwork> shared = shared_from_this();

		{
			auto lock = other->members.uniqueLock();
			for (const std::weak_ptr<Pipe> &member: other->members)
				if (std::shared_ptr<Pipe> locked = member.lock())
					add(locked);
			other->members.clear();
		}

		{
			auto lock = other->insertions.uniqueLock();
			for (const auto &[position, direction]: other->insertions)
				addInsertion(position, direction);
			other->insertions.clear();
		}

		{
			auto lock = other->extractions.uniqueLock();
			for (const auto &[position, direction]: other->extractions)
				addExtraction(position, direction);
			other->extractions.clear();
		}

		reset();
	}

	std::shared_ptr<PipeNetwork> PipeNetwork::partition(const std::shared_ptr<Pipe> &start) {
		auto realm = weakRealm.lock();
		assert(realm);

		auto this_lock = uniqueLock();

		const PipeType type = getType();

		std::shared_ptr<PipeNetwork> new_network = PipeNetwork::create(type, realm->pipeLoader.newID(), realm);

		auto new_lock = new_network->uniqueLock();

		std::unordered_set visited{start};
		std::vector queue{start};

		while (!queue.empty()) {
			auto pipe = queue.back();
			queue.pop_back();
			visited.insert(pipe);
			new_network->add(pipe);

			pipe->getDirections()[type].iterate([&](Direction direction) {
				if (std::shared_ptr<Pipe> neighbor = pipe->getConnected(type, direction); neighbor && !neighbor->dying[type] && !visited.contains(neighbor))
					queue.push_back(neighbor);
			});
		}

		reset();
		return new_network;
	}

	void PipeNetwork::addExtraction(Position position, Direction direction) {
		removeInsertion(position, direction);
		{
			auto lock = extractions.uniqueLock();
			extractions.emplace(position, direction);
		}
		reset();
	}

	void PipeNetwork::addInsertion(Position position, Direction direction) {
		removeExtraction(position, direction);
		{
			auto lock = insertions.uniqueLock();
			insertions.emplace(position, direction);
		}
		reset();
	}

	bool PipeNetwork::removeExtraction(Position position, Direction direction) {
		bool out{};
		{
			auto lock = extractions.uniqueLock();
			out = 1 == extractions.erase(std::make_pair(position, direction));
		}
		reset();
		return out;
	}

	bool PipeNetwork::removeInsertion(Position position, Direction direction) {
		bool out{};
		{
			auto lock = insertions.uniqueLock();
			out = 1 == insertions.erase(std::make_pair(position, direction));
		}
		reset();
		return out;
	}

	void PipeNetwork::reconsiderPoints(Position position) {
		RealmPtr realm = weakRealm.lock();
		if (!realm)
			throw std::runtime_error("Couldn't lock realm");

		PipeType type = getType();

		if (!canWorkWith(realm->tileEntityAt(position))) {
			for (const Direction direction: ALL_DIRECTIONS) {
				if (auto pipe = std::dynamic_pointer_cast<Pipe>(realm->tileEntityAt(position + flipDirection(direction)))) {
					if (auto network = pipe->getNetwork(type)) {
						network->removeInsertion(position, direction);
						network->removeExtraction(position, direction);
					}
				}
			}
			return;
		}

		for (const Direction direction: ALL_DIRECTIONS) {
			if (auto pipe = std::dynamic_pointer_cast<Pipe>(realm->tileEntityAt(position + flipDirection(direction)))) {
				if (auto network = pipe->getNetwork(type)) {
					if (pipe->getDirections()[type][direction]) {
						if (pipe->getExtractors()[type][direction]) {
							network->addExtraction(position, flipDirection(direction));
							network->removeInsertion(position, flipDirection(direction));
						} else {
							network->addInsertion(position, flipDirection(direction));
							network->removeExtraction(position, flipDirection(direction));
						}
					} else {
						network->removeInsertion(position, flipDirection(direction));
						network->removeExtraction(position, flipDirection(direction));
					}
				}
			}
		}
	}

	void PipeNetwork::removePipe(const std::shared_ptr<Pipe> &member) {
		const PipeType type = getType();
		member->dying[type] = true;

		{
			auto lock = members.uniqueLock();
			members.erase(member);

			if (members.empty()) {
				lock.unlock();
				lastPipeRemoved(member->getPosition());
			}
		}

		// Partition neighboring pipes if necessary.

		std::vector<Direction> directions = member->getDirections()[type].toVector();
		std::span remaining_directions(directions);
		FriendSet<std::shared_ptr<Pipe>> friends;

		const RealmPtr realm = member->getRealm();

		for (const Direction first_direction: remaining_directions) {
			remaining_directions = remaining_directions.subspan(1);

			const auto [first_pipe, first_network] = member->getNeighbor(type, first_direction);
			if (!first_network)
				continue;

			for (const Direction second_direction: remaining_directions) {
				const auto [second_pipe, second_network] = member->getNeighbor(type, second_direction);
				if (!second_network)
					continue;

				if (first_pipe->reachable(type, second_pipe)) {
					friends.insert(first_pipe, second_pipe);
					friends.insert(second_pipe, first_pipe);
				} else {
					friends.insert(first_pipe);
				}
			}
		}

		for (const auto &[index, set]: friends) {
			std::shared_ptr<Pipe> pipe = *set.begin();
			pipe->getNetwork(type)->partition(pipe);
		}
	}

	void PipeNetwork::tick(Game &game, Tick tick) {
		lastTick = tick;
		game.enqueue([weak = weak_from_this()](const TickArgs &args) {
			if (auto network = weak.lock())
				network->tick(args.game, args.game.getCurrentTick());
		});
	}

	bool PipeNetwork::canTick(Tick tick) {
		return lastTick < tick;
	}
}
