#include "types/Layer.h"

namespace Game3 {
	const std::unordered_map<std::string, Layer> layerMap{
		{"bedrock",    Layer::Bedrock},
		{"soil",       Layer::Soil},
		{"vegetation", Layer::Vegetation},
		{"flooring",   Layer::Flooring},
		{"snow",       Layer::Snow},
		{"submerged",  Layer::Submerged},
		{"objects",    Layer::Objects},
		{"highest",    Layer::Highest},
	};
}
