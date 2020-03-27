#!/usr/bin/python3

import socket
import argparse
import subprocess
from os import walk
from os.path import join, abspath, dirname, isfile, isdir

def ipv4_addr(string):
    sp = string.split('.')
    e = argparse.ArgumentTypeError("Invalid IP address: " + string)
    if len(sp) != 4:
        raise e
    for b in sp:
        try:
            n = int(b)
        except ValueError:
            raise e
        if not 0 <= n < 256:
            raise e
    return string

def tcp_port(string):
    try:
        port = int(string)
        if not 0 <= port < 65536:
            raise ValueError
        return port
    except ValueError:
        raise argparse.ArgumentTypeError("Invalid TCP port: " + string)

def uint8(string):
    try:
        return bytes([int(string)])
    except ValueError:
        raise argparse.ArgumentTypeError("Invalid token: " + string)

def get_last_backup(directory):
    if not isdir(directory):
        return ""
    dir_list = next(walk(directory))[1]
    dir_list.sort(reverse=True)
    if len(dir_list) == 0:
        return ""
    return dir_list[0]

parser = argparse.ArgumentParser()
parser.add_argument("filename", nargs='?',
                    default='low_level.ino.hex',
                    help="Name of the .hex file to program")
parser.add_argument("-b", "--backup", nargs='?', const='latest', default=None,
                    help="Program a backup instead of the current build. The "
                         "latest backup will be used if the backup folder is "
                         "not specified")
parser.add_argument("--ip", type=ipv4_addr, default="127.0.0.1",
                    help="IP address of the pause socket")
parser.add_argument("--tcp", type=tcp_port, default=23747,
                    help="TCP port of the pause socket")
parser.add_argument("--token", type=uint8, default=bytes([19]),
                    help="Token of the pause socket")
parser.add_argument("--teensy-loader", default="teensy_loader_cli",
                    help="Name of the teensy loader executable")
args = parser.parse_args()

base_dir = dirname(abspath(__file__))
backups_dir = join(base_dir, "backups")
teensy_loader = join(base_dir, "bin", args.teensy_loader)

if not isfile(teensy_loader):
    print("Teensy loader not found:", teensy_loader)
    exit(-1)

if args.backup is None:
    hex_file = join(base_dir, args.filename)
elif args.backup == 'latest':
    last_backup = get_last_backup(backups_dir)
    if len(last_backup) == 0:
        print("Backup directory is empty:", backups_dir)
        exit(-1)
    hex_file = join(backups_dir, last_backup, args.filename)
else:
    hex_file = join(backups_dir, args.backup, args.filename)

if not isfile(hex_file):
    print("File not found:", hex_file)
    exit(-1)

print("Request pause at " + args.ip + ":" + str(args.tcp) + " with token 0x" +
      args.token.hex())
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.settimeout(2)
try:
    s.connect((args.ip, args.tcp))
    s.send(args.token)
    s.recv(1)
    print("LowLevelServer is now on pause")
except socket.timeout:
    print("Timeout: LowLevelServer is not running or already on pause")

p = subprocess.Popen([teensy_loader, "--mcu=mk64fx512", "-v", "-s", hex_file],
                     stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                     universal_newlines=True)
try:
    while p.poll() is None:
        print(p.stdout.readline(), end='')
except KeyboardInterrupt:
    p.kill()
    p.communicate()
print(p.stdout.read(), end='')

s.close()
print("Pause socket closed")
