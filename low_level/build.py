#!/usr/bin/python3

REMOTE_ADDR = "pi@192.168.0.25"  # User@address where the code should be uploaded
# REMOTE_ADDR = "pi@172.24.1.1"
REMOTE_FOLDER = "senpai-flash"  # On the remote folder to copy the .hex in (under ~)
REMOTE_PROGRAMMER = "senpai-flash.py"  # Name of the program to use to flash (should be located under REMOTE_FOLDER)
REMOTE_USER_DIR = "/home/pi/"
ARDUINO_EXE = "arduino_debug"  # Executable to use for building the code
ARDUINO_PROJECT = "low_level.ino"  # Project to compile
ARDUINO_BOARD = "teensy:avr:teensy35"  # Board Teensy3.5
ARDUINO_BOARD_OPTIONS = ":usb=serial,speed=120,opt=o2std,keys=en-us"
BUILD_DIR = "_build"

import argparse
import subprocess
import shutil
from functools import partial
from os import mkdir
from os.path import join, abspath, dirname, isfile, isdir

def print_compiler_line(line):
    s_line = line.split()
    if not (len(s_line) > 0 and s_line[0] in ["TRACE", "DEBUG", "INFO"]):
        print(line, end='')

def remote_folder():
    global REMOTE_ADDR, REMOTE_FOLDER
    return REMOTE_ADDR + ":~/" + REMOTE_FOLDER

def remote_programmer():
    global REMOTE_FOLDER, REMOTE_PROGRAMMER, REMOTE_USER_DIR
    return REMOTE_USER_DIR + REMOTE_FOLDER + "/" + REMOTE_PROGRAMMER

def find_last_hex_file(base_dir):
    global BUILD_DIR, ARDUINO_PROJECT
    hex_file = join(base_dir, BUILD_DIR, ARDUINO_PROJECT + ".hex")
    if not isfile(hex_file):
        raise FileNotFoundError
    return hex_file

def execute(command, print_func=None):
    if print_func is None:
        print_func = partial(print, end='')
    else:
        assert callable(print_func)

    p = subprocess.Popen(command, stdout=subprocess.PIPE,
                         stderr=subprocess.STDOUT, universal_newlines=True,
                         encoding='utf-8')
    try:
        ret_code = None
        while ret_code is None:
            print_func(p.stdout.readline())
            ret_code = p.poll()
    except Exception:
        p.kill()
        p.communicate()
        raise
    print_func(p.stdout.read())
    return ret_code

def build_project(base_dir, clean_build=False, verbose=False):
    global ARDUINO_EXE, ARDUINO_PROJECT, ARDUINO_BOARD, ARDUINO_BOARD_OPTIONS
    global BUILD_DIR

    # Check Arduino executable
    if shutil.which(ARDUINO_EXE) is None:
        raise FileNotFoundError("Arduino executable not found:" + ARDUINO_EXE)

    # Check Arduino project
    ard_project = join(base_dir, ARDUINO_PROJECT)
    if not isfile(ard_project):
        raise FileNotFoundError("Arduino project not found:" + ard_project)

    # Create build directory if necessary
    build_dir = join(base_dir, BUILD_DIR)
    if not isdir(build_dir):
        mkdir(build_dir)
    elif clean_build:
        shutil.rmtree(build_dir, ignore_errors=True)
        mkdir(build_dir)

    # Launch build process
    options = ["--verify", "--board", ARDUINO_BOARD + ARDUINO_BOARD_OPTIONS,
               "--pref", "build.path=" + build_dir]
    if verbose:
        options.append("-v")
    ret = execute([ARDUINO_EXE] + options + [ard_project], print_compiler_line)
    if ret == 0:
        return join(build_dir, ARDUINO_PROJECT + ".hex")
    else:
        raise RuntimeError("Compilation failed")

def upload_hex(hex_file):
    if shutil.which("scp") is None:
        raise FileNotFoundError("scp command not available")

    if execute(["scp", hex_file, remote_folder()]) != 0:
        raise RuntimeError("Failed to copy .hex file on remote")

def flash_hex():
    if shutil.which("ssh") is None:
        raise FileNotFoundError("ssh command not available")
    remote_command = "'" + remote_programmer() + "'"
    if execute(["ssh", REMOTE_ADDR, remote_command]) != 0:
        raise RuntimeError("Failed to flash .hex on remote")

def parse_input(parser):
    """
    :param parser: ArgumentParser() instance
    :return: tuple of boolean (build, flash, backup, clean, verbose)
    """
    parser.add_argument("-v", "--verbose", action='store_true',
                        help="Enable verbose during compilation")
    parser.add_argument("-c", "--clean", action='store_true',
                        help="Clean previously compiled files before compiling")
    group = parser.add_mutually_exclusive_group()
    group.add_argument("-b", "--build-only", action='store_true',
                        help="Only build the .hex")
    group.add_argument("-f", "--flash-only", action='store_true',
                        help="Only upload and flash the last .hex")
    args = parser.parse_args()

    if args.build_only:
        return True, False, args.clean, args.verbose
    elif args.flash_only:
        return False, True, args.clean, args.verbose
    else:
        return True, True, args.clean, args.verbose

def main():
    base_dir = dirname(abspath(__file__))
    build, flash, clean, verbose = parse_input(argparse.ArgumentParser())
    hex_file = None

    if not build:
        try:
            hex_file = find_last_hex_file(base_dir)
        except FileNotFoundError:
            pass

    if build:
        try:
            print("Building...")
            hex_file = build_project(base_dir, clean, verbose)
        except (OSError, RuntimeError) as e:
            print(e)
            return

    if flash:
        if hex_file is None:
            print("No .hex file to flash")
            return
        try:
            print("Uploading to remote...")
            upload_hex(hex_file)
            print("Flashing on teensy...")
            flash_hex()
        except (OSError, RuntimeError) as e:
            print(e)
            return

if __name__ == "__main__":
    main()
