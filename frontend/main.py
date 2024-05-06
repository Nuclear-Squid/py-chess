#!/bin/env python3
import ctypes
import pathlib

def main():
    # c_lib_name = pathlib.Path().absolute() / "lib.so"
    c_lib_name = pathlib.Path().absolute() / "zig-out/lib/libchess.so"
    c_lib = ctypes.CDLL(c_lib_name)
    c_lib.print_int(12)

if __name__ == "__main__":
    main()
