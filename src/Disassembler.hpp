#pragma once
#include <string>
#include <format>
#include <cstdint>


inline std::string disassemble(const std::uint16_t instruction) {
    std::string txt;

    const std::uint16_t nnn = instruction & 0x0FFF;
    const std::uint8_t nn = instruction & 0x00FF;
    const std::uint8_t n = instruction & 0x000F;
    const std::uint8_t x = (instruction >> 8) & 0x0F;
    const std::uint8_t y = (instruction >> 4) & 0x0F;

    auto unknown = [&txt, instruction] {
        txt = std::format("unknown: {:X}", instruction);
    };

    switch (instruction >> 12) {
        case 0x0:
            if (nn == 0xE0) txt = "CLS";
            else if (nn == 0xEE) txt = "RET";
            else unknown();
            break;
        case 0x1: txt = std::format("JP {:X}", nnn); break;
        case 0x2: txt = std::format("CALL {:X}", nnn); break;
        case 0x3: txt = std::format("SE V{:X}, %{:X}", x, nn); break;
        case 0x4: txt = std::format("SNE V{:X}, {:X}", x, nn); break;
        case 0x5: txt = std::format("SE V{:X}, V{:X}", x, y); break;
        case 0x6: txt = std::format("LD V{:X}, {:X}", x, nn); break;
        case 0x7: txt = std::format("ADD V{:X}, {:X}", x, nn); break;
        case 0x8:
            switch (n) {
                case 0: txt = std::format("LD V{:X}, V{:X}", x, y); break;
                case 1: txt = std::format("OR V{:X}, V{:X}", x, y); break;
                case 2: txt = std::format("AND V{:X}, V{:X}", x, y); break;
                case 3: txt = std::format("XOR V{:X}, V{:X}", x, y); break;
                case 4: txt = std::format("ADD V{:X}, V{:X}", x, y); break;
                case 5: txt = std::format("SUB V{:X}, V{:X}", x, y); break;
                case 6: txt = std::format("SHR V{:X} {{, V{:X}}}", x, y); break;
                case 7: txt = std::format("SUBN V{:X}, V{:X}", x, y); break;
                case 0xE: txt = std::format("SHL V{:X} {{, V{:X}}}", x, y); break;
                default: unknown();
            }
            break;
        case 0x9:
            if (n == 0) txt = std::format("SNE V{:X} V{:X}", x, y);
            else unknown();
            break;
        case 0xA: txt = std::format("LD I, {:X}", nnn); break;
        case 0xB: txt = std::format("JP V0, {:X}", nnn); break;
        case 0xC: txt = std::format("RND V{:X}, nn", x, nn); break;
        case 0xD: txt = std::format("DRW {:X}, {:X}, {:X}", x, y, n); break;
        case 0xE:
            switch (n) {
                case 0xE: txt = std::format("SKP V{:X}", x); break;
                case 0x1: txt = std::format("SKNP V{:X}", x); break;
                default: unknown();
            }
            break;
        case 0xF:
            switch (nn) {
                case 0x07: txt = std::format("LD V{:X}, DT", x); break;
                case 0x15: txt = std::format("LD V{:X}, K", x); break;
                case 0x18: txt = std::format("LD DT, V{:X}", x); break;
                case 0x1E: txt = std::format("LD ST, V{:X}", x); break;
                case 0x29: txt = std::format("LD F, V{:X}", x); break;
                case 0x33: txt = std::format("LD B, V{:X}", x); break;
                case 0x55: txt = std::format("LD [I], V{:X}", x); break;
                case 0x65: txt = std::format("LD V{:X}, [I]", x); break;
                default: unknown();
            }
            break;
        default: unknown();
    }
    return txt;
}
