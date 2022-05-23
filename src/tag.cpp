//
// Created by Daniel Garcia on 5/22/2022.
//

#include "dabers/tag.h"
#include "buffer_check.h"
#include "exception.h"

#include <doctest/doctest.h>
#include <fmt/core.h>
#include <fmt/ostream.h>

#include <algorithm>
#include <vector>

namespace dabers {

    namespace {

        constexpr int MAX_TAG_NUM_LENGTH = 9;

        tag test_parse_tag(const std::vector<unsigned int>& v) {
            std::vector<std::byte> b;
            b.reserve(v.size());
            std::transform(v.begin(), v.end(), std::back_inserter(b),
                           [](unsigned int a){ return static_cast<std::byte>(a); });
            const std::byte* beg = b.data();
            return parse_tag(beg, b.data() + b.size());
        }

    }

    std::strong_ordering operator<=>(tag_class_type a, tag_class_type b) noexcept {
        //This works because the canonical ordering is the numeric ordering.
        return static_cast<uint8_t>(a) <=> static_cast<uint8_t>(b);
    }

    std::ostream& operator<<(std::ostream& os, tag_class_type t) {
        switch (t) {
            case tag_class_type::universal: os << "Universal"; break;
            case tag_class_type::context_specific: os << "Context Specific"; break;
            case tag_class_type::application: os << "Application"; break;
            case tag_class_type::private_class: os << "Private"; break;
            default: os << "Unknown Tag Class (" << static_cast<int>(t) << ')'; break;
        }
        return os;
    }

    std::strong_ordering operator<=>(const tag& a, const tag& b) noexcept {
        auto retval = a.tag_class <=> b.tag_class;
        if (retval == std::strong_ordering::equal) {
            retval = a.tag_number <=> b.tag_number;
            if (retval == std::strong_ordering::equal) {
                //This part is unspecified in terms of canonical ordering, so we don't really care.
                retval = a.constructed <=> b.constructed;
            }
        }
        return retval;
    }

    bool operator==(const tag& a, const tag& b) noexcept {
        return (a <=> b) == std::strong_ordering::equal;
    }

    std::ostream& operator<<(std::ostream& os, const tag& t) {
        fmt::print(os, "[{0}; {1}; {2} ({2:#x})]",
                   t.tag_class,
                   t.constructed ? "Constructed" : "Primitive",
                   t.tag_number);
        return os;
    }

    tag parse_tag(const std::byte*& begin, const std::byte* const end) {
        auto* first = consume_buffer(begin, end, 1);
        auto cl = static_cast<tag_class_type>(*first & std::byte{0xc0u});
        auto is_cons = (*first & std::byte{0x20u}) != std::byte{0};
        auto num = static_cast<uint64_t>(*first & std::byte{0x1fu});
        if (num == 0x1fu) {
            uint64_t num_bits = 0;
            bool more = true, first_num = true;
            int count = 0;
            while (more) {
                auto* next = consume_buffer(begin, end, 1);
                more = (*next & std::byte{0x80u}) != std::byte{0};
                if (more && first_num && (*next & std::byte{0x7fu}) == std::byte{0}) {
                    throw_ex("The first octet of an extended tag number "
                             "cannot have 0 for the number bits.");
                }
                num_bits <<= 7;
                num_bits |= static_cast<uint8_t>(*next & std::byte{0x7fu});
                ++count;
                first_num = false;
            }
            if (count > MAX_TAG_NUM_LENGTH) {
                throw_ex("The length of the tag ({}) is more than the "
                         "maximum supported by this library ({}).",
                         count, MAX_TAG_NUM_LENGTH);
            }
            num = num_bits;
        }
        return {cl, is_cons, num};
    }

