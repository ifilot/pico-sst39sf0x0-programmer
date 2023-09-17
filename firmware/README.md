[![build](https://github.com/ifilot/pico-sst39sf0x0-programmer/actions/workflows/build.yml/badge.svg)](https://github.com/ifilot/pico-sst39sf0x0-programmer/actions/workflows/build.yml)
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)

# PICO Firmware

This folder contains the firmware for the PICO.

## Compilation

```
mkdir build-firmware
cd build-firmware
cmake ../firmware
make -j9
```

Next, copy the `sst39sf040.uf2` file from the `build` folder to your PICO.

## Toolchain

First install the following packages

```bash
sudo apt update
sudo apt install -y cmake gcc-arm-none-eabi libnewlib-arm-none-eabi \
build-essential libstdc++-arm-none-eabi-newlib
```

Next, install the toolchain

```bash
sudo git clone https://github.com/raspberrypi/pico-sdk.git /opt/pico-sdk
sudo git -C /opt/pico-sdk submodule update --init
echo 'export PICO_SDK_PATH=/opt/pico-sdk' | sudo tee -a /etc/profile.d/pico-sdk.sh
source /etc/profile
```

## UART commands

The PICO exposes a COM port for interdevice communication. All commands have a fixed
width of 8 characters, which is echoed back. Depending on the command, either the
PICO expects to receive another chunk of data or returns more output. The list
of commands are given below.

* `READINFO`: Reads the board ID. Returns 16 bytes of data.
* `RDBKXXXX`: Reads a block of data (256 bytes) at hex position `XXXX00`. Returns
  256 bytes of data.
* `RDSECTXX`: Reads a sector of data (4kb) at hex position `XX000`. Returns
  4kb bytes of data.
* `RDBANKXX`: Reads a bank of data (16kb) where `XX` corresponds to the
  bank id.
* `RP2KCRXX`: Reads a block of data (0x1000 bytes) at hex position `XX000`. Returns
  0x1000 bytes of data. This function is used to interface with the P2000T cartridge,
  do not use this function when reading from a regular ROM chip.
* `DEVIDSST`: Attempts to read the device ID of the SST39SF0x0 chip. Returns
  2 bytes of data.
* `WRBKXXXX`: Write a block of data at hex position `XXXX00`.
  Expects 256 bytes of data after the command echo. Returns 1 byte checksum.
* `WRSECTXX`: Writes a sector of data (4kb) at hex position `XX000`. Returns
  a CRC16 XMODEM checksum (2 bytes).
* `ESSTXXXX`: Erases a block of data (writing `0xFF`) at hex position `XXXX000`.
  Returns number of wait cycles as 16 bit signed integer.
* `TESTTEST`: Simple test routine. Reads the first 32 bytes on the chip. Returns
  32 bytes of data.
