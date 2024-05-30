# Flash-Eprom-Programmer
This is complete solution to program FLASH EEPROM memory devices.

## State: NOT-WORKING at this moment
Working on it...

Project includes firmware and PC software for controlling it through USB port.

Currently tested with these models:
Intel 28F256
Intel 28F020

![EPROM-Programmer-PCB](https://raw.githubusercontent.com/Juvar1/flash-eprom-programmer/main/rom-prog-pcb01.jpg)

## Hardware Instructions

Connect USB and 12VDC regulated power supply to programmer. IDLE LED should blink once at a second when firmware works. 12 volt supply is only needed for write operations.

When write or erase operation is performed PROG LED should lit.

Always when there is communication between programmer and computer two USB LED's should blink.

## Software Instructions

Download PIC 16F1454 USB to Serial Adapter Firmware precompiled HEX file:
https://github.com/jgeisler0303/PIC16F1454_USB2Serial

Flash PIC program with PIC programmer. Connect it to board. Flash AVR with Arduino bootloader. Connect it to board too. Then connect USB and program firmware using Arduino IDE.

Write text file with all necessary binary data in ascii format. File name should be binData.txt. Alternatively you can skip this phase completely by writing data in hexadecimal format in to file hexData.txt.

You can convert binary files to supported format in Linux with following command: hexdump -e '16/1 "%02x""\n"' -v biosv21.rom > hexData.txt

File content is filtered from any non-binary and non-hexadecimal characters. This includes spaces and linebreaks.

Example binData.txt content:
<br>01010101 01010101 01010101

Example hexData.txt content:
<br>00 11 22 0a
<br>aa 12 34 

Before first use of python scripts, input these commands once:
<br>chmod +x bin2hex.py
<br>chmod +x rom-loader.py

Then run scripts on this way:
<br>./bin2hex.py
<br>./rom-loader.py

### Rom-Loader.py usage
./rom-loader.py [--port PORT] [--size SIZE] [--file FILE] {program,fillzeros,erase,read,write,ident,test,reset}

#### port
Defines programmer serial port.

#### size
Defines EEPROM memory size in bytes.

#### file
Defines file used in read and write operations.

#### program
Fills memory with zeros and then erases it to 1's and finally writes defined data file into memory with verify.

#### fillzeros
Fills memory with zeros.

#### erase
Erases EEPROM if possible. (Should be first filled with zeroes.)

#### read
Read EEPROM content into file.

#### write
Write file content into EEPROM.

#### ident
Read EEPROM identification codes.

#### test
Simple read/write test for this software development purposes.

#### reset
Resets EEPROM and programmer firmware.

## License

You are free to build and use this project for non-commercial use. For commercial use or selling it in kit form or ready built version you can buy commercial license. Contact me before publish a copy of this project in any media. I don't give any varranties to this project.
