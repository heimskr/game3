#pragma once

#include "ui/gl/dialog/FileDialog.h"

namespace Game3 {
	class TextInput;

	class SaveFileDialog: public FileDialog {
		public:
			using FileDialog::FileDialog;

			void init() override;
			void selectFile(const std::filesystem::path &path) override;
			void submit(const std::filesystem::path &path) override;

		private:
			std::shared_ptr<TextInput> filenameInput;
	};
}
