//
// Created by Daniel Garcia on 5/22/2022.
//

#include "exception.h"

namespace dabers {

    void throw_ex(std::string_view fmt_str, const fmt::format_args& args) {
        throw exception{fmt::vformat(fmt_str, args)};
    }

} /* namespace dabers */