#!/usr/bin/env python3

# AVR FLASH ROM Loader
# 
#  Uploads data file to ROM memory using FLASH ROM Programmer.
#  - Data should be in ASCII HEX format.
#  - Default filename is hexData.txt.
#  - File content is filtered from linebreaks and other non-hex characters.
#  
#  All necessary files and circuit schematics are in this same repository.
#
# Copyright (C) 2021, Juha-Pekka Varjonen

import serial

# restart AVR and serial connection
s = serial.Serial('/dev/ttyACM0', 9600, timeout=1)

# wait until command execution is ready
def waitExec(value=''):
    d = ''
    while not d.endswith(value + '\r\n'):
        d += s.read(1).decode('latin_1')
    return d[:-2].encode('latin_1')

# send command to AVR
def sendComm(value):
    s.write(value + b'\r')
    return waitExec()

print('Waiting power-up cycle...')
# wait until AVR is ready
waitExec('ready')
print('Ready!')

def eraseChip():
    print('Starting to erase memory.')
    erase = sendComm(b'erase').decode()
    if erase == 'fail':
        print('Erase failed. Aborting.')
        exit()
    elif erase == 'pass':
        print('Erasing completed.')
    else:
        print('Unknown error. Aborting.')
        exit()

eraseChip()

print('Starting to write.')
# open file and write it's content to ROM and verify
with open('hexData.txt') as f:
    addr = 0
    tt = b''
    t = f.read(1)
    while t:
        if len(tt) == 2:
            sendComm(b'write ' + str(addr).encode())
            comp = '{:02x}'.format(ord((sendComm(chr(int(tt, 16)).encode('latin_1'))))).encode()
            if tt.lower() != comp:
                print('Verify error at address 0x{:02x}.'.format(addr))
                print('0x{} expected but 0x{} found.'.format(tt.decode(), comp.decode()))
                print('Aborting.')
                break
            addr += 1
            tt = b''
        if t.lower() in list('abcdef0123456789'):
            tt += t.encode()
        t = f.read(1)
    else:
        print('Writing of {} bytes complete.'.format(addr + 1))

# I don't need memory reading, but here is simple example:
# reading one byte from address 0x0a
#print('{:02x}'.format(ord(sendComm(b'read 10'))).encode())

# finally close connection
s.close()
