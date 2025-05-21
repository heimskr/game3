#pragma once

#include "ui/gl/dialog/FileChooserDialog.h"

#include <filesystem>
#include <memory>

namespace Game3 {
	class Box;
	class Scroller;

	class WorldsDialog final: public FileChooserDialog {
		public:
			WorldsDialog(UIContext &, float selfScale);

		protected:
			bool filter(const std::filesystem::directory_entry &) const override;
	};
}
