#pragma once

#include "ui/gl/dialog/DraggableDialog.h"

#include <sigc++/sigc++.h>

namespace Game3 {
	class TextInput;

	class WorldCreatorDialog final: public DraggableDialog {
		public:
			sigc::signal<void(const std::filesystem::path &, std::optional<size_t> seed)> signalSubmit;

			WorldCreatorDialog(UIContext &, float selfScale);

			void init() final;
			void onFocus() final;
			void rescale(float new_scale) final;

		private:
			std::shared_ptr<TextInput> pathInput;
			std::shared_ptr<TextInput> seedInput;

			void submit();
			void randomizeSeed();
	};
}