    TEST_CASE("parse_tag success") {
        for (uint8_t i = 0u; i < 31; ++i) {
            CHECK((test_parse_tag({0x00u + i}) == tag{tag_class_type::universal, false, i}));
            CHECK((test_parse_tag({0x40u + i}) == tag{tag_class_type::application, false, i}));
            CHECK((test_parse_tag({0x80u + i}) == tag{tag_class_type::context_specific, false, i}));
            CHECK((test_parse_tag({0xc0u + i}) == tag{tag_class_type::private_class, false, i}));

            CHECK((test_parse_tag({0x20u + i}) == tag{tag_class_type::universal, true, i}));
            CHECK((test_parse_tag({0x60u + i}) == tag{tag_class_type::application, true, i}));
            CHECK((test_parse_tag({0xa0u + i}) == tag{tag_class_type::context_specific, true, i}));
            CHECK((test_parse_tag({0xe0u + i}) == tag{tag_class_type::private_class, true, i}));
        }
        CHECK((test_parse_tag({0x1fu, 0x01u}) == tag{tag_class_type::universal, false, 1}));
        CHECK((test_parse_tag({0x7fu, 0x1fu}) == tag{tag_class_type::application, true, 31}));
        CHECK((test_parse_tag({0x9fu, 0x7fu}) == tag{tag_class_type::context_specific, false, 127}));

        CHECK((test_parse_tag({0x1fu, 0x81u, 0x00u}) == tag{tag_class_type::universal, false, 128}));
        CHECK((test_parse_tag({0x1fu, 0x81u, 0x01u}) == tag{tag_class_type::universal, false, 129}));

        CHECK((test_parse_tag({0x1fu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0x7fu}) == tag{tag_class_type::universal, false, 0x7fffffffffffffffu}));

        //Check with buffer with extra data.
        CHECK((test_parse_tag({0x1fu, 0x81u, 0x00u, 0x8au, 0x0bu, 0x8cu, 0x0du}) == tag{tag_class_type::universal, false, 128}));

        std::vector<std::byte> buf;
        buf.push_back(std::byte{0x9fu});
        buf.push_back(std::byte{0x93u});
        buf.push_back(std::byte{0x14u});
        buf.push_back(std::byte{0x8au});
        buf.push_back(std::byte{0x0bu});
        buf.push_back(std::byte{0x8cu});
        buf.push_back(std::byte{0x0du});
        buf.push_back(std::byte{0x8eu});
        buf.push_back(std::byte{0x0fu});
        const std::byte *beg = buf.data(), *end = buf.data() + buf.size();
        auto t = parse_tag(beg, end);
        CHECK_EQ(t, tag{tag_class_type::context_specific, false, 0b001'0011'001'0100});
        CHECK((*beg == std::byte{0x8au}));
    }

    TEST_CASE("parse_tag failures") {
        //Empty buffer:
        CHECK_THROWS_AS(test_parse_tag({}), exception);
        //Long tag number implied, but not provided:
        CHECK_THROWS_AS(test_parse_tag({0x1fu}), exception);
        //Long tag number with no ending byte:
        CHECK_THROWS_AS(test_parse_tag({0x1fu, 0x81u, 0x81u}), exception);
        //Too long for library to support:
        CHECK_THROWS_AS(test_parse_tag({0x1fu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0x7fu}), exception);
        //Most significant byte has only zeroes:
        CHECK_THROWS_AS(test_parse_tag({0x1fu, 0x80u, 0xffu, 0xffu, 0xffu, 0x7fu}), exception);
    }

    void write_tag(const tag& t, const std::function<void(std::byte)>& output) {
        auto first = std::byte{static_cast<uint8_t>(t.tag_class)};
        first |= t.constructed ? std::byte{0x20u} : std::byte{0};
        if (t.tag_number > 30) {
            constexpr int NSIZE = 7;
            first |= std::byte{0x1fu};
            output(first);
            uint64_t mask = 0x7f00000000000000u;
            int num = 8;
            bool found = false;
            while (mask != 0) {
                if (found || t.tag_number & mask) {
                    uint8_t val = num * 0x80u;
                    val |= (t.tag_number & mask) >> (num * NSIZE);
                    output(std::byte{val});
                    found = true;
                }
                mask >>= NSIZE;
                --num;
            }
        }
        else {
            first |= std::byte{static_cast<uint8_t>(t.tag_number)};
            output(first);
        }
    }

} /* namespace dabers */