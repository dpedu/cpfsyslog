#!/usr/bin/env python3

import socket
import os
from time import sleep
import argparse

from random import random

DEST = "127.0.0.1"


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("file")
    parser.add_argument("port", type=int, default=4200)
    parser.add_argument("rate", default=1)
    args = parser.parse_args()

    sleep_times = [float(i) for i in args.rate.split("-")]

    def sleep_lenght():
        if len(sleep_times) == 2:
            return (max(sleep_times) - min(sleep_times)) * random() + min(sleep_times)
        else:
            return sleep_times[0]

    with open(os.path.join(os.path.dirname(args.file), args.file), "r") as f:
        lines = [line.rstrip().encode("UTF-8") for line in f]
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    while True:
        for line in lines:
            sock.sendto(line, (DEST, args.port))
            print(sleep_lenght())
            sleep(sleep_lenght())


if __name__ == '__main__':
    main()
