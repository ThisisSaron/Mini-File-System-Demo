// PUT YOUR NAMES HERE:
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include "splitname.h"

//////////////////////////////////////
// Do not modify the following code //
//////////////////////////////////////

// DO NOT MODIFY
#define BLOCK_SIZE 64  // Block size (in bytes)
#define NUM_BLOCKS 512  // Number of blocks in the block device
#define MAX_FILES 64  // Max number of files in the filesystem (also the num of inodes)
#define INODE_SIZE 128  // Size of an inode structure (adjustable)
#define NO_TYPE 0
#define IS_FILE 1
#define IS_DIR 2
// Note: MAX_FILENAME_LEN is defined as 16 in splitname.h. Do not change it.

// DO NOT MODIFY
// Whether a block is used or free
typedef enum {
        FREE = 0,
        USED = 1
} BlockStatus;

// DO NOT MODIFY
// Emulated Block Device
typedef struct {
        uint8_t blocks[NUM_BLOCKS][BLOCK_SIZE];  // Array of blocks
        BlockStatus block_status[NUM_BLOCKS];  // Block status (FREE or USED)
} BlockDevice;

// DO NOT MODIFY
// Inode structure
typedef struct {
        uint32_t inode_id;       // Inode ID (unique identifier)
        uint32_t size;           // File size in bytes
        uint32_t block_pointers[2];  // Direct block pointers (assuming 2 direct blocks)
        uint32_t file_type;     // File type (e.g., NO_TYPE, IS_FILE, IS_DIR)
        uint32_t parent_inode; // Parent directory inode (for hierarchical filesystem)
        char name[MAX_FILENAME_LEN+1]; // The file name
} Inode;

// DO NOT MODIFY
// Superblock structure
typedef struct {
        uint32_t inode_count;     // Number of inodes
        uint32_t block_count;     // Total number of blocks
        uint32_t free_inodes;     // Number of free inodes
        uint32_t free_blocks;     // Number of free blocks
        uint32_t root_inode_id;   // Root directory inode ID
} Superblock;

// DO NOT MODIFY
// File descriptor structure
typedef struct {
        uint32_t inode_id;               // Inode ID associated with the file
        uint32_t current_offset;   // Current read/write offset
} FileDescriptor;

// DO NOT MODIFY
// Pseudo Filesystem Structure
typedef struct {
        BlockDevice *block_device;      // Block device emulation
        Superblock superblock;     // Superblock (filesystem metadata)
        Inode inodes[MAX_FILES];         // Inodes (file metadata)
        FileDescriptor open_files[MAX_FILES]; // Open files (for file descriptor management)
} Filesystem;

// DO NOT MODIFY
// Function declarations
void init_block_device(BlockDevice *device);
void init_filesystem(Filesystem *fs);
int create_file(Filesystem *fs, const char *filename);
int open_file(Filesystem *fs, const char *filename);
int read_file(Filesystem *fs, int fd, void *buf, size_t size);
int write_file(Filesystem *fs, int fd, const void *buf, size_t size);
int mkdir(Filesystem *fs, const char *dirname);
int rm(Filesystem *fs, const char *filename);
void list_files(Filesystem *fs);

// DO NOT MODIFY
void init_block_device(BlockDevice *device) {
        // Set all bytes to 0
        memset(device, 0, sizeof(BlockDevice));
        for (int i = 0; i < NUM_BLOCKS; i++) {
                // Mark all blocks as free
                device->block_status[i] = FREE;
        }
}

//////////////////////////////////////////////
//  Do not modify any code above this line. //
//     Your work goes below this comment    //
/////////////////////////////////////////////

