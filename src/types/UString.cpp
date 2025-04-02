#pragma GCC diagnostic ignored "-Wdeprecated-copy"

#include "util/Log.h"
#include "graphics/TextRenderer.h"
#include "test/Testing.h"
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
		if (empty()) {
			return {};
		}

		size_t next = (this->*finder)(delimiter, 0);
		if (next == UString::npos) {
			return {*this};
		}

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

		std::map<UString, float> width_map;

		auto text_width = [&, text_scale](const UString &text) {
			if (auto iter = width_map.find(text); iter != width_map.end()) {
				return iter->second;
			}
			return width_map[text] = texter.textWidth(text, text_scale);
		};

		auto char_width = [&texter, text_scale](gunichar character) {
			return texter.textWidth(character, text_scale);
		};

		const float hyphen_width = char_width('-');
		const float space_width = char_width(' ');

		UString line;
		UString output;
		std::vector<UString> pieces = split(" ");

		if (pieces.empty()) {
			return {};
		}

		std::optional<float> line_width = 0;

		auto get_line_width = [&] {
			if (line_width) {
				return *line_width;
			}
			return *(line_width = text_width(line));
		};

		std::optional<UString> word = pieces.front();

		for (auto iter = pieces.begin(); iter != pieces.end();) {
			if (!word) {
				word = *iter;
			}

			const bool at_end = iter + 1 == pieces.end();

			if (get_line_width() + text_width(*word) + (at_end? 0 : space_width) <= max_width) {
				line += *word;
				if (!at_end) {
					line += ' ';
				}
				line_width.reset();
				word.reset();
				++iter;
				continue;
			}

			if (word->length() >= 5 || line.empty()) {
				if (get_line_width() + text_width(word->substr(0, 2)) + hyphen_width < max_width) {
					for (std::size_t i = 0; i < word->length(); ++i) {
						const auto character = word->at(i);
						const auto width = char_width(character);

						if (get_line_width() + width + hyphen_width >= max_width || word->length() <= 3 + i) {
							output += line;
							output += '-';
							output += '\n';
							line.clear();
							line_width = 0;
							word->erase(0, i);
							break;
						}

						line += character;
						if (line_width) {
							*line_width += width;
						}
					}
				} else {
					if (line.empty()) {
						return *this;
					}
					output += line;
					output += '\n';
					line.clear();
					line_width = 0;
				}
			} else {
				if (line.empty()) {
					return *this;
				}
				output += line;
				output += '\n';
				line.clear();
				line_width = 0;
			}
		}

		output += line;
		return output;
	}

	std::vector<std::pair<Glib::ustring::const_iterator, Glib::ustring::const_iterator>> UString::getLines() const {
		std::vector<std::pair<Glib::ustring::const_iterator, Glib::ustring::const_iterator>> out;

		if (empty()) {
			return out;
		}

		auto left = begin();
		auto right = begin();

		while (right != end()) {
			if (*right == '\n') {
				out.emplace_back(left, right++);
				left = right;
			} else {
				++right;
			}
		}

		return out;
	}
}
