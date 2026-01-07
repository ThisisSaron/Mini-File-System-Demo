# Mini File System Implementation (C)

## Overview
This project is a **mini file system** implemented in **C** as part of an **Operating Systems** course. The goal was to design and implement core file system functionality while working within a constrained starter framework, similar to how real-world systems interact with existing kernels or APIs. A minimal skeleton was provided for setup and testing (including sections marked "do not modify").

---

## Project Goals
- Understand low-level file system design
- Implement persistent storage abstractions
- Work directly with memory, blocks, and metadata
- Gain experience writing systems-level C code under strict constraints

---

## Features Implemented
- File and directory representation
- Path parsing and name resolution
- Inode and block management
- File creation and deletion
- Read and write operations
- Error handling for invalid paths and operations

---

## File System Design

### Storage Model
- Uses a **simulated block device** to represent disk storage
- Data is divided into fixed-size blocks
- Metadata and file contents are stored separately

### Metadata Management
- Files and directories are represented using **inode-like structures**
- Inodes store metadata such as size, type, and block pointers
- Directory entries map filenames to inode numbers

### Block Allocation
- Tracks free and used blocks
- Allocates blocks dynamically during file writes
- Ensures proper cleanup during file deletion

---

## Supported Operations
- Create files and directories
- Delete files and directories
- Read file contents
- Write data to files
- Traverse directory paths

These operations mirror the responsibilities of a real file system at a simplified scale.

---

## Constraints & Design Considerations
- Required to integrate with a provided framework
- Certain sections of code were restricted from modification
- Emphasis on correctness, memory safety, and robustness
- Designed to function without relying on external libraries beyond the C standard library

---

## Technologies Used
- **C** – systems-level programming
- **Linux / Command Line** – development and testing environment
- **Operating Systems concepts** – file systems, memory management, and abstraction layers

---

## What This Project Demonstrates
- Understanding of core operating systems concepts
- Ability to work at a low level with memory and data structures
- Experience implementing complex logic within existing constraints
- Strong debugging and problem-solving skills in C

---

## Future Improvements
- Support for file permissions
- Improved directory traversal performance
- Journaling or crash recovery mechanisms
- Enhanced scalability for larger simulated disks

---

## License
This project was developed for academic purposes as part of an Operating Systems course.


