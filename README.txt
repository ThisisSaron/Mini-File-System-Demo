**************************************
* In-Memory Filesystem Lab: Overview *
**************************************

    In this lab, you will implement a simple in-memory filesystem that simulates
    the basic operations of a Unix-like filesystem. The filesystem runs entirely in
    memory and supports a small subset of filesystem operations, such as creating
    files, reading/writing files, and managing directories. This lab will help you
    understand the concepts of filesystems, inodes, block devices, and file
    descriptors.

    The lab will require you to work with the provided code template, which
    implements an incomplete filesystem structure using an emulated block device. You
    will need to extend and modify this code to support additional filesystem
    features.

	C does not support object oriented programming. Instead, I defined a filesystem
	struct and it should be passed as the first parameter to all functions that work
	with the filesystem. Technically this lets you have multiple filesystems but you
	won't need to do this.

    Note: splitname.h contains a function that can split a file path into substrings.
	This will be useful. An example of how to use it can be found near the bottom of
    filesystem.c. This is provided because string handling in C is not fun.

*************
* Lab Tasks *
*************
    The short version: Add support for multi-block files and nested directories in all functions.
    You will need to add appropriate test code in main() as well.

    1. File Creation

        The create_file() function creates a new file by allocating an inode and
        initializing its metadata. The current implementation assumes a file can be
        created by simply allocating the first free inode.

        Tasks:
            Extend this function to handle directory structure.
            Given a path, put the new file in the correct directory.
            You will need to use the helper function splitname()
			Also make any changes needed to support multi-block files (at least 4 blocks, for 256 B)
			Note that an inode contains two block pointers.
			You can use the second one for indirect blocks.

    2. File Opening/Closing

        The open_file() function opens a file by its inode and returns a file
        descriptor. A file descriptor is used to track the current read/write position
        in the file.

		the close_file() function takes a file descriptor of an open file and closes it.

        Tasks:
            Extend these functions to handle directory structure.
            Given a path, open the file from the correct directory.
            You will need to use the helper function splitname()

    3. File Reading and Writing

        The read_file() and write_file() functions allow reading and writing data to
        the file system. These functions interact with blocks of the block device,
        simulating actual data I/O.

        Tasks:
			Make any changes needed to support subdirectories.
			Also make any changes needed to support multi-block files (at least 4 blocks, for 256 B)
            Implement support for multiple blocks, ensuring that larger files span correctly
            across multiple blocks. You should use indirect block addressing to handle
            large files and fragmentation.

    4. Directory creation

        The filesystem currently only supports basic file creation. You need to extend
        it to support directories.

        Tasks:
        Implement the mkdir() function to create directories (including nested directories).

    5. File and Directory Deletion

        The rm() function is a placeholder.

        Tasks: Implement proper file deletion, which involves:
        (1) Marking the inode as free.  (2) Marking the blocks used by the file as free.

		It should also be able to delete an empty directory, but it should refuse to delete
        a non-empty directory (return -1).

    6. File Listing
		The list_files() function is a placeholder.
        Tasks:
		Use depth-first search and show file hierarchy.
		It should produce output that shows the entire file tree using indentation:
		/
          foo.txt
          bar/
            baz.txt
            foo/
              hello.txt
          file.txt


***********
* Grading *
***********
    Grade will be based on the portion of the above features that work correctly.
    Partial credit will be awarded for code/comments that indicate understanding.
	Grading may be adjusted in case of disaster.

*********************
* How to turn it in *
*********************
    Use the server turnin script to submit your new version of filesystem.c
    turnin lab-filesystem@cs425 filesystem.c