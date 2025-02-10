# achip8emu

In order to deep-dive into emulators, I decided to start with the so-called simplest emulator project, therefore a Chip-8 Emulator.

## Dependencies:
- SDL2

On Ubuntu machines, install with:
```bash
sudo apt install libsdl2-dev
```

On other systems, you're on your own for now.

## Building
From the project root directory, create a build folder, run cmake and build:
```bash
mdkir build
cd build
cmake ..
make -j
```

## Testing
There are two rom in the `support` directory that can be used to test:
- ibm_logo.ch8 --> prints the IBM logo on the screen (good for debugging screen drawing issues)
- test_opcode.ch8 --> tests all opcodes and print either OK or NO.

Simply run the binary built in the build folder and pass a rom as the argument. From the root folder:
```bash
build/achip8emu support/test_opcode.ch8
```
