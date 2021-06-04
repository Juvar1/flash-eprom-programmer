#!/usr/bin/env python3

# bin2hex.py
# This program converts binary ascii file to hex ascii.
# Script rejects all non-binary characters.
#
# Copyright (C) 2021, Juha-Pekka Varjonen

result = open('hexData.txt', 'w')

with open('binData.txt') as f:
    tt = ''
    count = 0
    t = f.read(1)
    while t:
        if len(tt) == 8:
            result.write('{:02x}'.format(int(tt, 2)))
            count += 1
            tt = ''
            if not count % 32:
                result.write('\r')
        t = f.read(1)
        if t in ['1', '0']:
            tt += t
    else:
        print('Processing of {} bytes complete.'.format(count))

result.close()