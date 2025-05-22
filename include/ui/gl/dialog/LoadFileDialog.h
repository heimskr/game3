#pragma once

#include "ui/gl/dialog/FileDialog.h"
#include "ui/gl/Types.h"

#include <sigc++/sigc++.h>

#include <filesystem>
#include <memory>

namespace Game3 {
	class Box;
	class Icon;
	class Label;
	class Scroller;

	class LoadFileDialog: public FileDialog {
		public:
			using FileDialog::FileDialog;

			void init() override;
			void selectFile(const std::filesystem::path &path) override;
			void selectDirectory(const std::filesystem::path &path) override;
			void submit(const std::filesystem::path &path) override;
			void populate() override;

		private:
			std::shared_ptr<Scroller> scroller;
			std::shared_ptr<Scroller> pathScroller;
			/** Contains the current path header and the entry list. */
			std::shared_ptr<Box> outerVbox;
			std::shared_ptr<Box> entryList;
			/** Contains the "up" button and the path label. */
			std::shared_ptr<Box> header;
			std::shared_ptr<Icon> upIcon;
			std::shared_ptr<Label> pathLabel;
			bool showHidden = true;
	};
}
