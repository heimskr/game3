#pragma once

#include "graphics/Rectangle.h"
#include "threading/LockableSharedPtr.h"
#include "ui/gl/tab/Tab.h"

namespace Game3 {
	class Module;

	class InventoryTab: public Tab {
		public:
			InventoryTab(UIContext &);

			void render(UIContext &, RendererContext &) final;
			void renderIcon(RendererContext &) final;
			void click(int button, int x, int y) final;
			void dragStart(int x, int y) final;
			void dragEnd(int x, int y) final;

			void setModule(std::shared_ptr<Module>);
			Module * getModule(std::shared_lock<DefaultMutex> &);
			Module * getModule(std::unique_lock<DefaultMutex> &);
			void removeModule();

		private:
			Rectangle innerRectangle;
			std::shared_ptr<Module> playerInventoryModule;
			LockableSharedPtr<Module> activeModule;
	};
}
