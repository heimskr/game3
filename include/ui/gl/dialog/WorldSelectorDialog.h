#pragma once

#include "ui/gl/dialog/FileChooserDialog.h"

namespace Game3 {
	class WorldSelectorDialog final: public FileChooserDialog {
		public:
			WorldSelectorDialog(UIContext &, float selfScale);

		protected:
			bool filter(const std::filesystem::directory_entry &) const override;
			TexturePtr getTexture(const std::filesystem::directory_entry &) const override;
	};
}
