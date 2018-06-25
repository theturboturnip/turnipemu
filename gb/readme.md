# How to use the GameBoy emulator

1. Download a copy of the GameBoy BIOS to ./data/bios.gb. It should be exactly 256 bytes long.
2. Run the following commands to make sure it passes all of the tests.
~ make clean
~ make test-collated
3. Download a GameBoy rom, and run it like so
~ ./run ./data/bios.gb <PATH_TO_ROM>
(This only supports a very limited amount of ROMs. It's been tested on Tetris and Pokemon Red, and it doesn't support many cartridge types. Also, it doesn't support CGB.)

Controls:
A - A
S or B - B
Enter - Start
Right Shift - Select
Arrow Keys - D-Pad

# Notes
The data/ directory contains the Blargg CPU test ROMs, so that the functionality of the CPU can be tested on compile.