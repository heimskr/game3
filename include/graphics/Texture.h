#pragma once

#include <filesystem>

#include <nlohmann/json_fwd.hpp>

#include "registry/Registerable.h"

namespace Game3 {
	class Texture: public NamedRegisterable {
		public:
			unsigned id = 0;
			int width   = 0;
			int height  = 0;
			int format  = 0;
			int filter  = 0;
			bool alpha  = false;
			std::filesystem::path path;
			std::shared_ptr<uint8_t[]> data;

			Texture(Identifier = {}, bool alpha = true, int filter = -1);
			Texture(Identifier, std::filesystem::path, bool alpha = true, int filter = -1);

			Texture(const Texture &) = delete;
			Texture(Texture &&) = delete;

			Texture & operator=(const Texture &) = delete;
			Texture & operator=(Texture &&) = delete;

			void init();
			void init(std::shared_ptr<uint8_t[]>, int data_width, int data_height);
			void bind(int bind_id = -1);
			bool getValid() const { return valid; }
			void dump(const std::filesystem::path &);

			static std::string filterToString(int);
			static int stringToFilter(const std::string &);

		private:
			bool valid = false;
	};

	using TexturePtr = std::shared_ptr<Texture>;

	TexturePtr cacheTexture(const std::filesystem::path &, bool alpha = true, int filter = -1);
	TexturePtr cacheTexture(const char *, bool alpha = true, int filter = -1);
	TexturePtr cacheTexture(const nlohmann::json &);

	void to_json(nlohmann::json &, const TexturePtr &);
	void from_json(const nlohmann::json &, TexturePtr &);
}
