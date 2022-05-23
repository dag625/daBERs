//
// Created by Daniel Garcia on 5/22/2022.
//

#include "dabers/length.h"
#include "exception.h"
#include "buffer_check.h"

#include <array>

namespace dabers {

    std::optional<uint64_t> parse_length(const length_options opts, const std::byte*& begin, const std::byte* const end) {
        auto* first = consume_buffer(begin, end, 1);
        bool long_form = (*first & std::byte{0x80u}) != std::byte{0};
        if (long_form) {
            auto num_long_bytes = to_integer<uint32_t>(*first & std::byte{0x7fu});
            if (num_long_bytes == 0) {
                if (opts == length_options::definite_required) {
                    throw_ex("Indefinite length form found, but definite form was required.");
                }
                return std::nullopt;
            }
            else {
                if (opts == length_options::indefinite_required) {
                    throw_ex("Definite length form found, but indefinite form was required.");
                }
                else if (num_long_bytes > sizeof(uint64_t)) {
                    throw_ex("The long form length ({}) is more than the "
                             "maximum supported by this library ({}).",
                             num_long_bytes, sizeof(uint64_t));
                }
                auto* buf = consume_buffer(begin, end, num_long_bytes);
                std::array<std::byte, sizeof(uint64_t)> arr{};
                std::memcpy(arr.data() + sizeof(uint64_t) - num_long_bytes, buf, num_long_bytes);
                uint64_t retval = 0;
                std::memcpy(&retval, arr.data(), arr.size());
                return retval;
            }
        }
        else {
            if (opts == length_options::indefinite_required) {
                throw_ex("Short form length field is invalid when the indefinite length form is required.");
            }
            return static_cast<uint64_t>(*first);
        }
    }

    bool write_length(const uint64_t len, const length_options opts, const std::function<void(std::byte)>& output) {
        if (opts == length_options::indefinite_required) {
            output(std::byte{0x80u});
            return true;
        }
        else if (opts == length_options::definite_required) {
            if (len < 128) {
                output(std::byte{static_cast<uint8_t>(len)});
            }
            else {
                uint8_t num_len = 8;
                uint64_t mask = 0xff00000000000000u;
                bool started = false;
                while (num_len > 0) {
                    if (auto v = mask & len; v) {
                        if (!started) {
                            output(std::byte{0x80u} | std::byte{num_len});
                        }
                        started = true;
                        --num_len;
                        output(std::byte{static_cast<uint8_t>(v >> (num_len * CHAR_BIT))});
                    }
                }
            }
            return false;
        }
        else {
            throw_ex("Invalid length option ({}) for writing the length ({}).", static_cast<int>(opts), len);
        }
    }

} /* namespace dabers */