#!/usr/bin/env python
# -*-coding:utf-8-*-

import sys
import cv2 as cv
import numpy as np

src_file = sys.argv[1]
out_file = sys.argv[2]

SCREEN_SIZE = (128, 64)

def img2bin(src):
    img = cv.imread(src)
    cv.imshow("origin", img)

    img_gray = cv.cvtColor(img, cv.COLOR_BGR2GRAY)
    cv.imshow("gray", img_gray)

    img_resized = cv.resize(img_gray, SCREEN_SIZE)
    cv.imshow("resize", img_resized)

    ret, img_bin = cv.threshold(img_resized, 144, 255, cv.THRESH_BINARY)
    cv.imshow("binary", img_bin)

    return img_bin.tolist()

def bin2array(img_list):
    x = len(img_list)
    y = len(img_list[0])
    size = x * y
    out = bytearray(size//8)

    for i in range(x):
        for j in range(y):
            if img_list[i][j] > 0:
                index = (i*y+j)//8
                off = 7 - j % 8
                out[index] = out[index] | (1 << off)

    return out

if __name__ == "__main__":
    img_list = img2bin(src_file)
    img_array = bin2array(img_list)
    fout = open(out_file, "wb")
    fout.write(img_array)
    fout.close
    cv.waitKey(0)
