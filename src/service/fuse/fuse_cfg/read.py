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

def read_files_in_directory(relative_dir):
    start_directory = os.path.join(os.getcwd(), relative_dir)
    file_contents_dict = {}

    for root, dirs, files in os.walk(start_directory):
        # Skip directories that start with '.'
        dirs[:] = [d for d in dirs if not d.startswith('.')]

        for file in files:
            file_path = os.path.join(root, file)
            relative_path = os.path.relpath(file_path, start_directory)
            with open(file_path, 'r') as f:
                file_contents = f.read()
                file_contents_dict[relative_path] = file_contents

    return file_contents_dict

# Basic test for files and object pools
def test_read():
    print("----------- TEST READ -----------")
    word_to_repeat = "cascade"
    # 1kb file
    repetitions = (1024 // len(word_to_repeat)) + 1
    one_kb_string = word_to_repeat * repetitions

    expected_contents = {
        'latest/pool1/k1': 'p1v1',
        'latest/pool1/k2': 'p1v2',
        'latest/pool1/1kb': one_kb_string,
        'latest/pool2/k1': 'p2v1',
        'latest/pool2/k2': 'p2v2',
        'latest/pool3/k1': 'p3v1',
        'latest/pool3/k2': 'p3v2',
        'latest/pool1/subpool0/subsubpool0/k1': 'p1s0s0v1',
        'latest/pool1/subpool1/k1': 'p1s1v1',
        'latest/pool1/subpool1/subsubpool0/k1': 'p1s1s0v1',}

    base_directory = "test"
    actual_contents = read_files_in_directory(base_directory)
    print(actual_contents)
    assert expected_contents == actual_contents, "File content dictionaries do not match."
    print("----------- PASSED TEST READ -----------")
    return

def main(argv):
    test_read()

if __name__ == "__main__":
    main(sys.argv[1:])