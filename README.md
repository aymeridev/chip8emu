# chip8emu

[![CI Badge](https://github.com/aymeridev/chip8emu/actions/workflows/ci.yml/badge.svg)](https://github.com/aymeridev/chip8emu/actions)
[![codecov](https://codecov.io/gh/aymeridev/chip8emu/graph/badge.svg?token=72HVBG11O2)](https://codecov.io/gh/aymeridev/chip8emu)
[![License](https://img.shields.io/badge/License-MIT-blue)](https://github.com/aymeridev/chip8emu/blob/main/LICENSE)

![video](https://aymeri.dev/projects/chip8emu_demo.gif)

> Emulator running with the SDL3 frontend using [Timo Triisa's Snake](https://github.com/JohnEarnest/chip8Archive/blob/master/src/snake) game.


A CHIP-8 Emulator made with C++20 (and CMake >= 3.31), with an SDL3 Frontend, and a complete set of Dear ImGui tools to inspect and debug ROMs.

The emulator currently passes tests 1 to 5 from [Timendus CHIP-8 Test Suite](https://github.com/Timendus/chip8-test-suite).


## Features

- Inspect Emulator State (show register values, keyboard, stack pointer and sound timer). Pause and change Cycles Per Tick Execution.
- Hexadecimal Memory Inspector (Show internal memory from 0x000 to 0x200 and 0x200 to 0xFFF for the rest)
- Disassembler: List all the instructions of the current ROM, mapped to the instructions from Cowgod's Guide.
- Display Settings: Change background/foreground color (and randomize colors)
- Keypad support: Use the same keybinds as [Tobias V. Langhoff’s Guide](https://tobiasvl.github.io/blog/write-a-chip-8-emulator/#keypad). Use `ZQSD` (AZERTY) to move in most of the games.
- Browse games/tests roms in `./roms`. Support for drag-and-drop.

You can toggle on/off the Dear ImGui debugger using the `F1` key.

## Architecture

The project uses 3 targets:

- `chip8core`: The core of the emulator. No **dynamic allocation**, no exceptions and no RTTI (to be portable for [STM32](#roadmap)).
- `chip8tools`: The disassembler.
- `chip8emu`: Includes the SDL3 frontend and the ImGui tools (disassembler, memory viewer, ...)

## Roadmap

- Audio support
- STM32 support
- XO-CHIP support

## Install

Prerequisites: a C++20 compiler and CMake >= 3.31.

- Clone the repository
```bash
git clone https://github.com/aymeridev/chip8emu.git
cd chip8emu
```

- Run the project:

```bash
cmake --workflow --preset debug
./build/debug/chip8emu
```

## Tests

Tests are made with doctest and run with ASan/UBSan.

You can check the [Codecov Coverage here](https://app.codecov.io/gh/aymeridev/chip8emu/tree/main/src).
Coverage only covers the emulation core (CMake targets `chip8core` and `chip8tools`).

- To run the tests (via CTest):

```bash
ctest --preset debug
```

### Coverage

You need `gcovr` to generate coverage. Run `brew install gcovr` (macOS) or `pipx install gcovr` (Linux) to install it.

- To generate coverage:

```bash
cmake --workflow --preset coverage

mkdir -p build/coverage/html

gcovr \
  --root . \
  --filter src/ \
  --gcov-executable "xcrun llvm-cov gcov" \
  --html-details build/coverage/html/index.html \
  --print-summary build/coverage
```

> Important: If you're not on macOS, remove the `--gcov-executable "xcrun llvm-cov gcov"` argument to make it work with Linux and GCC.

You can then open `build/coverage/html/index.html` to check the report.

## Credits

Thanks to:
- Tobias V. Langhoff’s Guide (https://tobiasvl.github.io/blog/write-a-chip-8-emulator)
- CHIP-8 Test Suite (https://github.com/Timendus/chip8-test-suite)
- CHIP-8 Opcodes (https://chip8.gulrak.net/)
- Cowgod's CHIP-8 Technical Reference (http://devernay.free.fr/hacks/chip8/C8TECH10.HTM)
- Tetris ROM by Fran Dachille (https://www.hpcalc.org/details/858)
- Snake ROM by Timo Triisa (https://github.com/JohnEarnest/chip8Archive/tree/master/src/snake)

Note: ROMs included in the repository keep their own licenses. Please check their corresponding license. The Timendus Test Suite uses the GPL-3.0 license.