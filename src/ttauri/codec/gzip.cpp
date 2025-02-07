// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "gzip.hpp"
#include "inflate.hpp"
#include "../endian.hpp"
#include "../placement.hpp"

namespace tt {

struct GZIPMemberHeader {
    uint8_t ID1;
    uint8_t ID2;
    uint8_t CM;
    uint8_t FLG;
    little_uint32_buf_t MTIME;
    uint8_t XFL;
    uint8_t OS;
};

static bstring gzip_decompress_member(std::span<std::byte const> bytes, ssize_t &offset, ssize_t max_size)
{
    ttlet header = make_placement_ptr<GZIPMemberHeader>(bytes, offset);

    tt_parse_check(header->ID1 == 31, "GZIP Member header ID1 must be 31");
    tt_parse_check(header->ID2 == 139, "GZIP Member header ID2 must be 139");
    tt_parse_check(header->CM == 8, "GZIP Member header CM must be 8");
    tt_parse_check((header->FLG & 0xe0) == 0, "GZIP Member header FLG reserved bits must be 0");
    tt_parse_check(header->XFL == 2 || header->XFL == 4, "GZIP Member header XFL must be 2 or 4");
    [[maybe_unused]] ttlet FTEXT = static_cast<bool>(header->FLG & 1);
    ttlet FHCRC = static_cast<bool>(header->FLG & 2);
    ttlet FEXTRA = static_cast<bool>(header->FLG & 4);
    ttlet FNAME = static_cast<bool>(header->FLG & 8);
    ttlet FCOMMENT = static_cast<bool>(header->FLG & 16);

    if (FEXTRA) {
        ttlet XLEN = make_placement_ptr<little_uint16_buf_t>(bytes, offset);
        offset += XLEN->value();
    }

    if (FNAME) {
        std::byte c;
        do {
            tt_parse_check(offset < std::ssize(bytes), "GZIP Member header FNAME reading beyond end of buffer");
            c = bytes[offset++];
        } while (c != std::byte{0});
    }

    if (FCOMMENT) {
        std::byte c;
        do {
            tt_parse_check(offset < std::ssize(bytes), "GZIP Member header FCOMMENT reading beyond end of buffer");
            c = bytes[offset++];
        } while (c != std::byte{0});
    }

    if (FHCRC) {
        [[maybe_unused]] ttlet CRC16 = make_placement_ptr<little_uint16_buf_t>(bytes, offset);
    }

    auto r = inflate(bytes, offset, max_size);

    [[maybe_unused]] auto CRC32 = make_placement_ptr<little_uint32_buf_t>(bytes, offset);
    [[maybe_unused]] auto ISIZE = make_placement_ptr<little_uint32_buf_t>(bytes, offset);

    tt_parse_check(
        ISIZE->value() == (size(r) & 0xffffffff),
        "GZIP Member header ISIZE must be same as the lower 32 bits of the inflated size.");
    return r;
}

bstring gzip_decompress(std::span<std::byte const> bytes, ssize_t max_size)
{
    auto r = bstring{};

    ssize_t offset = 0;
    while (offset < std::ssize(bytes)) {
        auto member = gzip_decompress_member(bytes, offset, max_size);
        max_size -= std::ssize(member);
        r.append(member);
    }

    return r;
}

} // namespace tt