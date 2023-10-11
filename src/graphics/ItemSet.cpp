#include "graphics/ItemSet.h"

namespace Game3 {
	ItemSet::ItemSet(Identifier identifier_):
		NamedRegisterable(std::move(identifier_)) {}
}
