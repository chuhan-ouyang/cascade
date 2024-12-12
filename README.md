# Cascade Distributed Key-Value Store File System Interface 

Note: The tar package contain the entire Cascade code with the file system interface as part of its service.
The code specifically associated with my project is in the directory /src/service/fuse

# Prequisites
* Ubuntu 22.04
* Docker Engine 
* Access to 4 servers and 1 client node (or can use separte processes on the same machine)
* TCP / RDMA connectivity

# Environment and Updated Code
1. docker pull chuhanouyang/fusedev:multithread for contianer environment to install Cascade dependency (otherwise see https://github.com/Derecho-Project/cascade for installation)
2. pull latest cascade code from the given zip file in CMS submission or from: https://github.com/chuhan-ouyang/cascade/tree/chuhan_dev

# Build Cascade and File System Interface
1. ./build_cascade.sh (creating the build*/ directory, ex. build-Release)


# How to Run the File System Interface
1. In server1 launch Cascade server:
```bash
cd /root/workspace/cascade/build-Release/src/service/fuse/fuse_cfg/n0
./run.sh server
```
2. In server2 launch Cascade server:
```bash
cd /root/workspace/cascade/build-Release/src/service/fuse/fuse_cfg/n1
./run.sh server
```
3. In server3 launch Cascade server:
```bash
cd /root/workspace/cascade/build-Release/src/service/fuse/fuse_cfg/n2
./run.sh server
```
4. In server4 launch Cascade server:
```bash
cd /root/workspace/cascade/build-Release/src/service/fuse/fuse_cfg/n3
./run.sh server
```

5. In client node put in some key/value store objects using the Python API:
```bash
cd /root/workspace/cascade/build-Release/src/service/fuse/fuse_cfg/n3
python fuse_put.py
```

6. In client node, launch the multithreadedFUSE file system, and mount it on a 'test' directory (all done in script):
```bash
./fuse_run_multi.sh.tmp 
```

7. Open another terminal in the client node, interact with the file system
```bash
cd test # mount point of the filesystem
cd latest # dir storing the object pools and their objects
```
In /test/latest, the client will see all existing object pools (from fuse_put.py) as directories and all key value obejcts as files
The client can read/write these files and they will be reflected in the distributed system.

# How to Run the Read/Write Performance Test
Run server nodes as above.
1. In client node put objects for the read test (if performing read benchmarks): 
```bash
cd /root/workspace/cascade/build-Release/src/service/fuse/fuse_cfg/n4
./fuse_perftest_put <-s> <file size in KB> <-n> <num runs>
```
2. In client node launch the file system:
```bash
./fuse_run_multi.sh.tmp
```
3 Open another terminal in the client node, execute benchmark code:
```bash
./fuse_perftest <read/write> <-s> <file size in KB> <-n> <num runs> <-v> (optional)>
```
Read results will be saved in fuse_client_logger.csv, denoting each run and the start and finish timestamps in ns.
Write results will be saved in write_perf*.csv file, denoting each run and the start and finish timestamps in ns.