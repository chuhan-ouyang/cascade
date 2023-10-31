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
    dict = capi.put("/pool1/k4", bytes("p1v4", 'utf-8'))
    time = 0
    if dict:
        odict = dict.get_result()
        # Get latest time
        time = odict[1]
    directory_path = os.path.join("..", "n4", "test", "snapshot", str(time))
    # Create the directory if it doesn't exist
    os.makedirs(directory_path, exist_ok=True)


def main(argv):
    do_snapshot()
    # test_snapshot()
    return

if __name__ == "__main__":
    main(sys.argv[1:])