void init_filesystem(Filesystem *fs) {
        fs->block_device = malloc(sizeof(BlockDevice));
        init_block_device(fs->block_device);

        // Initialize superblock
        fs->superblock.inode_count = 0;
        fs->superblock.block_count = NUM_BLOCKS;
        fs->superblock.free_inodes = MAX_FILES;
        fs->superblock.free_blocks = NUM_BLOCKS;
        fs->superblock.root_inode_id = 1; // Root directory inode

        // Initialize the root inode
        fs->inodes[0].inode_id = 1; // inodes start at 1, even though the index starts at 0
        fs->inodes[0].size = 0;
        memset(fs->inodes[0].block_pointers, 0, sizeof(fs->inodes[0].block_pointers));
        fs->inodes[0].file_type = IS_DIR;    // Root inode is a directory
        fs->inodes[0].parent_inode = 0; // Root inode has no parent
        char rootname = '/';
        strncpy(fs->inodes[0].name, &rootname, 1); // Give it a name so we can see it in listings

        // The root dir needs to have a data block to store inode references
        // The current format is as follows:
        //    First byte is the number of files
        //    Other bytes contain the inode ids
        // Note that file deletion will cause problems (you should try to fix this)
        fs->block_device->block_status[0] = USED;
        fs->inodes[0].block_pointers[0] = 1; // Note: Again, this means block index 0, not index 1
        fs->superblock.free_blocks -=1;

        // Initialize other inodes along with the open file table
        for (int i = 1; i < MAX_FILES; i++) {
                fs->inodes[i].inode_id = 0; // Unused inode
                fs->inodes[i].size = 0;
                memset(fs->inodes[i].block_pointers, 0, sizeof(fs->inodes[i].block_pointers));
                fs->inodes[i].file_type = NO_TYPE; // No type
                fs->inodes[i].parent_inode = 0; // No parent

                fs->open_files[i].inode_id = 0;
                fs->open_files[i].current_offset = 0;
        }
}

// Create a new file in the filesystem
int create_file(Filesystem *fs, const char *filename) {
        // Note: Currently allows creation of multiple files with same name.
        // You can fix this if you want, or just avoid it in testing.

        if (fs->superblock.free_inodes == 0) {
                return -1; // No free inodes left
        }

        // Find a free inode
        int inode_id = -1;
        for (int i = 0; i < MAX_FILES; i++) {
                if (fs->inodes[i].inode_id == 0) {  // Free inode found
                        inode_id = i+1; // Can't be 0 since 0 is not a valid inode
                        break;
                }
        }

        if (inode_id == -1) {
                return -1; // No free inodes found
        }

        // Initialize the inode for the new file
        fs->inodes[inode_id-1].inode_id = inode_id;
        fs->inodes[inode_id-1].size = 0; // File size is initially 0
        strncpy(fs->inodes[inode_id-1].name, filename, MAX_FILENAME_LEN);
        fs->inodes[inode_id-1].file_type = IS_FILE; // File (not directory)
        fs->superblock.free_inodes--;

        // For now, this code will just put any new file into the root dir
        unsigned blocknum = fs->inodes[0].block_pointers[0];
        // See how many files existed here before
        unsigned num_prior_files = (fs->block_device->blocks[blocknum-1])[0];
        printf("Dir previously contained %d files\n",num_prior_files);

        // Each entry is a 32 bit id and the first one is 4 bytes in, since the first entry is a counter
        ((unsigned *) (fs->block_device->blocks[blocknum-1]))[1+num_prior_files] = inode_id;
        // Add one to the file count for this dir (cast is for 32-bit field)
        ((unsigned *) (fs->block_device->blocks[blocknum-1]))[0] += 1;

        printf("File '%s' created with inode %d\n", filename, inode_id);
        return inode_id;
}

