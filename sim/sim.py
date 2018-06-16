#!/usr/bin/env python3

import socket
import os
from time import sleep


DEST = ("127.0.0.1", 4200)
FILE = "pflog.txt"


def main():
    with open(os.path.join(os.path.dirname(__file__), FILE), "r") as f:
        lines = [line.rstrip().encode("UTF-8") for line in f]
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    while True:
        for line in lines:
            sock.sendto(line, DEST)
            sleep(1)

if __name__ == '__main__':
    main()
