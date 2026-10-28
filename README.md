# chip8emu

[![CI Badge](https://github.com/aymeridev/chip8emu/actions/workflows/ci.yml/badge.svg)](https://github.com/aymeridev/chip8emu/actions)
[![codecov](https://codecov.io/gh/aymeridev/chip8emu/graph/badge.svg?token=72HVBG11O2)](https://codecov.io/gh/aymeridev/chip8emu)
[![License](https://img.shields.io/badge/License-MIT-blue)](https://github.com/aymeridev/chip8emu/blob/main/LICENSE)

![video](https://projects.aymeri.dev/chip8emu_demo.gif)

> Emulator running with the SDL3 frontend using [Timo Triisa's Snake](https://github.com/JohnEarnest/chip8Archive/blob/master/src/snake) game.


A CHIP-8 Emulator made with C++20 (and CMake >= 3.31) with no dynamic allocations for the core, an [STM32](#stm32) port and for the desktop an SDL3 Frontend with a complete set of Dear ImGui tools to inspect and debug ROMs.

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

The project uses 4 targets:

- `chip8core`: The core of the emulator. No **dynamic allocation**, no exceptions and no RTTI (to be portable for [STM32](#stm32)).
- `chip8tools`: The disassembler.
- `chip8emu`: Includes the SDL3 frontend and the ImGui tools (disassembler, memory viewer, ...)
- `nucleo_f446re`: the firmware for the NUCLEO-F446RE board, which links `chip8core`

## Roadmap

- Audio support (desktop)
- XO-CHIP support
- STM32: Use DMA for screen rendering
- STM32: Menu to select ROMs at runtime


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

## STM32

https://github.com/user-attachments/assets/2936e60b-3037-4e15-b26c-808adb45396b

The emulator is also available on STM32!

In this version, some changes have been made:

- Instead of loading roms via filesystem, the ROM is in the flash. (through a `constexpr std::array`)
- The screen (64x32) is converted and sent to the SSD1306 (via x2 zoom) only when `screen_dirty` is true.
- An I2C bus recovery runs at boot up if a reset interrupted a transaction.

### Hardware

To make the project you need:

- **ST NUCLEO-F446RE**
- SSD1306 0.96" OLED Screen (128x64)
- 4 push buttons
- Murata PKM22EPP-40 piezo sounder (driven via an optional 6.8 kΩ series resistor)

For the screen, the program uses the address 0x3C. Since the CHIP-8 screen is 64x32, the display is scaled by 2 for the OLED screen.

For the wiring:

#### Screen/Buzzer

- `OLED_SCL`: pin PB8 (header D15)
- `OLED_SDA`: pin PB9 (header D14)
- `OLED_VDD`/`GND`: 3V3/GND (header CN6)
- `BUZZER`: PC7 (header D9)

#### Inputs

> By default the keys are mapped to Snake. If you want to change them to a different game, you can edit `keysMap` in `stm32/nucleo_f446re/App/app.cpp`

- `BTN_UP`: pin PA10 (header D2) **(key code 5)**
- `BTN_RIGHT`: pin PB5 (header D4) **(key code 9)**
- `BTN_DOWN`: pin PB4 (header D5) **(key code 8)**
- `BTN_LEFT`: pin PB10 (header D6) **(key code 7)**


### Build/Flash

Prerequisites: arm-none-eabi-gcc, Ninja, OpenOCD

On macOS using Homebrew, you can run `brew install --cask gcc-arm-embedded` and `brew install ninja open-ocd` to install the toolchain.

```bash
cmake --preset stm32
cmake --build --preset stm32
openocd -f board/st_nucleo_f4.cfg -c "program build/stm32/stm32/nucleo_f446re/nucleo_f446re.elf verify reset exit"
```

### Limits

Currently, screen is sent via blocking I2C, which means games that redraw at each frame will run a little bit below 60 Hz. (see Roadmap)

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
- STM32F446RE Documentation (https://www.st.com/en/evaluation-tools/nucleo-f446re.html)
- SSD1306 Data Sheet (https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf)

Note: ROMs included in the repository keep their own licenses. Please check their corresponding license. The Timendus Test Suite uses the GPL-3.0 license.
