//
// Created by Daniel Garcia on 5/22/2022.
//

#ifndef DABERS_TAG_H
#define DABERS_TAG_H

#include <cstdint>
#include <functional>
#include <concepts>
#include <compare>
#include <ostream>

namespace dabers {

    enum class tag_class_type : uint8_t {
        universal = 0x00u,
        application = 0x40u,
        context_specific = 0x80u,
        private_class = 0xc0u
    };

    std::strong_ordering operator<=>(tag_class_type a, tag_class_type b) noexcept;
    std::ostream& operator<<(std::ostream& os, tag_class_type t);

    struct tag {
        tag_class_type tag_class = tag_class_type::application;
        bool constructed = false;
        uint64_t tag_number = 0u;
    };

    std::strong_ordering operator<=>(const tag& a, const tag& b) noexcept;
    bool operator==(const tag& a, const tag& b) noexcept;
    std::ostream& operator<<(std::ostream& os, const tag& t);

    tag parse_tag(const std::byte*& begin, const std::byte* end);

    /**
     * This writes a tag in a way which is compatible with BER, CER, and DER formats
     * so that we don't need separate functions for each.
     * @param t The tag to write.
     * @param output A function that can be called multiple times to write a single byte with each call.
     */
    void write_tag(const tag& t, const std::function<void(std::byte)>& output);

    template <std::output_iterator<std::byte> Iter>
    void write_tag(const tag& t, Iter output) {
        write_tag(t, [&output](std::byte b){ *output = b; ++output; });
    }

    template <std::integral T, std::output_iterator<T> Iter>
    void write_tag(const tag& t, Iter output) {
        write_tag(t, [&output](std::byte b){ *output = static_cast<T>(b); ++output; });
    }

} /* namespace dabers */

#endif //DABERS_TAG_H
