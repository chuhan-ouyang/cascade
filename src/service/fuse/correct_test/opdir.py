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

from util import (
    base_cmdline,
    basename,
    cleanup,
    compare_dirs,
    fuse_proto,
    powerset,
    safe_sleep,
    test_printcap,
    umount,
    wait_for_mount,
)

def list_directories_relative(path, base_directory):
    relative_paths = []
    for root, dirs, files in os.walk(path):
        for dir_name in dirs:
            relative_path = os.path.relpath(os.path.join(root, dir_name), base_directory)
            relative_paths.append(relative_path)
    return relative_paths

# Basic test for all object pools
def test_opdir():
    print("----------- TEST OPDIR -----------")
    expected_dirs = ["test/latest",
                     "test/snapshot",
                     "test/latest/pool1",
                     "test/latest/pool1/subpool0",
                     "test/latest/pool1/subpool1",
                     "test/latest/pool1/subpool0/subsubpool0",
                     "test/latest/pool1/subpool1/subsubpool0",
                     "test/latest/pool2",
                     "test/latest/pool3",
                     "test/latest/pool4",
                     "test/latest/.cascade"]
    folder_name = "test"
    base_directory = os.getcwd()
    folder_path = os.path.join(base_directory, folder_name)
    actual_dirs = list_directories_relative(folder_path, base_directory)
    assert compare_dirs(expected_dirs, actual_dirs)
    print("----------- PASSED TEST OPDIR -----------")
    return

def main(argv):
    test_opdir()

if __name__ == "__main__":
    main(sys.argv[1:])