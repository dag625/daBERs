//
// Created by Daniel Garcia on 5/22/2022.
//

#ifndef DABERS_EXCEPTION_H
#define DABERS_EXCEPTION_H

#include <stdexcept>
#include <string_view>
#include <fmt/core.h>

namespace dabers {

    struct exception : public std::runtime_error {
        using std::runtime_error::runtime_error;
    };

    [[noreturn]] void throw_ex(std::string_view fmt_str, const fmt::format_args& args);

    template <typename... Args>
    [[noreturn]] void throw_ex(std::string_view fmt_str, Args&&... args) {
        throw_ex(fmt_str, {fmt::make_format_args(std::forward<Args>(args)...)});
    }

} /* namespace dabers */

#endif //DABERS_EXCEPTION_H
