#!/usr/bin/env python
# -*-coding:utf-8-*-

import sys
import cv2 as cv
import numpy as np
import socket

HOST = "192.168.2.7"
PORT = 3333

def sock_init():
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
        sock.connect((HOST, PORT))
        return sock

def send_data(sock, data):
    sock.sendall(data)

def format_to_bin_img(img_src, size):
    #cv.imshow("origin", img)
    img_gray = cv.cvtColor(img_src, cv.COLOR_BGR2GRAY)
    #cv.imshow("gray", img_gray)
    img_resized = cv.resize(img_gray, size)
    #cv.imshow("resize", img_resized)
    ret, img_bin = cv.threshold(img_resized, 144, 255, cv.THRESH_BINARY)
    #cv.imshow("binary", img_bin)
    return img_bin

def bin_img_to_bytes(img_bin):
    img_list = img_bin.tolist()
    x = len(img_list)
    y = len(img_list[0])
    size = x * y
    out = bytearray(size//8)

    for i in range(x):
        for j in range(y):
            if img_list[i][j] > 0:
                #index = (i*y+j)//8
                index = (i*y+j)>>3
                #off = 7 - j % 8
                off = 7 - j & 0x7
                out[index] = out[index] | (1 << off)
    return out

def video_decode(video, size):
    cap = cv.VideoCapture(video)
    if not cap:
        print("can't open " + video)
        exit()
        
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
        sock.connect((HOST, PORT))
        while True:
            ret, frame = cap.read()
            cv.imshow("origin", frame)
            if not ret:
                break
            img = format_to_bin_img(frame, size)
            #cv.imshow("bin", img)
            data = bin_img_to_bytes(img)
            sock.sendall(data)
            if cv.waitKey(25) == ord('q'):
                break
        cap.release()
        cv.destroyAllWindows()

if __name__ == "__main__":
    screen_size = (128, 64)
    video_decode(sys.argv[1], screen_size)
