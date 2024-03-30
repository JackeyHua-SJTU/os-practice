# Type script of the project

## Compile
If you are at the current directory `proj1/`, you can compile via `make MACRO=TIME1` or `make MACRO=TIME2` to compile the project with the `TIME1` or `TIME2` macro defined respectively. `TIME1` and `TIME2` defines different time macros for copy project.

If you are at `proj1/shell` or `proj1/sort`, you can just call `make` to compile.

If you are at `proj1/copy`, you can call `make MACRO=TIME1` or `make MACRO=TIME2` to compile.

## Copy
### Usage
```bash
./MyCopy <InputFile> <OutputFile>
./PipeCopy <InputFile> <OutputFile>
./ForkCopy <InputFile> <OutputFile>
```
The purpose of the above 3 executables is to copy the contents of the `InputFile` to the `OutputFile`. If the `OutputFile` does not exist, it will be created. If the `OutputFile` already exists, it will be overwritten.

If successful, the program will output the following message.
```bash
My Copy takes : 0.112000 ms 
Pipe Copy takes : 0.112000 ms
Fork Copy takes : 0.112000 ms
```

You can also call `python3 test.py` to automatically run test for pre-defined configuration. It tests three copy strategy at same file size, and after execution, two figure `plot1.png` and `plot2.png` will be generated. The former one is the chart for `TIME1` strategy and the latter one is for `TIME2` strategy.

### Error Handling
- Wrong number of arguments. 
    - If the arguments is less than 3, which means the user did not provide enough arguments, the program will output the following message.
    ```bash
    Missing parameters. 
    ```
    - If the arguments is more than 3, which means the user provided too many arguments, the program will output the following message.
    ```bash
    Too many parameters. 
    ```
- Fail to create a child process.
    ```bash
    Fork failed.
    ```
- Fail to open source file.
    ```bash
    Error opening source file
    ```
- Fail to open destination file.
    ```bash
    Error opening destination file
    ```
- Fail to create a pipe.
    ```bash
    Pipe failed.
    ```
- Child process exit with error in `ForkCopy.c`.
    ```bash
    Error copying
    ```

## Shell
### Usage
- Server side. Run the program with a port number.
    ```bash
    ./server <port>
    ```
    As soon as a client connects to the server, it will output the following message.
    ```bash
    New child process (seq #<seqno>)
    ```
- Client side. After the server is running, use telnet to connect to a server (port).
    ```bash
    telnet localhost <port>
    ```
    - If successful connected, you can use the following outputs. The last row mimics what real bash is, `username@hostname:current_directory$`.
    ```bash
    Trying 127.0.0.1...
    Connected to localhost.
    Escape character is '^]'.
    parallels@ubuntu:/home/parallels/Desktop/cs2303/proj1/shell$
    ```
#### Run commands
My tiny shell has implemented the following commands. **Any of commands not listed below will call Linux bin to execute.**
- `ls`
    - Support default one. `ls`
    - Support `-l` and `-a` options. `ls -l` or `ls -a`
    - Support particular path. `ls <path>`
- `pwd`
    - Run with `pwd`, which prints the absolute path of current directory.
- `cat`
    - Run with `cat <file>`, which prints the content of the file.
- `wc`
    - Pair with pipe.
- `exit`
    - Exit the shell in the client side.
    - ***Use Ctrl+C to exit the server side.***
- `rm`
    - Run with `rm <file>`, which removes the file.
    - Does not support `-r` option.
- `cd`
    - Support default one `cd`, which switches to `~` directory.
    - Support particular path. `cd <path>`
- `pipe`
    - Redirect I/O. E.g. `ls | wc`

#### Special note
- At first, i implement a pure local shell, which does not support socket communication. You can find the source code in `shell.c`. Then i migrate the code to `shell_server.c` and add socket communication. Both can run.
- The server side will not exit **UNTIL** you press `Ctrl+C`. The client side will exit if you type `exit`.
- I search the web for the first line of `ls -l` in Linux original shell (for example, total 128). It is said that the number is the total blocks, but when i calculate the sum of blocks via C API, it does not correspond to the number. So i just print the number directly via Linux shell.
- It is said that the last column of `wc` is the byte counts of the file. But it is wield that `ls -l | wc | wc` and `ls -l | wc | wc | wc` will output the same result `     1       3      24`. I can not figure out why the latter one has 24 byte counts. I just simply count the byte of the input/file.
- The port can be reused. If the previous server used port 8080, the next server can still use port 8080.

### Error Handling
In this project, i mainly use `perror` to print the error message. So i just list the error that will be thrown into `perror`.
- `fork`: in forking a child process
- `pipe`: in creating pipe to communicate
- `execvp`: in executing a command
- `socket`: in creating a socket
- `bind`: in binding a socket
- `listen`: in listening a socket
- `accept`: in accepting a connection
- `setsockopt`: in setting socket options
- `cd`: in changing directory

And there are several manually handled errors.
- If the server is called with `argc != 2`, it will print the following message.
    ```bash
    Usage: ./server <port>
    ```
- If the major execution funciton `void exec(char** argv, int lb, int rb, int clientfd)` is called with `lb > rb`, which will be reached if there is no arguments after a pipe, then it will print the following message.
    ```bash
    Invalid parameters.
    ```

## Sort
### Usage
You should write the size of array and the array in `data.in`, and then run the following commands.
```bash
./MergesortSingle < data.in > data.out
./MergesortMulti < data.in > data.out
```
It redirects the input from `data.in` and output to `data.out`. If successful, the program will output the sorted array in ascending order to `data.out`.

To quantify the performance of the two sorting algorithms, you can call `python3 script.py` to automatically run test for pre-defined configuration. It tests two sorting algorithms at different array size, and after execution, `plot.png` will be generated.

### Error Handling
- Error when create a thread.
    ```bash
    Error: return code from pthread_create() is rc
    ```
- Error when join a thread.
    ```bash
    Error: return code from pthread_join() is rc
    ```