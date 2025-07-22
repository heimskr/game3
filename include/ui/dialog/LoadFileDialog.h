#pragma once

#include "ui/dialog/FileDialog.h"

namespace Game3 {
	class LoadFileDialog: public FileDialog {
		public:
			using FileDialog::FileDialog;

			void selectFile(const std::filesystem::path &path) override;
			void submit(const std::filesystem::path &path) override;
	};
}
