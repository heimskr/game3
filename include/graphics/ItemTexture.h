#pragma once

#include "registry/Registerable.h"

namespace Game3 {
	class Texture;

	class ItemTexture: public NamedRegisterable {
		public:
			constexpr static int DEFAULT_WIDTH = 16;
			constexpr static int DEFAULT_HEIGHT = 16;

			std::weak_ptr<Texture> texture;
			int x = -1;
			int y = -1;
			int width = -1;
			int height = -1;

			ItemTexture() = delete;
			ItemTexture(Identifier, const std::shared_ptr<Texture> &, int x_, int y_, int width_ = DEFAULT_WIDTH, int height_ = DEFAULT_HEIGHT);

			std::shared_ptr<Texture> getTexture();

			explicit operator bool() const;
	};
}
