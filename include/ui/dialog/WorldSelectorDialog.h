#pragma once

#include "ui/dialog/LoadFileDialog.h"

namespace Game3 {
	class WorldSelectorDialog final: public LoadFileDialog {
		public:
			WorldSelectorDialog(UIContext &, float selfScale);

		protected:
			bool filter(const std::filesystem::directory_entry &) const override;
	};
}
