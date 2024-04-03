#include "Log.h"
#include "pipes/DataNetwork.h"
#include "realm/Realm.h"

namespace Game3 {
	DataNetwork::DataNetwork(size_t id_, const std::shared_ptr<Realm> &realm):
		PipeNetwork(id_, realm) {}

	bool DataNetwork::canWorkWith(const std::shared_ptr<TileEntity> &) const {
		return true;
	}
}
