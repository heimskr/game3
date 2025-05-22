#pragma once

#include "ui/gl/dialog/FileDialog.h"
#include "ui/gl/Types.h"

#include <sigc++/sigc++.h>

#include <filesystem>
#include <memory>

namespace Game3 {
	class Label;

	class LoadFileDialog: public FileDialog {
		public:
			using FileDialog::FileDialog;

			void selectFile(const std::filesystem::path &path) override;
			void selectDirectory(const std::filesystem::path &path) override;
			void submit(const std::filesystem::path &path) override;
	};
}
