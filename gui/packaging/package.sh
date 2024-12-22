#!/bin/sh

# clean any previous data
rm -rvf packages/com.vendor.product/data

# create folder
mkdir -v packages/com.vendor.product/data

# copy executable
cp -v ../build/release/pico-sst39sf0x0-programmer.exe packages/com.vendor.product/data

# run windeploy
/c/Qt/6.7.0/mingw_64/bin/windeployqt.exe packages/com.vendor.product/data/pico-sst39sf0x0-programmer.exe --release --force

# copy icon
cp -v ../../gui/assets/icon/eeprom_icon.ico packages/com.vendor.product/data/eeprom_icon.ico

# run script generators
python3 package.py

# run packaging tool
/c/Qt/Tools/QtInstallerFramework/4.8/bin/binarycreator.exe -c config/config.xml -p packages pico-sst39sf0x0-programmer-installer-win64.exe