#pragma once

#include <memory>

namespace Game3 {
	class Realm;
	class Window;
	struct RendererContext;

	struct RealmRenderer {
		RealmRenderer() = default;
		virtual ~RealmRenderer() = default;
		virtual void render(const RendererContext &, const std::shared_ptr<Realm> &, Window &);
	};
}
