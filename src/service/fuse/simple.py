#!/usr/bin/env python3

import errno
import filecmp
import getopt
import multiprocessing
import os
import queue
import shutil
import stat
import subprocess
import sys
import tempfile
import threading
import time
from contextlib import contextmanager
from os.path import join as pjoin
from tempfile import NamedTemporaryFile

def simple_mount():
    # Command 1: Create a directory named "test"
    mkdir_command = "mkdir test"

    # Command 2: Run ./cascade_fuse_client_hl with options -s and -f test
    cascade_command = "./cascade_fuse_client_hl -s -f test"

    try:
        # Run the first command to create the directory
        subprocess.Popen(mkdir_command, shell=True).wait()

        # Run the second command to execute ./cascade_fuse_client_hl
        subprocess.Popen(cascade_command, shell=True).wait()

        print("Commands executed successfully.")
    except subprocess.CalledProcessError as e:
        print(f"Error executing commands: {e}")

def main(argv):
    simple_mount()

if __name__ == "__main__":
    main(sys.argv[1:])