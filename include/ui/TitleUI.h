#pragma once

#include "tools/Updater.h"
#include "ui/UI.h"

#include <atomic>

namespace Game3 {
	class Aligner;
	class Box;
	class IconButton;
	class Spinner;

	class TitleUI final: public UI {
		public:
			using UI::UI;

			void init(Window &) final;
			void render(const RendererContext &) final;
			Rectangle getPosition() const final;

		private:
			UpdaterPtr updater;
			std::shared_ptr<Aligner> aligner;
			std::shared_ptr<Box> hbox;
			std::shared_ptr<IconButton> updateButton;
			std::shared_ptr<Spinner> spinner;
			std::atomic_bool updating = false;

			std::shared_ptr<TitleUI> getSelf();
			std::weak_ptr<TitleUI> getWeakSelf();
			void setSpinning(bool);
	};
}
