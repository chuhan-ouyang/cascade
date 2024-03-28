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
    simple_mount()


if __name__ == "__main__":
    main(sys.argv[1:])
