#pragma once

#include "ui/gl/tab/Tab.h"
#include "ui/gl/UIContext.h"

#include <memory>

namespace Game3 {
	class Grid;
	class Scroller;

	class SettingsTab: public Tab {
		public:
			using Tab::Tab;

			void init(UIContext &) final;
			void render(UIContext &, const RendererContext &, float x, float y, float width, float height) final;
			void renderIcon(const RendererContext &) final;

		private:
			std::shared_ptr<Scroller> scroller;
			std::shared_ptr<Grid> grid;

			inline void applySetting(auto member_ptr, const auto &value) {
				ui.getRenderers().settings.withUnique([this, member_ptr, &value](ClientSettings &settings) {
					(settings.*member_ptr) = value;
					settings.apply(*ui.getGame());
				});
			}

			void saveSettings();
	};
}