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

# Check the output dir using popen
# Hardcode expected directory
# Make sure the directories are actually there
def initial_setup():
    capi = ServiceClientAPI()
    print("----------- CASCADE INITIAL SETUP   -----------")

    # Create object pool
    capi.create_object_pool("/pool1", "PersistentCascadeStoreWithStringKey", 0)
    capi.create_object_pool("/pool2", "PersistentCascadeStoreWithStringKey", 0)
    capi.create_object_pool("/pool3", "PersistentCascadeStoreWithStringKey", 0)
    time.sleep(5)

    # Put in object pool
    capi.put("/pool1/k1", bytes("p1v1", 'utf-8'))
    capi.put("/pool1/k2", bytes("p1v2", 'utf-8'))
    capi.put("/pool2/k1", bytes("p2v1", 'utf-8'))
    capi.put("/pool2/k2", bytes("p2v2", 'utf-8'))
    capi.put("/pool3/k1", bytes("p3v1", 'utf-8'))
    capi.put("/pool3/k2", bytes("p3v2", 'utf-8'))
    time.sleep(5)
    print("----------- FINISHED CASCADE INITIAL SETUP   -----------")

def test_opdir():
    expected_dirs = []
    actual_dirs = os.listdir()
    assert compare_dirs(expected_dirs, actual_dirs)
    return

def test_mkdir():
    return

def test_read():
    return

def test_write():
    return

def cascade_fuse_mount(mnt_dir):
    # 1. create mounting piont
    subprocess.Popen(["mkdir", "test"])
    cmd_input = base_cmdline + [pjoin(basename, "cascade_fuse_client_hl"), mnt_dir, "-f", "-s"]
    # 2. run cascade_fuse_client_hl
    mount_process = subprocess.Popen(
        cmd_input, stdout=subprocess.PIPE, stderr=subprocess.PIPE
    )
    # try:
    wait_for_mount(mount_process, mnt_dir)
    # except:
    #     cleanup(mount_process, mnt_dir)
    #     raise
    # else:
    #     umount(mnt_dir)

def simple_mount():
    print("----------- CASCADE MOUNT -----------")
    mkdir_command = "mkdir test"
    cascade_command = "./cascade_fuse_client_hl -s -f test"
    try:
        subprocess.Popen(mkdir_command, shell=True).wait()
        print("----------- FINISHED MAKE -----------")
        subprocess.Popen(cascade_command, shell=True).wait()
        print("Commands executed successfully.")
    except subprocess.CalledProcessError as e:
        print(f"Error executing commands: {e}")
    print("----------- FINISHED CASCADE MOUNT -----------")


# TODO: Check whether umount succeeded (see whether process still exist)
def cascade_fuse_umount(mnt_dir):
    # 1. umount the mounting process at mnt_dir
    umount(mnt_dir)
    # 2. delete the mounted directory
    subprocess.Popen(["rm", "-r", mnt_dir])
    print(" ~~~~~~~~~~~~~  FINISHED cascade_fuse_client_hl unmount  ~~~~~~~~~~~\n")


def main(argv):
    cascade_process = multiprocessing.Process(target=initial_setup)
    cascade_process.start()
    cascade_process.join()

    # TODO: getting stuck here?
    # fuse_process = multiprocessing.Process(target=simple_mount)
    # fuse_process.start()
    # fuse_process.join()

    # cascade_fuse_mount(mnt_dir)


if __name__ == "__main__":
    main(sys.argv[1:])
