# flash-eprom-programmer
This is complete solution to program FLASH EPROM memory devices.

Project includes firmware and PC software for controlling it through USB port.

Currently tested with these models:
Intel 28F256

## Hardware Instructions

Connect USB and 12VDC regulated power supply to programmer. RUN-LED should blink when firmware works. 12 volt supply is only needed for write operations.

When write or erase operation is performed PROG-LED should lit.

Always when there is communication between programmer and computer two USB-LED's should blink.

## Software Instructions

Download PIC 16F1454 USB to Serial Adapter Firmware precompiled HEX file:
https://github.com/jgeisler0303/PIC16F1454_USB2Serial

Flash PIC program with PIC programmer. Connect it to board. Flash AVR with Arduino bootloader. Connect it to board too. Then connect USB and program firmware using Arduino IDE.

Write text file with all necessary binary data in ascii format. File name should be binData.txt. Alternatively you can skip this phase completely by writing data in hexadecimal format in to file hexData.txt.

File content is filtered from any non-binary and non-hexadecimal characters. This includes space and linebreaks.

Example binData.txt content:
01010101 01010101 01010101

Example hexData.txt content:
00 11 22 0a
aa 12 34 

Before first use of python scripts, input these commands once:
chmod +x bin2hex.py
chmod +x rom-loader.py

Then run scripts on this way:
./bin2hex.py
./rom-loader.py

## License

You are free to build and use this project for non-commercial use. For commercial use or selling it in kit form or ready built version you can buy commercial license. Contact me before publish a copy of this project in any media. I don't give any varranties to this project.
