# Fuse API

Cascade service supports FUSE(Filesystem in Userspace) API to access data. It is
a standalone client application that links with
[`libfuse`](https://github.com/libfuse/libfuse). Cascade fuse api mount the file
system to interact with Cascade K/V stored objects.

If you didn't see
`-- Looking for include files fuse3/fuse.h, fuse3/fuse_lowlevel.h - found`, it
is probably because the libfuse support is not installed. FUSE application shall
be built as an executable in the built directory
`src/service/fuse/cascade_fuse_client`

Running FUSE POSIX shell application is similar to run cascade cmd client. Once
the cascade service is configured and started, in the client node, running
`../../fuse/cascade_fuse_client [directory_to_mount]` will mount cascade file
systsem to the dedicated directory.

Once fuse application is mounted to the directory, you can access the K/V object
stored in cascade using linux shell command. The structured of the mounted file
system is as following

```bash
.
|-- MetadataService
|-- PersistentCascadeStoreWithStringKey
|   |-- subgroup-0
|       |-- shard-0
|       |-- .cascade
|           |-- key-0
|       ...
|-- VolatileCascadeStoreWithStringKey
|   |-- subgroup-0
|       |-- shard-0
|       |-- .cascade
|       ...
|-- TriggerCascadeStoreWithStringKey
|-- ObjectPools
|   |-- objectpoolpathname-a
|       |-- a1
|           |-- objectpool object 0
|           ...
|           |-- .cascade
|   |-- .cascade
|-- .cascade
```

## Supported Commands:

```
cd [dir]              open directory
ls [dir]              list directory
ls -a                 list directory with attributes
cat [file]            read file
cat .cascade          read directory metadata information
echo ".." > [file]    write file
echo ".." >> [file]   append to file
vim [file]            vim read/write file
```

## Correctness Test
Start a cascade fuse file system client. Test the correctness of setting the directories in the file system, reading contents of files, handling large files, writing new files and appending to current files. 
1. run 4 cascade servers in n0 .. n3 of ../fuse_cfg
2. cd into n4  
```bash
cd ../fuse_cfg/n4
```
3. run correctness test
```bash
python correct_test/testfuse_hl.py
```
4. after test, run umount script
```bash
./umount.sh
```

## Performance Test
Use the cascade timestamp logger to record the total latency and latency breakdown of read and write operations. 
1. run 4 cascade servers in n0 .. n3 of ../fuse_cfg
2. cd into n4
```bash
cd ../fuse_cfg/n4
```
3. run perftest put
```bash
./fuse_perftest_put <-s> <file size in KB> <-n> <num runs>
```
4. make directory for mounting the file syste
```bash
mkdir test
```
5. run cascade fuse client
```bash
./cascade_fuse_client -f test
```
6. start a new terminal for n4
7. run perftest
```bash
./fuse_perftest <read/write> <-s> <file size in KB> <-n> <num runs> <-v> (optinal)
```
Result is in the csv files in the n4 directory