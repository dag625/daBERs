//
// Created by Daniel Garcia on 5/22/2022.
//

#ifndef DABERS_LENGTH_H
#define DABERS_LENGTH_H

#include "dabers/rules.h"
#include <cstdint>
#include <functional>
#include <concepts>
#include <optional>

namespace dabers {

    enum class length_options : int {
        indefinite_optional,
        indefinite_required,
        definite_required
    };

    std::optional<uint64_t> parse_length(length_options opts, const std::byte*& begin, const std::byte* end);

    std::optional<uint64_t> parse_length(ber, bool constructed, const std::byte*& begin, const std::byte* const end) {
        return parse_length(constructed ? length_options::indefinite_optional : length_options::definite_required,
                            begin, end);
    }

    std::optional<uint64_t> parse_length(cer, bool constructed, const std::byte*& begin, const std::byte* const end) {
        return parse_length(constructed ? length_options::indefinite_required : length_options::definite_required,
                            begin, end);
    }

    std::optional<uint64_t> parse_length(der, bool constructed, const std::byte*& begin, const std::byte* const end) {
        return parse_length(length_options::definite_required, begin, end);
    }

    std::optional<uint64_t> parse_length(rules r, bool constructed, const std::byte*& begin, const std::byte* const end) {
        switch (r) {
            case rules::cer: return parse_length(cer{}, constructed, begin, end);
            case rules::der: return parse_length(der{}, constructed, begin, end);
            default: return parse_length(ber{}, constructed, begin, end);
        }
    }


    bool write_length(uint64_t len, length_options opts, const std::function<void(std::byte)>& output);

    template <std::output_iterator<std::byte> Iter>
    bool write_length(ber, bool, uint64_t len, Iter output) {
        return write_length(len, length_options::definite_required,
                            [&output](std::byte b){ *output = b; ++output; });
    }

    template <std::output_iterator<std::byte> Iter>
    bool write_length(cer, bool constructed, uint64_t len, Iter output) {
        return write_length(len, constructed ? length_options::indefinite_required : length_options::definite_required,
                            [&output](std::byte b){ *output = b; ++output; });
    }

    template <std::output_iterator<std::byte> Iter>
    bool write_length(der, bool, uint64_t len, Iter output) {
        return write_length(len, length_options::definite_required,
                            [&output](std::byte b){ *output = b; ++output; });
    }

    template <std::output_iterator<std::byte> Iter>
    bool write_length(rules r, bool constructed, uint64_t len, Iter output) {
        switch (r) {
            case rules::cer: return write_length(cer{}, constructed, output);
            case rules::der: return write_length(der{}, constructed, output);
            default: return write_length(ber{}, constructed, output);
        }
    }

    template <std::integral T, std::output_iterator<T> Iter>
    bool write_length(ber, bool, uint64_t len, Iter output) {
        return write_length(len, length_options::definite_required,
                            [&output](std::byte b){ *output = static_cast<T>(b); ++output; });
    }

    template <std::integral T, std::output_iterator<T> Iter>
    bool write_length(cer, bool constructed, uint64_t len, Iter output) {
        return write_length(len, constructed ? length_options::indefinite_required : length_options::definite_required,
                            [&output](std::byte b){ *output = static_cast<T>(b); ++output; });
    }

    template <std::integral T, std::output_iterator<T> Iter>
    bool write_length(der, bool, uint64_t len, Iter output) {
        return write_length(len, length_options::definite_required,
                            [&output](std::byte b){ *output = static_cast<T>(b); ++output; });
    }

    template <std::integral T, std::output_iterator<T> Iter>
    bool write_length(rules r, bool constructed, uint64_t len, Iter output) {
        switch (r) {
            case rules::cer: return write_length(cer{}, constructed, output);
            case rules::der: return write_length(der{}, constructed, output);
            default: return write_length(ber{}, constructed, output);
        }
    }

} /* namespace dabers */

#endif //DABERS_LENGTH_H