// Open an existing file by filename
// Returns a file descriptor
int open_file(Filesystem *fs, const char *filename) {
        // Note: Doesn't split the path (/ is not supported). You need to fix this to support directories.

        // Find the file based on the name
        int inode_id = -1;
        for (int i = 0; i < MAX_FILES; i++) {
                if ((fs->inodes[i].file_type == IS_FILE)&&(!strcmp((fs->inodes[i].name), filename))) {
                        inode_id = fs->inodes[i].inode_id;
                        break;
                }
        }

        // Error checks
        if (inode_id == -1) {
                printf("File not found\n");
                return -1;
        }
        else if(inode_id == 0) {
                printf("Error: inode id is 0!\n");
                return -1;
        }
        printf("Found file with inode %d\n",inode_id);

        // Refuse to open a file that is already open
        for(int i=0; i<MAX_FILES; i++){
                if (fs->open_files[i].inode_id == inode_id){
                        printf("Error: This file is already open with fd = %d\n",i);
                        return -1;
                }
        }

        // Open the file and return a file descriptor (fd)
        for (int i = 0; i < MAX_FILES; i++) {
                if (fs->open_files[i].inode_id == 0) {
                        fs->open_files[i].inode_id = inode_id;
                        return i;
                }
        }

        return -1;
}


// Closes a specified file descriptor, unless it isn't open
int close_file(Filesystem *fs, int fd) {
        if (fs->open_files[fd].inode_id != 0) {
                fs->open_files[fd].inode_id =0;
                return 1;
        }
        return -1; // The fd is not open
}

// Takes a file system ptr, a file descriptor int, a buffer, and a size.
// Read from an open file into a provided buffer buf.
int read_file(Filesystem *fs, int fd, void *buf, size_t size) {
        if (fd < 0 || fd >= MAX_FILES || fs->open_files[fd].inode_id == 0) {
                return -1; // Invalid file descriptor
        }

        int inode_id = fs->open_files[fd].inode_id;
        Inode *inode = &fs->inodes[inode_id - 1];

        if (inode->size == 0) {
                return 0; // End of file, no data to read
        }

        // For simplicity, reading directly from blocks (not handling fragmentation, etc.)
        // You will need to do this for each block of the file, in order.
        size_t bytes_to_read = size < inode->size ? size : inode->size;
        memcpy(buf, fs->block_device->blocks[inode->block_pointers[0]-1], bytes_to_read);

        fs->open_files[fd].current_offset += bytes_to_read;
        return bytes_to_read;
}

// Write to an open file
int write_file(Filesystem *fs, int fd, const void *buf, size_t size) {
        if (fd < 0 || fd >= MAX_FILES || fs->open_files[fd].inode_id == 0) {
                return -1; // Invalid file descriptor
        }

        /////// GET RID OF THIS ERROR WHEN ADDING MULTIPLE BLOCKS PER FILE
        if(size > BLOCK_SIZE){
                printf("Error: data exceeds current max file size (1 block)\n");
                return -1;
        }

        int inode_id = fs->open_files[fd].inode_id;
        Inode *inode = &fs->inodes[inode_id - 1];

        ////////// THIS CODE ASSUMES THE FILE IS ORIGINALLY EMPTY (JUST CREATED)
        ////////// add a section where it checks if block_pointer[0] is already being used, if it is, replace or add?, check the size then add block_pointer[1]
        // Find a free block and store the id in inode->block_pointers[0]
        if(inode->block_pointers[0] == 0){
                uint32_t newblock = 0;
                for(int i=0; i< NUM_BLOCKS; i++){
                        if(fs->block_device->block_status[i] == FREE){
                                fs->block_device->block_status[i] = USED;
                                fs->superblock.free_blocks -=1;
                                printf("Found free block with id=%d\n",i+1);
                                newblock = i+1;
                                break;
                        }
                }
                // If we didn't find a free block to use
                if(newblock == 0) return -1;

                // Use the block we found
                inode->block_pointers[0] = newblock;
        }

        // Clear the old contents
        memset(fs->block_device->blocks[inode->block_pointers[0]-1], 0, BLOCK_SIZE);
        // Write to the first (curently only) block
        memcpy(fs->block_device->blocks[inode->block_pointers[0]-1], buf, size);
        inode->size = size;

        return size;
}

