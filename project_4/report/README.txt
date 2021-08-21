POULIANOU XAIDO 2613 cpoulianou@uth.gr 
TSANGARIS ANDREAS 2667 antsangaris@uth.gr 
POLYKARPOU CHRISTOFOROS 2668 chpolykarpou@uth.gr
________________________________________________

Project 4
-------------

Compilation: in src type -> make
Run VFS: in examples directory type -> ../src/bbfs rootdir mountdir    /* mountdir is the directory where the VFS is mounted */

***Modified Makefile****
all: 
	gcc -g -Wall bbfs.c log.c -lssl -lcrypto -D_FILE_OFFSET_BITS=64 \
	`pkg-config fuse --cflags --libs` \
	`pkg-config libssl --cflags --libs` -o bbfs
	
clean: 
	rm -rf ./bbfs

****************************

We implemented our File system by creating structs for storing information about files, blocks and metadata. We didn't use any
storage directory to store file info. 

This struct describes a file 
------------------------------------
typedef struct bb_file {
    int block[MAX_BLOCKS];    // index of blocks
    struct bb_file *files;	
    bb_metadata *metadata;
}bb_file;

New variables
--------------------
- Global variable called all_blocks where each unique file system's block is stored here.
- Global variable called root_files where all metadata info of each file is stored here.

New functions
--------------------
- check_bb_file: 	 checks if a file already exists in mountdir. Returns pointer to the file or Null if file does not exist.  
- get_name: 	 given a file's fullpath, get_name returns name of the file.
- add_on_root_dir: 	 inserts new directory node to root_files struct. 
- add_on_root: 	 inserts new file node to root_files struct.
- remove_from_root: removes file/directory from root_files struct.

Modified FUSE functions: read(), write(), readdir(), mkdir(), opendir(), open(), unlink(), mknod(), getattr(), init(), destroy()
_________________________________________________________________________________________________

 Results
------------

We created 3 test files (mountdir is the mount directory):
- test1.c: creates test_out1, writes and reads 4096 bytes containing the character 'a' 4096 times.
- test2.c: creates test_out2, writes and reads 8192 bytes containing the character 'a' 8192 times.
- test3.c: creates test_out3, writes and reads 12288 bytes containing the character 'b' 8192 times 
	and character 'a' 4096 times.

Without using our compression algorithm the FS would allocate 6 blocks of 4KB each. By running
all test files with our FS, the system allocated only 2 blocks.
The total number of blocks can be checked by reading bbfs.log and finding the last instance of  
"Total Blocks: " which is 2 in our case.

Conclusion: The file system works as expected allocating a minimum amount of memory and virtually
	    reconstructing files by reading from the blocks. 

 