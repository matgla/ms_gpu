#!/usr/bin/python3

import argparse
import sys
import os
from PIL import Image
from math import ceil

parser = argparse.ArgumentParser(description = "Sprite generator, converts image to source file")
parser.add_argument("-i", "--file", dest="input_file", action="store", help="Path to input file, for animation format is, file1,file2,file3", required=True)
args, rest = parser.parse_known_args()

def get_image_size(files):
    x_min = sys.maxsize
    x_max = 0
    y_min = sys.maxsize
    y_max = 0
    for file in files.split(","):
        img = Image.open(args.input_file)
        x_start, y_start, x_stop, y_stop = img.getbbox()
        if x_start < x_min:
            x_min = x_start
        if x_stop > x_max:
            x_max = x_stop
        if y_start < y_min:
            y_min = y_start
        if y_stop > y_max:
            y_max = y_stop

    return (x_min, y_min, x_max, y_max)

def main():
    output_file = "image.hpp" 
    print("Converting image from:", args.input_file, ", to:", output_file)
    with open(output_file, "w") as output:
        img = Image.open(args.input_file)
        output.write("constexpr uint32_t data[] = {\n")
        print ("x: " + str(img.width - (img.width % 4)) + ", y: " + str(img.height))
        pic = img.load()
        x_size = img.width - (img.width % 4)
        for y in range(img.height):
            data = 0
            offset = 0
            for x in range(x_size):
                byte = 0 
                try: 
                    byte = pic[x, y]
                except IndexError:
                    byte = 0
                if byte < 0:
                    byte = 0
                data = data | (byte << offset)
                offset += 8
                if offset == 32:                
                    output.write(hex(data) + ",\n")
                    data = 0 
                    offset = 0
        output.write("};\n")

main()