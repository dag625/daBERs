//
// Created by Daniel Garcia on 5/22/2022.
//

#include "buffer_check.h"
#include "exception.h"

namespace dabers {

    void check_buffer(const std::byte* const begin, const std::byte* const end, const std::size_t min_size, const std::string_view context) {
        if (begin == nullptr) {
            throw_ex("Null beginning to buffer for '{}'.", context);
        }
        else if (end == nullptr) {
            throw_ex("Null end to buffer for '{}'.", context);
        }
        else if (auto sz = std::distance(begin, end); sz < min_size) {
            throw_ex("Buffer too small for '{}'.  Expected {} < {}.", context, sz, min_size);
        }
    }

    const std::byte* consume_buffer(const std::byte*& begin, const std::byte* const end, const std::size_t size, const std::string_view context) {
        check_buffer(begin, end, size, context);
        auto* retval = begin;
        begin += size;
        return retval;
    }

} /* namespace dabers */