#pragma once

#include <functional>
#include <optional>
#include <utility>

#include "Position.h"
#include "item/Landfill.h"

namespace Game3 {
	std::optional<Landfill::Result> clayRequirement(const Place &);
}
