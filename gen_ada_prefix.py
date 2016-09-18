#!/usr/bin/env python
# -*- coding: utf-8 -*-

'''
Generate prefix for Adalight/Hyperion Arduino sketch
'''

from __future__ import print_function
import sys


def calc_prefix(num_leds):
    prefix = [ord(c) for c in "Ada"]

    high_byte = ((num_leds - 1) >> 8) & 0xff
    low_byte = (num_leds - 1) & 0xff
    checksum = high_byte ^ low_byte ^ 0x55

    prefix.extend([high_byte, low_byte, checksum])

    return prefix


def print_prefix_list(up_to):
    for i in range(1, up_to + 1):
        prefix = calc_prefix(i)
        sys.stdout.write("{:<15}".format(i))
        out = [format(b, "02x") for b in prefix]
        sys.stdout.write(" ".join(out))
        sys.stdout.write("\n")


def main():
    args = sys.argv[1:]
    if not args:
        num_leds = raw_input("Please enter total number of LEDs: ")

        if not num_leds.isdigit():
            print("You have to enter a number!")
        else:
            num_leds = int(num_leds)
            prefix = calc_prefix(num_leds)

            print("\n")
            print("Number of LEDs: {:d}".format(num_leds))
            print("\n")

            sys.stdout.write("Prefix: ")
            sys.stdout.write(" ".join([format(b, "02x") for b in prefix]))
            print("\n")
    else:
        if args[0] == "--list" or args[0] == "-l":
            if args[1].isdigit():
                print_prefix_list(int(args[1]))

    return


if __name__ == "__main__":
    main()