// Simple directory creation (assuming inode is available for directories)
int mkdir(Filesystem *fs, const char *path) {
        if (fs->superblock.free_inodes == 0) {
                return -1; // No free inodes left
        }

        // Find a free inode
        int inode_id = -1;
        for (int i = 0; i < MAX_FILES; i++) {
                if (fs->inodes[i].inode_id == 0) {  // Free inode found
                        inode_id = i+1; // Can't be 0 since 0 is not a valid inode
                        break;
                }
        }

        if (inode_id == -1) {
                return -1; // No free inodes found
        }
        ///////////////////////////////////////////////////////
        char names[20][MAX_FILENAME_LEN] = {}; // Larger than necessary

        printf("Full path: %s\n",path);
        int num = splitname((char *)path, names);
        printf("**** Split path: ****\n");
        printf("Got %d names\n",num);
        for(int i=0; i<num; i++){
                printf("  %s\n",names[i]);
        }
        //////////////////////////////////////////////

        // Initialize the inode for the new file
        fs->inodes[inode_id-1].inode_id = inode_id;
        fs->inodes[inode_id-1].size = 0;
        strncpy(fs->inodes[inode_id-1].name, names[num-1], MAX_FILENAME_LEN);

        //printf("num: %d\n",num);

        fs->inodes[inode_id-1].file_type = IS_DIR; // IS NOW A DIRECTORY
        fs->superblock.free_inodes--;

        int free_block = -1;
        for(int i = 0; i < 512;i++){
                if (fs->block_device->block_status[i] == FREE){
                        free_block = i;
                        break;
                }
        }
        if (free_block == -1){ //WHAT DO WE DO WHEN THERE ARE NO MORE BLOCKS
                return -1;
        }
        fs->block_device->block_status[free_block] = USED;
        fs->inodes[inode_id-1].block_pointers[0] = free_block; // Note: Again, this means block index 0, not index 1
        fs->superblock.free_blocks -=1;

        if (num == 1){
                n = 0 // ROOT INODE POSITION
        }else{
                for (int i = 0; i < num; i++) { // VALIDATE IF PATH EXIST (if we make it exist, we need recursion)
                        if (fs->inodes[i].inode_id > 0) { // GET RID OF NUMBER OF FILES IN THE FIRST 4 BYTES (USE inode->size INSTEAD)
                                printf("Inode %d: File size = %d bytes File name: %s\n", fs->inodes[i].inode_id, fs->inodes[i].size, fs->inodes[i].name);

                }

        // ADD THE DIR INTO ROOT DIR
        unsigned blocknum = fs->inodes[0].block_pointers[0]; // REPLACE 1ST 0 WITH INODE NUMBER OF PREV DIR
        // See how many files existed here before
        unsigned num_prior_files = (fs->block_device->blocks[blocknum-1])[0];
        printf("Dir previously contained %d files\n",num_prior_files);

        // Each entry is a 32 bit id and the first one is 4 bytes in, since the first entry is a counter
        ((unsigned *) (fs->block_device->blocks[blocknum-1]))[1+num_prior_files] = inode_id;
        // Add one to the file count for this dir (cast is for 32-bit field)
        ((unsigned *) (fs->block_device->blocks[blocknum-1]))[0] += 1;

        printf("Directory '%s' created with inode %d\n", names[num-1], inode_id);
        return inode_id;
        //return create_file(fs,path);
}

// Remove a file or (empty) directory
int rm(Filesystem *fs, const char *filename) {
        // For simplicity, just remove the file
        return 0; // Implement file deletion logic here
}

// List all files
void list_files(Filesystem *fs) {
        // This is a placeholder that just prints the names of each in-use inode
        for (int i = 0; i < MAX_FILES; i++) {
                if (fs->inodes[i].inode_id > 0) {
                        printf("Inode %d: File size = %d bytes File name: %s\n", fs->inodes[i].inode_id, fs->inodes[i].size, fs->inodes[i].name);
                }
        }
}

/*
        *** Example of how to use splitname: ***

        printf("Testing splitname...\n");
        char names[20][MAX_FILENAME_LEN] = {}; // Larger than necessary

        char * path = "/foo/bar/baz.txt";
        printf("Full path: %s\n",path);
        int num = splitname(path, names);
        printf("**** Split path: ****\n");
        printf("Got %d names\n",num);
        for(int i=0; i<num; i++){
                printf("  %s\n",names[i]);
        }
*/

// Main function to run the example
int main() {
        Filesystem fs; // This contains everything else.
        init_filesystem(&fs);

        char buf[512];    // For file data
        char path[64];    // For names or paths
        char cmd[8];      // For menu commands
        int running = 1;  // To quit

        while (running) {
                printf(" c: create file\n o: open file\n cl: close file\n r: read file\n w: write file\n m: make directory\n rm: remove file or directory\n ls: list all files\n q: quit\n");
                printf("Command: ");
                if (!fgets(cmd, sizeof(cmd), stdin)) break; // If stdin closes, exit

                // Create a file
                if (cmd[0] == 'c' && cmd[1] == '\n') {
                        printf("Path: ");
                        fgets(path, sizeof(path), stdin);
                        path[strcspn(path, "\n")] = 0;
                        int inode = create_file(&fs, path);
                        if (inode == -1) printf("Error\n");
                        else printf("File created with inode %d\n", inode);

                // Open a file
                } else if (cmd[0] == 'o' && cmd[1] == '\n') {
                        printf("Path: ");
                        fgets(path, sizeof(path), stdin);
                        path[strcspn(path, "\n")] = 0;
                        int fd = open_file(&fs, path);
                        if (fd == -1) printf("Error\n");
                        else printf("opened file with descriptor %d\n", fd);

                // Read a file
                } else if (cmd[0] == 'r' && cmd[1] == '\n') {
                        int fd;
                        printf("FD: ");
                        scanf("%d", &fd);
                        fgets(path, sizeof(path), stdin); // consume newline
                        int n = read_file(&fs, fd, buf, sizeof(buf));
                        if (n == -1) printf("Error\n");
                        else {
                                printf("%.*s", n, buf); // Safe print of up to n bytes
                                printf("\n");
                        }

                // Write to a file
                } else if (cmd[0] == 'w' && cmd[1] == '\n') {
                        int fd;
                        printf("FD: ");
                        scanf("%d", &fd);
                        fgets(path, sizeof(path), stdin); // consume newline
                        printf("Data: ");
                        fgets(buf, sizeof(buf), stdin);
                        size_t len = strcspn(buf, "\n");
                        buf[len] = 0;
                        int n = write_file(&fs, fd, buf, len);
                        if (n == -1) printf("Error\n");
                        else printf("%d bytes written to fd %d \n", n, fd);

                // close a file
                } else if (cmd[0] == 'c' && cmd[1] == 'l') {
                        int fd;
                        printf("FD: ");
                        scanf("%d", &fd);
                        fgets(path, sizeof(path), stdin); // consume newline
                        int n = close_file(&fs, fd);
                        if (n == -1) printf("Error\n");
                        else printf("Closed.\n");

                // Make directory
                } else if (cmd[0] == 'm' && cmd[1] == '\n') {
                        printf("Path: ");
                        fgets(path, sizeof(path), stdin);
                        path[strcspn(path, "\n")] = 0;
                        int r = mkdir(&fs, path);
                        if (r == -1) printf("Error\n");
                        else printf("Success\n");

                // Remove file or (empty) directory
                } else if (cmd[0] == 'r' && cmd[1] == 'm') {
                        printf("Path: ");
                        fgets(path, sizeof(path), stdin);
                        path[strcspn(path, "\n")] = 0;
                        int r = rm(&fs, path);
                        if (r == -1) printf("Error\n");
                        else printf("Success\n");

                // List files
                } else if (cmd[0] == 'l' && cmd[1] == 's') { list_files(&fs);

                // Quit
                } else if (cmd[0] == 'q') { running = 0; }
        } // End while


        return 0;
}