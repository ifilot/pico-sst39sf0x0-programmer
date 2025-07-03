#!/bin/sh

# clean any previous data
rm -rvf data

# create folder
mkdir -v data

# copy executable
cp -v ../build/release/pico-sst39sf0x0-programmer.exe data/

# run windeploy
/c/Qt/6.9.1/mingw_64/bin/windeployqt.exe data/pico-sst39sf0x0-programmer.exe --release --force

# copy icon
cp -v ../../gui/assets/icon/eeprom_icon.ico data/

# build installer script
/C/Users/iawfi/anaconda3/python.exe package.py

# create installer
/c/Program\ Files\ \(x86\)/Inno\ Setup\ 6/ISCC.exe setup.iss