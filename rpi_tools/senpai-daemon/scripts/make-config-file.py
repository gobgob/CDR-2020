#!/usr/bin/python3

import argparse

parser = argparse.ArgumentParser()
parser.add_argument("filename", help="Filename for the new file")
parser.add_argument("--ip", help="IP address in the config")
parser.add_argument("--tcp", help="TCP port in the config")
args = parser.parse_args()

if args.ip is None:
    ip = "172.16.0.2"
else:
    ip = args.ip

if args.tcp is None:
    tcp = "80"
else:
    tcp = args.tcp

with open(args.filename, 'w') as f:
    print("IP_ADDRESS=" + ip, file=f)
    print("TCP_PORT=" + tcp, file=f)
