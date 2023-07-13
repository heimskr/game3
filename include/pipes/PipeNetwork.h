#pragma once

#include "util/WeakSet.h"

#include <memory>

namespace Game3 {
	class Pipe;

	class PipeNetwork: public std::enable_shared_from_this<PipeNetwork> {
		private:
			WeakSet<Pipe> members;
			size_t id = 0;

		public:
			PipeNetwork(size_t id_);

			void add(std::weak_ptr<Pipe>);
			void absorb(const std::shared_ptr<PipeNetwork> &);
	};
}
