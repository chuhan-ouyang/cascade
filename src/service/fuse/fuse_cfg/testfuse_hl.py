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

def many_files(capi):
    base_path = "/pool4/k"
    data_template = "p4vx"
    start_x = 1
    end_x = 100

    for x in range(start_x, end_x + 1):
        path = f"{base_path}{x}"
        data = bytes(data_template.replace('x', str(x), 1), 'utf-8')
        capi.put(path, data)


# Create object pools and files
def initial_setup():
    capi = ServiceClientAPI()
    print("----------- CASCADE INITIAL SETUP   -----------")

    # Create object pool
    capi.create_object_pool("/pool1", "PersistentCascadeStoreWithStringKey", 0)
    capi.create_object_pool("/pool2", "PersistentCascadeStoreWithStringKey", 0)
    capi.create_object_pool("/pool3", "PersistentCascadeStoreWithStringKey", 0)
    capi.create_object_pool("/pool4", "PersistentCascadeStoreWithStringKey", 0)

    # Nested object pools
    capi.create_object_pool("/pool1/subpool0/subsubpool0",
                             "PersistentCascadeStoreWithStringKey", 0)
    capi.create_object_pool("/pool1/subpool1/subsubpool0",
                             "PersistentCascadeStoreWithStringKey", 0)

    time.sleep(1)

    # Put in object pool
    capi.put("/pool1/k1", bytes("p1v1", 'utf-8'))
    capi.put("/pool1/k2", bytes("p1v2", 'utf-8'))
    capi.put("/pool2/k1", bytes("p2v1", 'utf-8'))
    capi.put("/pool2/k2", bytes("p2v2", 'utf-8'))
    capi.put("/pool3/k1", bytes("p3v1", 'utf-8'))
    capi.put("/pool3/k2", bytes("p3v2", 'utf-8'))

    # # Nested puts
    capi.put("/pool1/subpool0/subsubpool0/k1", bytes("p1s0s0v1", 'utf-8'))
    capi.put("/pool1/subpool1/k1", bytes("p1s1v1", 'utf-8'))
    capi.put("/pool1/subpool1/subsubpool0/k1", bytes("p1s1s0v1", 'utf-8'))

    # # Many small files
    many_files(capi)

    word_to_repeat = "cascade"
    # 1kb file (large file)
    repetitions = (1024 // len(word_to_repeat)) + 1
    one_kb_string = word_to_repeat * repetitions
    encoded_bytes = one_kb_string.encode('utf-8')
    capi.put("/pool1/1kb", encoded_bytes)

    time.sleep(10)

    print("----------- FINISHED CASCADE INITIAL SETUP   -----------")

def simple_mount():
    print("----------- CASCADE MOUNT -----------")
    mkdir_command = "mkdir test"
    cascade_command = "./cascade_fuse_client_hl -s test"
    try:
        subprocess.Popen(mkdir_command, shell=True).wait()
        print("----------- FINISHED MAKE -----------")
        subprocess.Popen(cascade_command, shell=True).wait()
        print("Commands executed successfully.")
    except subprocess.CalledProcessError as e:
        print(f"Error executing commands: {e}")
    print("----------- FINISHED CASCADE MOUNT -----------")

def main(argv):
    initial_setup()
    time.sleep(10)    
    simple_mount()

if __name__ == "__main__":
    main(sys.argv[1:])
