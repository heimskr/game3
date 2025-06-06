#pragma once

#include "registry/Registerable.h"

#include <boost/json/fwd.hpp>

#include <filesystem>
#include <memory>
#include <span>
#include <string>

namespace GL {
	class Texture;
}

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
			void init(std::span<const uint8_t>, int data_width, int data_height);
			void init(int data_width, int data_height);
			void init(const GL::Texture &);
			void repeat();
			bool upload(std::span<const uint8_t>);
			void bind(int bind_id = -1);
			bool getValid() const { return valid; }
			void dump(const std::filesystem::path &);
			void destroy();

			static std::string filterToString(int);
			static int stringToFilter(std::string_view);

		private:
			bool valid = false;
	};

	using TexturePtr = std::shared_ptr<Texture>;

	TexturePtr cacheTexture(const std::filesystem::path &, bool alpha = true, int filter = -1);
	TexturePtr cacheTexture(const char *, bool alpha = true, int filter = -1);
	TexturePtr cacheTexture(const boost::json::value &);

	void tag_invoke(boost::json::value_from_tag, boost::json::value &, const TexturePtr &);
	TexturePtr tag_invoke(boost::json::value_to_tag<TexturePtr>, const boost::json::value &);
}
