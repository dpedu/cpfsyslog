#!/usr/bin/env python3

import socket
import os
from time import sleep
import argparse


DEST = "127.0.0.1"


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("file")
    parser.add_argument("port", type=int, default=4200)
    parser.add_argument("rate", type=float, default=1)
    args = parser.parse_args()

    with open(os.path.join(os.path.dirname(args.file), args.file), "r") as f:
        lines = [line.rstrip().encode("UTF-8") for line in f]
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    while True:
        for line in lines:
            sock.sendto(line, (DEST, args.port))
            sleep(args.rate)


if __name__ == '__main__':
    main()
