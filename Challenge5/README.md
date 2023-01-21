# Challenge_5

## Code Explanation
fs.c is an emulation of a file system used by an operating system. Everything written to the file system is stored on a file created through the disk.c library, but still fucntions as a normal fs would.

## Functionalities
### FS make, mount, unmount
fs_make will create all the variables, including inode list, used block list, and directory, needed for the file system to function and then write them to block 0 on the disk (AKA the file name specified in the input). Mount will then try to access this new file and pull all the variables back into the program. Unmount does the reverse of this and writes all these variables back to the disk with whatever was created in the interim period, so it can be accessed the next time the fs is mounted.

### FS Create, Destory, Read, Write
FS create will create a file, allocating a directory space, inode index, file descriptor, and 1 block, and write that to the mount variables. Destroy will comb through the mount vars and free any data pertaining to that file. Read/Write will find the directory, inode, and blocks associated with the file and read/write data to/from the block beginning with the file pointer (default starts at 0, but can be set with lseek).

Note: only 64 files can be created, only 32 file descriptors can be used at a time, Write will increase file size as more data is written

### Get_filesize, listfiles, lseek, truncate
Get_filesize returns the current filesize of the file corresponding to the file descriptor, listfiles fills the double array inputted in the function and fills it with all currently opened files, lseek will increase or decrease the file pointer to a specified point provided its within the files size, truncate will decrease the size of a file to a specified point

## Notes and Known Code Issues
1.) When creating multiple files and then trying to run test, an attempt to read from the first file (maybe any file) gives this error code: "sysmalloc: Assertion `(old_top == initial_top (av) && old_size == 0) || ((unsigned long) (old_size) >= MINSIZE && prev_inuse (old_top) && ((unsigned long) old_end & (pagesize - 1)) == 0)' failed."
