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

# Create a new file in an object pool directory
def write_new():
    directory_path = "test/latest/pool1"
    os.makedirs(directory_path, exist_ok=True)
    file_name = "k3"
    file_content = "p1v3"
    file_path = os.path.join(directory_path, file_name)
    with open(file_path, "w") as file:
        file.write(file_content)


# Overwrite an existing content
def modify():
    file_path = "test/latest/pool2/k2"
    new_content = "new2"
    if os.path.exists(file_path):
        with open(file_path, "w") as file:
            file.write(new_content)


# Append onto an existing file
def append():
    file_path = "test/latest/pool3/k2"

    # Read the existing content from the file
    with open(file_path, "r") as file:
        existing_content = file.read()

    # Append the existing content to the file
    with open(file_path, "a") as file:
        file.write(existing_content)

def append_expected_contents(expected_contents):
    for x in range(1, 101):
        key = f'latest/pool4/k{x}'
        value = f'p4v{x}'
        expected_contents[key] = value

# Basic test for creating new files in object pool directories
def test_write():
    print("----------- TEST WRITE -----------")
    word_to_repeat = "cascade"
    # 1kb file
    repetitions = (1024 // len(word_to_repeat)) + 1
    one_kb_string = word_to_repeat * repetitions

    expected_contents = {
        'latest/pool1/k1': 'p1v1',
        'latest/pool1/k2': 'p1v2',
        'latest/pool1/1kb': one_kb_string,
        'latest/pool1/k3': 'p1v3', # new file
        'latest/pool2/k1': 'p2v1',
        'latest/pool2/k2': 'new2', # overwrite
        'latest/pool3/k1': 'p3v1',
        'latest/pool3/k2': 'p3v2p3v2',
        'latest/pool1/subpool0/subsubpool0/k1': 'p1s0s0v1',
        'latest/pool1/subpool1/k1': 'p1s1v1',
        'latest/pool1/subpool1/subsubpool0/k1': 'p1s1s0v1'} #append
    append_expected_contents(expected_contents)

    base_directory = "test"
    actual_contents = read_files_in_directory(base_directory)
    assert expected_contents == actual_contents, "File content dictionaries do not match."
    print("----------- PASSED TEST WRITE -----------")
    return

def main(argv):
    write_new()
    modify()
    append()
    test_write()
    return

if __name__ == "__main__":
    main(sys.argv[1:])