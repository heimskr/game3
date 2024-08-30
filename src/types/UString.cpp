#include "graphics/TextRenderer.h"
#include "types/UString.h"

namespace Game3 {
	UString::UString(Glib::ustring &&other) noexcept {
		*this = std::move(other);
	}

	UString & UString::operator=(Glib::ustring &&other) noexcept {
		Glib::ustring::operator=(std::move(other));
		return *this;
	}

	std::vector<UString> UString::split(const UString &delimiter, Glib::ustring::size_type(UString::*finder)(const Glib::ustring &, Glib::ustring::size_type) const) const {
		if (empty())
			return {};

		size_t next = (this->*finder)(delimiter, 0);
		if (next == UString::npos)
			return {*this};

		std::vector<UString> out;
		const size_t delimiter_length = finder == static_cast<decltype(finder)>(&Glib::ustring::find_first_of)? 1 : delimiter.length();
		size_t last = 0;

		out.emplace_back(substr(0, next));

		while (next != UString::npos) {
			last = next;
			next = (this->*finder)(delimiter, last + delimiter_length);
			out.emplace_back(substr(last + delimiter_length, next - last - delimiter_length));
		}

		return out;
	}

	UString UString::wrap(const TextRenderer &texter, float max_width, float text_scale) const {
		// Credit for this algorithm: Fayabella
		// TODO: cache line width

		auto get_width = [&](const auto &text) {
			return texter.textWidth(text, text_scale);
		};

		const float hyphen_width = get_width('-');
		const float space_width = get_width(' ');

		UString line;
		UString output;
		std::vector<UString> pieces = split(" ");

		for (auto iter = pieces.begin(); iter != pieces.end();) {
			const Glib::ustring word = *iter;

			if (get_width(line) + get_width(word) + space_width <= max_width) {
				line += word;
				line += ' ';
				++iter;

				if (get_width(line) >= max_width) {
					output += line;
					output += '\n';
					line.clear();
				}
			} else if (word.length() >= 5) {
				if (get_width(line) + get_width(word.substr(0, 2)) < max_width - hyphen_width) {
					for (size_t i = 0; i < word.length(); ++i) {
						if (get_width(line) + get_width(word.at(i)) >= max_width - hyphen_width || word.length() <= 3 + i) {
							output += line;
							output += '-';
							output += '\n';
							line.clear();
							*iter = word.substr(i);
							break;
						} else {
							line += word.at(i);
						}
					}
				} else {
					output += line;
					output += '\n';
					line.clear();
				}
			} else {
				output += line;
				output += '\n';
				line.clear();
			}
		}

		output += line;
		return output;
	}
}
