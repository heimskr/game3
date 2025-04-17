#include "pipes/DataNetwork.h"
#include "realm/Realm.h"
#include "tileentity/Pipe.h"
#include "util/Log.h"

namespace Game3 {
	DataNetwork::DataNetwork(size_t id_, const std::shared_ptr<Realm> &realm):
		PipeNetwork(id_, realm) {}

	bool DataNetwork::canWorkWith(const std::shared_ptr<TileEntity> &tile_entity) const {
		return std::dynamic_pointer_cast<Pipe>(tile_entity) == nullptr;
	}
}
