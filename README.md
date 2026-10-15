# chip8emu

![video](https://aymeri.dev/projects/chip8emu_demo.gif)

A CHIP-8 Emulator made with C++20 (and CMake >= 4.3), with an SDL3 Frontend, and a complete set of ImGui tools to inspect and debug ROMs.

The emulator currently passes tests 1 to 5 from [Timendus CHIP-8 Test Suite](https://github.com/Timendus/chip8-test-suite).


## Features

- Inspect Emulator State (show register values, keyboard, stack pointer and sound timer). Pause and change Cycles Per Tick Execution.
- Hexadecimal Memory Inspector (Show internal memory from 0x000 to 0x200 and 0x200 to 0xFFF for the rest)
- Disassembler: List all the instructions of the current ROM, mapped to the instructions from Cowgod's Guide.
- Display Settings: Change background/foreground color (and randomize colors)
- Keypad support: Use the same keybinds as [Tobias V. Langhoff’s Guide](https://tobiasvl.github.io/blog/write-a-chip-8-emulator/#keypad). Use `ZQSD` (Azerty) to move in most of the games.
- Browse games/tests roms in ./roms. Support for drag-and-drop.

You can toggle on/off the imgui debugger using the `F1` key.

### Roadmap

- Audio support
- Remove all dynamic allocations
- STM32 support
- XO-CHIP support
- Use CMake FetchContent for Doctest.

## Install

Prerequisites: a C++20 compiler and CMake >= 4.3.

- Clone the repository
- Build the project:

```bash
cmake -B cmake-build-debug # -DENABLE_SANITIZERS=OFF to disable sanitizers
cmake --build cmake-build-debug
```

- Run:

```bash
cd ./cmake-build-debug # (from the repository root)
./chip8emu
```

- Run the tests:

```bash
cd ./cmake-build-debug
./chip8tests
```

## Credits

Thanks to:
- Tobias V. Langhoff’s Guide (https://tobiasvl.github.io/blog/write-a-chip-8-emulator)
- CHIP-8 Test Suite (https://github.com/Timendus/chip8-test-suite)
- CHIP-8 Opcodes (https://chip8.gulrak.net/)
- Cowgod's CHIP-8 Technical Reference (http://devernay.free.fr/hacks/chip8/C8TECH10.HTM)
- Tetris ROM by Fran Dachille (https://www.hpcalc.org/details/858)
- Snake ROM by Timo Triisa (https://github.com/JohnEarnest/chip8Archive/tree/master/src/snake)

Note: ROMs included in the repository keep their own licenses. Please check their corresponding license. The Timendus Test Suite uses the GPL-3.0 license.