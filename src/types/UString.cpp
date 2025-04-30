#include "graphics/TextRenderer.h"
#include "net/Buffer.h"
#include "test/Testing.h"
#include "types/UString.h"
#include "util/Log.h"
#include "util/Timer.h"

namespace Game3 {
	UString::UString(Glib::ustring &&other) noexcept {
		*this = std::move(other);
	}

	UString::UString(const UStringSpan &span) {
		reserve(span.size_bytes());
		append(span.begin(), span.end());
	}

	UString & UString::operator=(Glib::ustring &&other) noexcept {
		Glib::ustring::operator=(std::move(other));
		return *this;
	}

	UString & UString::operator=(const UStringSpan &span) {
		clear();
		reserve(span.size_bytes());
		append(span.begin(), span.end());
		return *this;
	}

	UString & UString::operator+=(const UStringSpan &span) {
		reserve(bytes() + span.size_bytes());
		append(span.begin(), span.end());
		return *this;
	}

	UString::operator UStringSpan() const {
		return {begin(), end()};
	}

	std::vector<UStringSpan> UString::split(const UString &delimiter, Glib::ustring::size_type(UString::*finder)(const Glib::ustring &, Glib::ustring::size_type) const) const {
		std::vector<UStringSpan> out;

		if (empty()) {
			return out;
		}

		std::size_t next = (this->*finder)(delimiter, 0);

		if (next == UString::npos) {
			out.emplace_back(*this);
			return out;
		}

		const std::size_t delimiter_length = finder == static_cast<decltype(finder)>(&Glib::ustring::find_first_of)? 1 : delimiter.length();
		std::size_t last = 0;

		auto iter = begin();
		auto next_iter = std::next(iter, next);

		out.emplace_back(iter, next_iter);

		do {
			next_iter = std::next(iter, next - last);
			last = next;
			next = (this->*finder)(delimiter, last + delimiter_length);
			// out.emplace_back(substr(last + delimiter_length, next - last - delimiter_length));
			out.emplace_back(iter, next_iter);
			iter = next_iter;
			if (iter != end()) {
				std::advance(iter, delimiter_length);
			}
		} while (next != UString::npos);

		if (iter != end()) {
			out.emplace_back(iter, end());
		}

		return out;
	}

	UString UString::wrap(const TextRenderer &texter, float max_width, float text_scale) const {
		Timer timer{"UString::wrap"};

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

		UString output;

		for (bool first = true; UStringSpan span: UStringSpan(*this).split("\n")) {
			if (first) {
				first = false;
			} else {
				output += '\n';
			}

			UString line;

			std::vector<UStringSpan> pieces = span.split(" ");

			if (pieces.empty()) {
				continue;
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
						for (std::size_t i = 0; i < word->size(); ++i) {
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
							goto next_span;
						}
						output += line;
						output += '\n';
						line.clear();
						line_width = 0;
					}
				} else {
					if (line.empty()) {
						goto next_span;
					}
					output += line;
					output += '\n';
					line.clear();
					line_width = 0;
				}
			}

			output += line;
			next_span:
		}

		return output;
	}

	UStringSpan UString::span(size_t pos, size_t n) {
		auto start = std::next(begin(), pos);
		auto last = start;

		for (size_t i = 0; i < n && last != end(); ++i) {
			++last;
		}

		return {start, last};
	}

	std::vector<UStringSpan> UString::getLines() const {
		std::vector<UStringSpan> out;

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

	template <>
	UString popBuffer<UString>(Buffer &buffer) {
		return {popBuffer<std::string>(buffer)};
	}

	template <>
	std::string Buffer::getType<UString>(const UString &string, bool in_container) {
		if (in_container) {
			return getType(std::string{}, true);
		}
		return getType(string.raw(), false);
	}

	Buffer & operator+=(Buffer &buffer, const UString &string) {
		return buffer += string.raw();
	}

	Buffer & operator<<(Buffer &buffer, const UString &string) {
		return buffer << string.raw();
	}

	Buffer & operator>>(Buffer &buffer, UString &string) {
		std::string raw;
		buffer >> raw;
		string = std::move(raw);
		return buffer;
	}

	UStringSpan::UStringSpan(iterator left, iterator right):
		left(left),
		right(right) {}

	UStringSpan::operator UString() const {
		auto iter = left;
		UString string;
		while (iter != right) {
			string += *iter++;
		}
		return string;
	}

	UStringSpan::operator std::string() const {
		return std::string(left.base(), right.base());
	}

	UStringSpan::operator std::string_view() const {
		return std::string_view(left.base(), right.base());
	}

	bool UStringSpan::operator==(const UStringSpan &other) const {
		if (this == &other || (left == other.left && right == other.right)) {
			return true;
		}

		if (size_bytes() != other.size_bytes()) {
			return false;
		}

		auto this_iter = left;
		auto that_iter = other.left;

		while (this_iter != right && that_iter != other.right) {
			if (*this_iter != *that_iter) {
				return false;
			}
		}

		return this_iter == right && that_iter == other.right;
	}

	bool UStringSpan::operator==(const UString &other) const {
		if (size_bytes() != other.bytes()) {
			return false;
		}

		auto iter = begin();
		auto that = other.begin();

		for (; iter != end() && that != other.end(); ++iter, ++that) {
			if (*iter != *that) {
				return false;
			}
		}

		return iter == end() && that == other.end();
	}

	bool UStringSpan::operator==(std::string_view view) const {
		return static_cast<std::string_view>(*this) == view;
	}

	bool UStringSpan::operator==(const char *string) const {
		return static_cast<std::string_view>(*this) == std::string_view(string);
	}

	bool UStringSpan::empty() const {
		return left.base() == right.base();
	}

	std::size_t UStringSpan::size_bytes() const {
		std::ptrdiff_t difference = right.base() - left.base();
		assert(difference >= 0);
		return static_cast<std::size_t>(difference);
	}

	std::size_t UStringSpan::size() const {
		return empty()? 0 : std::distance(begin(), end());
	}

	std::vector<UStringSpan> UStringSpan::split(const UString &delimiter) const {
		std::vector<UStringSpan> out;
		split(delimiter, [&](UStringSpan span) {
			out.emplace_back(span);
		});
		return out;
	}

	void UStringSpan::split(const UString &delimiter, const std::function<void(UStringSpan)> &found) const {
		if (empty()) {
			return;
		}

		iterator iter = begin();

		while (iter != end()) {
			auto next = UStringSpan(iter, end()).find(delimiter);
			found({iter, next});

			if (next == end()) {
				return;
			}

			iter = iterator(next.base() + delimiter.bytes());
		}

		found({iter, end()});
	}

	bool UStringSpan::starts_with(const UStringSpan &other) const {
		if (this == &other) {
			return true;
		}

		if (size_bytes() < other.size_bytes()) {
			return false;
		}

		auto iter = begin();
		auto that = other.begin();

		while (iter != end() && that != other.end()) {
			if (*iter != *that) {
				return false;
			}
		}

		return that == other.end();
	}

	UStringSpan::iterator UStringSpan::find(const UStringSpan &other) const {
		if (empty()) {
			return right;
		}

		if (this == &other || other.empty()) {
			return left;
		}

		if (size_bytes() < other.size_bytes()) {
			return right;
		}

		auto iter = begin();

		while (iter != end()) {
			auto start = iter;
			auto that = other.begin();
			bool found = true;

			while (iter != end() && that != other.end()) {
				if (*iter++ != *that++) {
					found = false;
					break;
				}
			}

			if (found && that == other.end()) {
				return start;
			}
		}

		return right;
	}
}
