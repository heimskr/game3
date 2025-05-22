#pragma once

#include "ui/gl/dialog/DraggableDialog.h"
#include "ui/gl/widget/Box.h"

#include <filesystem>

namespace Game3 {
	class FileDialog;
	class Icon;
	class Label;
	class Scroller;

	namespace FileChooser {
		class Row: public Box {
			public:
				Row(FileDialog &dialog, std::filesystem::path path, TexturePtr iconTexture);

				void init() override;

			protected:
				FileDialog &dialog;
				std::filesystem::path path;
				TexturePtr iconTexture;
		};

		class DirectoryRow final: public Row {
			public:
				using Row::Row;
				void init() final;
				bool click(int button, int, int, Modifiers) final;
		};

		class FileRow final: public Row {
			public:
				using Row::Row;
				void init() final;
				bool click(int button, int, int, Modifiers) final;
		};
	}

	class FileDialog: public DraggableDialog {
		public:
			std::filesystem::path currentPath;
			sigc::signal<void(const std::filesystem::path &)> signalSubmit;

			FileDialog(UIContext &, float selfScale, UString title, int width, int height);

			void init() override;
			bool keyPressed(uint32_t key, Modifiers, bool is_repeat) override;
			void rescale(float new_scale) override;

			virtual void selectFile(const std::filesystem::path &path) = 0;
			virtual void selectDirectory(const std::filesystem::path &path) = 0;

			virtual void submit(const std::filesystem::path &path) = 0;
			virtual void populate();

		protected:
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

			virtual bool filter(const std::filesystem::directory_entry &) const;
			virtual TexturePtr getTexture(const std::filesystem::directory_entry &) const;
	};
}
