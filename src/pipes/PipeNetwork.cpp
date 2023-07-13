#include "pipes/PipeNetwork.h"
#include "tileentity/Pipe.h"

namespace Game3 {
	void PipeNetwork::add(std::weak_ptr<Pipe> pipe) {
		if (auto locked = pipe.lock()) {
			locked->setNetwork(shared_from_this());
			members.insert(std::move(pipe));
		} else
			throw std::runtime_error("Can't lock pipe in PipeNetwork::add");
	}

	void PipeNetwork::absorb(const std::shared_ptr<PipeNetwork> &other) {
		assert(other);

		const auto shared = shared_from_this();

		for (const std::weak_ptr<Pipe> &member: other->members) {
			if (auto locked = member.lock()) {
				locked->setNetwork(shared);
				members.insert(member);
			}
		}

		other->members.clear();
	}
}
