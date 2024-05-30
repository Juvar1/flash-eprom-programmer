#!/usr/bin/env python3

# AVR FLASH EEPROM Loader
# 
#  Uploads data file to ROM memory using FLASH EEPROM Programmer.
#  - Data should be in ASCII HEX format.
#  - File content is filtered from linebreaks and other non-hex characters.
#
# Copyright (C) 2021-2024, Juha-Pekka Varjonen

import serial
from tqdm import tqdm, trange
import argparse

parser = argparse.ArgumentParser(description="EEPROM Programmer")
parser.add_argument("--port", default="/dev/ttyACM0")
parser.add_argument("--size", type=int, default=262144) #131072, 32768
parser.add_argument("--file", default="hexData.txt")
parser.add_argument("action", choices=['program', 'fillzeros', 'erase', 'read', 'write', 'ident', 'test', 'reset'], default='program')
args = parser.parse_args()

# restart AVR and serial connection
s = serial.Serial(args.port, 115200, timeout=5)

# wait until command execution is ready
def waitExec(value, pbar):
    d = ''
    while not d.endswith(value + '\r\n'):
        sRead = s.read(1).decode('latin_1')
        if pbar is not None and sRead == '.':
            pbar.update(1)
        else:
            d += sRead
    return d[:-2].encode('latin_1')

print('Waiting programmer to reset...', end=' ')
# wait until AVR is ready
waitExec('ready', None)
print('Ready!')

# send command to AVR
def sendComm(value, pbar):
    s.write(value + b'\r')
    return waitExec('', pbar)

def fillZeros():
    for addr in trange(args.size, desc='Fill zeros'):
        comp = sendComm(b'write ' + str(addr).encode() + b' 0', None).decode('latin_1')
        if comp != '\x00':
            print('Verify error at address 0x{:02x}. Aborting.'.format(addr))
            exit()

def erase():
    pbar = tqdm(total=args.size, desc='Erasing')
    erase = sendComm(b'erase ' + str(args.size).encode(), pbar).decode('latin_1')
    pbar.close()
    if erase != 'pass':
        print('Erase failed at address 0x{:02x}. Aborting.'.format(ord(erase)))
        exit()

# Verifying erase command
def verifyErase():
    for addr in trange(args.size):
        data = sendComm(b'read ' + str(addr).encode(), None).decode('latin_1')
        if data != '\xff':
            print('Failed at address 0x{:02x}'.format(addr))
            break;

def reset():
    sendComm(b'reset', None)
    print('Chip reset OK.')

def ident():
    print('Manufacturer identification code is 0x{:02x}'.format(ord(sendComm(b'ident 0', None))))
    print('Chip identification code is 0x{:02x}'.format(ord(sendComm(b'ident 1', None))))

def test():
    print('Wrote 0x01 to address 0x01 and data in that address is 0x{:02x}'.format(ord(sendComm(b'write 1 1', None))))
    print('Data read from address 0x01 is 0x{:02x}'.format(ord(sendComm(b'read 1', None))))
    ident()

def read():
    with open(args.file, 'a') as f:
        for i in trange(args.size):
            f.write('{:02x}'.format(ord(sendComm(b'read ' + str(i).encode(), None))))
            if i % 20 == 0:
                f.write('\n')

# open file and write it's content to ROM and verify
def write():
    with open(args.file) as f:
        # Get file size
        f.seek(0, 2)
        fileSize = f.tell()
        f.seek(0)
        pbar = tqdm(total=fileSize, desc='Writing')
        
        addr = 0
        tt = b''
        t = f.read(1)
        pbar.update(1)
        while t:
            if len(tt) == 2:
                comp = '{:02x}'.format(ord(sendComm('write {} {}'.format(addr, int(tt, 16)).encode(), None))).encode()
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
            pbar.update(1)
        pbar.close()

if args.action == "program":
    fillZeros()
    erase()
    write()
elif args.action == "fillzeros":
    fillZeros()
elif args.action == "erase":
    erase()
elif args.action == "read":
    read()
elif args.action == "write":
    write()
elif args.action == "ident":
    ident()
elif args.action == "test":
    test()
elif args.action == "reset":
    reset()
    
# finally close connection
s.close()
