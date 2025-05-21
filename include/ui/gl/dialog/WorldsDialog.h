#pragma once

#include "ui/gl/dialog/FileChooserDialog.h"

namespace Game3 {
	class WorldsDialog final: public FileChooserDialog {
		public:
			WorldsDialog(UIContext &, float selfScale);

		protected:
			bool filter(const std::filesystem::directory_entry &) const override;
			TexturePtr getTexture(const std::filesystem::directory_entry &) const override;
	};
}
