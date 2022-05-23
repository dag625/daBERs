//
// Created by Daniel Garcia on 5/22/2022.
//

#ifndef DABERS_BUFFER_CHECK_H
#define DABERS_BUFFER_CHECK_H

#include <cstddef>
#include <string_view>

namespace dabers {

    void check_buffer(const std::byte* begin, const std::byte* end, std::size_t min_size, std::string_view context = "");
    const std::byte* consume_buffer(const std::byte*& begin, const std::byte* end, std::size_t size, std::string_view context = "");

} /* namespace dabers */

#endif //DABERS_BUFFER_CHECK_H
