#pragma once

#include "graphics/Rectangle.h"
#include "threading/LockableSharedPtr.h"
#include "ui/gl/tab/Tab.h"

namespace Game3 {
	class Module;
	class Scroller;

	class InventoryTab: public Tab {
		public:
			InventoryTab(UIContext &);

			void init() final;
			void render(UIContext &, const RendererContext &, float x, float y, float width, float height) final;
			void renderIcon(const RendererContext &) final;
			bool click(UIContext &, int button, int x, int y) final;
			bool dragStart(UIContext &, int x, int y) final;
			bool dragEnd(UIContext &, int x, int y) final;
			bool scroll(UIContext &, float x_delta, float y_delta, int x, int y) final;

			void setModule(std::shared_ptr<Module>);
			Module * getModule(std::shared_lock<DefaultMutex> &);
			Module * getModule(std::unique_lock<DefaultMutex> &);
			void removeModule();

		private:
			LockableSharedPtr<Module> activeModule;
			std::shared_ptr<Module> playerInventoryModule;
			std::shared_ptr<Scroller> playerScroller;
			std::shared_ptr<Scroller> moduleScroller;

			std::shared_ptr<Scroller> makePlayerScroller();
			std::shared_ptr<Scroller> makeModuleScroller();
	};
}
