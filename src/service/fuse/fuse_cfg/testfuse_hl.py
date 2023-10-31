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

# Create object pools and files
def initial_setup():
    capi = ServiceClientAPI()
    print("----------- CASCADE INITIAL SETUP   -----------")

    # Create object pool
    capi.create_object_pool("/pool1", "PersistentCascadeStoreWithStringKey", 0)
    capi.create_object_pool("/pool2", "PersistentCascadeStoreWithStringKey", 0)
    capi.create_object_pool("/pool3", "PersistentCascadeStoreWithStringKey", 0)
    time.sleep(1)

    # Put in object pool
    capi.put("/pool1/k1", bytes("p1v1", 'utf-8'))
    capi.put("/pool1/k2", bytes("p1v2", 'utf-8'))
    capi.put("/pool2/k1", bytes("p2v1", 'utf-8'))
    capi.put("/pool2/k2", bytes("p2v2", 'utf-8'))
    capi.put("/pool3/k1", bytes("p3v1", 'utf-8'))
    capi.put("/pool3/k2", bytes("p3v2", 'utf-8'))

    word_to_repeat = "cascade"
    # 1kb file
    repetitions = (1024 // len(word_to_repeat)) + 1
    one_kb_string = word_to_repeat * repetitions
    encoded_bytes = one_kb_string.encode('utf-8')
    capi.put("/pool1/1kb", encoded_bytes)

    time.sleep(1)

    print("----------- FINISHED CASCADE INITIAL SETUP   -----------")

def main(argv):
    initial_setup()

if __name__ == "__main__":
    main(sys.argv[1:])
