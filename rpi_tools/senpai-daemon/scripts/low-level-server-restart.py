#!/usr/bin/python3

import socket

IP_ADDR = "127.0.0.1"
TCP_PORT = 23747
TOKEN = 19

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.settimeout(0.1)
try:
    s.connect((IP_ADDR, TCP_PORT))
    s.send(bytes([TOKEN]))
except socket.timeout:
    pass
s.close()
