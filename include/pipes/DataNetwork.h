#pragma once

#include "pipes/PipeNetwork.h"

namespace Game3 {
	class DataNetwork: public PipeNetwork {
		public:
			DataNetwork(size_t id_, const std::shared_ptr<Realm> &);

			Substance getType() const final { return Substance::Data; }

			bool canWorkWith(const std::shared_ptr<TileEntity> &) const final;
	};
}
