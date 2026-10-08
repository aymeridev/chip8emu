#include <array>
#include <cstdint>
#include <string>
#include <string_view>

#include "../external/doctest.h"
#include "../src/Disassembler.hpp"

namespace {
    struct Case {
        std::uint16_t opcode;
        std::string_view expected;
    };

    // http://devernay.free.fr/hacks/chip8/C8TECH10.HTM
    constexpr std::array exact_cases = {
        Case{0x00E0, "CLS"},
        Case{0x00EE, "RET"},
        Case{0x1228, "JP 0x228"},
        Case{0x2300, "CALL 0x300"},
        Case{0x3A2F, "SE VA, 0x2F"},
        Case{0x4A2F, "SNE VA, 0x2F"},
        Case{0x5120, "SE V1, V2"},
        Case{0x6A2F, "LD VA, 0x2F"},
        Case{0x7A01, "ADD VA, 0x01"},
        Case{0x8120, "LD V1, V2"},
        Case{0x8121, "OR V1, V2"},
        Case{0x8122, "AND V1, V2"},
        Case{0x8123, "XOR V1, V2"},
        Case{0x8124, "ADD V1, V2"},
        Case{0x8125, "SUB V1, V2"},
        Case{0x8126, "SHR V1 {, V2}"},
        Case{0x8127, "SUBN V1, V2"},
        Case{0x812E, "SHL V1 {, V2}"},
        Case{0x9120, "SNE V1, V2"},
        Case{0xA228, "LD I, 0x228"},
        Case{0xB228, "JP V0, 0x228"},
        Case{0xC1FF, "RND V1, 0xFF"},
        Case{0xD125, "DRW V1, V2, 0x5"},
        Case{0xE19E, "SKP V1"},
        Case{0xE1A1, "SKNP V1"},
        Case{0xF107, "LD V1, DT"},
        Case{0xF10A, "LD V1, K"},
        Case{0xF115, "LD DT, V1"},
        Case{0xF118, "LD ST, V1"},
        Case{0xF11E, "ADD I, V1"},
        Case{0xF129, "LD F, V1"},
        Case{0xF133, "LD B, V1"},
        Case{0xF155, "LD [I], V1"},
        Case{0xF165, "LD V1, [I]"},
    };

    constexpr std::array<std::uint16_t, 12> unknown_cases = {
        0x0123, // 0nnn (SYS) is not supported
        0x01E0, // only 00E0 is CLS
        0x01EE, // only 00EE is RET
        0x5121, // 5xy0 requires n == 0
        0x8128, // 8xy8..8xyD do not exist
        0x812F,
        0x9121, // 9xy0 requires n == 0
        0xE100, // Ex9E / ExA1 only
        0xE00E, // used to be matched on n alone
        0xE19F,
        0xF100, // Fx00 does not exist
        0xF1FF,
    };
}

TEST_SUITE_BEGIN("disassembler");

TEST_CASE("every opcode family gives the expected mnemonic") {
    for (const auto& [opcode, expected] : exact_cases) {
        CAPTURE(opcode);
        CHECK(disassemble(opcode) == expected);
    }
}

TEST_CASE("invalid opcodes are reported as unknown") {
    for (const auto opcode : unknown_cases) {
        CAPTURE(opcode);
        CHECK(disassemble(opcode).starts_with("unknown"));
    }
}

TEST_CASE("operands use the right nibbles") {
    CHECK(disassemble(0x8AB4) == "ADD VA, VB");
    CHECK(disassemble(0x8BA4) == "ADD VB, VA");
    CHECK(disassemble(0x6000) == "LD V0, 0x00");
    CHECK(disassemble(0x6FFF) == "LD VF, 0xFF");
    CHECK(disassemble(0x1FFF) == "JP 0xFFF");
    CHECK(disassemble(0x1000) == "JP 0x000");
}

TEST_CASE("every possible opcode can be disassembled") {
    for (std::uint32_t opcode = 0; opcode <= 0xFFFF; ++opcode) {
        CAPTURE(opcode);
        CHECK_FALSE(disassemble(static_cast<std::uint16_t>(opcode)).empty());
    }
}

TEST_SUITE_END();