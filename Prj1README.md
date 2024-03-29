# Project 1 

## Structure
```bash
.
├── Makefile                # Makefile for the whole project.
├── Prj1README.md           # README file for the project.
├── copy
│   ├── ForkCopy.c          # Fork a child and run MyCopy
│   ├── Makefile            # Makefile for the copy subdirectory.
│   ├── MyCopy.c            # Copy file using C API.
│   ├── PipeCopy.c          # Copy file using pipe.
│   ├── dest.txt            # Destination file for testing.
│   ├── plot1.png           # Plot of performance test of 3 strategies for TIME1.
│   ├── plot2.png           # Plot of performance test of 3 strategies for TIME2.
│   ├── src.txt             # Source file for testing.
│   └── test.py             # Python script for testing.
├── shell
│   ├── Makefile            # Makefile for the shell subdirectory.
│   ├── cat.c               # Personal cat
│   ├── ls.c                # Personal ls
│   ├── pwd.c               # Personal pwd
│   ├── rm.c                # Personal rm
│   ├── shell.c             # local version of shell, do not support socket
│   ├── shell_server.c      # server version of shell, support socket
│   ├── test.txt            # Test cat command
│   └── wc.c                # Personal wc
├── sort
│   ├── Makefile            # Makefile for the sort subdirectory.
│   ├── MergesortMulti.c    # Multi-threaded merge sort
│   ├── MergesortSingle.c   # Single-threaded merge sort
│   ├── data.in             # Input file for testing.
│   ├── data.out            # Output file for testing.
│   ├── plot.png            # Plot of performance test.
│   └── script.py           # Python script for testing.
└── typescript.md
```