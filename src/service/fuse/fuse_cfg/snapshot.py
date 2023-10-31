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

from derecho.cascade.external_client import ServiceClientAPI
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

# Perform snapshot at a recent time
def do_snapshot():
    capi = ServiceClientAPI()
    time = capi.put("/pool1/k4", bytes("p1v4", 'utf-8'))
    print(time)


# Test for snapshotting including all files
def test_snapshot():
    print("----------- TEST WRITE -----------")
    expected_contents = {
        'latest/pool1/k1': 'p1v1',
        'latest/pool1/k2': 'p1v2',
        'latest/pool1/k3': 'p1v3', # new file
        'latest/pool2/k1': 'p2v1',
        'latest/pool2/k2': 'new2', # overwrite
        'latest/pool3/k1': 'p3v1',
        'latest/pool3/k2': 'p3v2p3v2'} #append
    base_directory = "test"
    actual_contents = read_files_in_directory(base_directory)
    assert expected_contents == actual_contents, "File content dictionaries do not match."
    print("----------- PASSED TEST WRITE -----------")
    return


def main(argv):
    do_snapshot()
    # test_snapshot()
    return

if __name__ == "__main__":
    main(sys.argv[1:])