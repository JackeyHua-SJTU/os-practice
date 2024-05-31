# Project 3 Readme
Check my [Gtihub page](https://github.com/JackeyHua-SJTU/os-practice/tree/main/proj3) for my full dev log.

## Structure
```bash
.
├── Makefile                # Makefile for the whole project
├── Prj3README.md           # Project 3 README File
├── step1                   # Disc Server Part
│   ├── include
│   │   └── disc.h          # Header file for disc struct
│   ├── makefile            # Makefile for step1
│   └── src
│       ├── BDC_CLI.c       # Basic Disc Client Command line interface version main file
│       ├── BDC_RAND.c      # Basic Disc Client Randomly Generated version main file
│       ├── BDS.c           # Basic Disc Server main file
│       └── disc.c          # Source file for disc struct
├── step2
│   ├── disc                # Disc Server Part
│   │   ├── include
│   │   │   └── disc.h      # Header file for disc struct
│   │   ├── makefile        # Makefile for the disc subdirectory
│   │   └── src 
│   │       ├── BDC_CLI.c   # Not used
│   │       ├── BDC_RAND.c  # Not used
│   │       ├── BDS.c       # Not used
│   │       └── disc.c      # Source file for disc struct
│   ├── include
│   │   ├── entry.h         # Header file for entry struct
│   │   ├── inode.h         # Header file for inode struct
│   │   ├── superblock.h    # Header file for superblock struct
│   │   └── wrapper.h       # Header file for wrapper functions
│   ├── makefile            # Makefile for step2
│   └── src
│       ├── FC.c            # File System Client main file
│       ├── FS.c            # File System Server main file
│       ├── inode.c         # Source file for inode struct
│       ├── superblock.c    # Source file for superblock struct
│       └── wrapper.c       # Source file for wrapper functions
├── step3
│   ├── disc                # Disc Server Part
│   │   ├── include
│   │   │   └── disc.h      # Header file for disc struct
│   │   ├── makefile
│   │   └── src
│   │       ├── BDC_CLI.c   # Not used
│   │       ├── BDC_RAND.c  # Not used
│   │       ├── BDS.c       # Not used
│   │       └── disc.c      # Source file for disc struct
│   ├── include
│   │   ├── entry.h         # Header file for entry struct
│   │   ├── inode.h         # Header file for inode struct
│   │   ├── superblock.h    # Header file for superblock struct
│   │   ├── user.h          # Header file for user struct
│   │   └── wrapper.h       # Header file for wrapper functions
│   ├── makefile            # Makefile for step3
│   └── src
│       ├── FC.c            # File System Client main file
│       ├── FS.c            # File System Server main file
│       ├── inode.c         # Source file for inode struct
│       ├── superblock.c    # Source file for superblock struct
│       └── wrapper.c       # Source file for wrapper functions
└── typescript.md           # Typescript of Project 3
```