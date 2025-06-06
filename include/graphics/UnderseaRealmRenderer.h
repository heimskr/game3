#pragma once

#include "graphics/RealmRenderer.h"

namespace Game3 {
	struct UnderseaRealmRenderer final: RealmRenderer {
		void render(const RendererContext &, const std::shared_ptr<Realm> &, Window &, GameUI &) final;
	};
}
