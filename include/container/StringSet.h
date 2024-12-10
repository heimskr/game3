#pragma once

#include <boost/json/fwd.hpp>

#include <string>
#include <unordered_set>

namespace Game3 {
    struct StringHasher {
        using is_transparent = void;

        inline auto operator()(const std::string &string) const {
            return std::hash<std::string>{}(string);
        }

        inline auto operator()(std::string_view string) const {
            return std::hash<std::string_view>{}(string);
        }
    };

    struct StringPred {
        using is_transparent = void;

        inline bool operator()(const std::string &left, const std::string &right) const {
            return left == right;
        }

        inline bool operator()(const std::string &left, std::string_view right) const {
            return left == right;
        }

        inline bool operator()(std::string_view left, const std::string &right) const {
            return left == right;
        }

        inline bool operator()(std::string_view left, std::string_view right) const {
            return left == right;
        }
    };

    using StringSet = std::unordered_set<std::string, StringHasher, StringPred>;

    // void tag_invoke(boost::json::value_from_tag, boost::json::value &, const StringSet &);
    // StringSet tag_invoke(boost::json::value_to_tag<StringSet>, const boost::json::value &);
}
