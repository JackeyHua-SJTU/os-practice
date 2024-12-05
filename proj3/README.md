# Mini File System
This is the course project for CS2303 Project Workshop of Operating System.

> I am preparing to migrate the codebase to **Rust/GoLang** for more practices.

## Description
This is a file system comprised of 3 parts, disk backend, file system server and file system clients. The core features are as follows:
- Clients connect to the server via Socket, and the server connects to the disk backend via Socket.
- Clients can set their own file/directory permissions.
- Clients can create, delete, read, write, and list files/directories.
- Persistence is implemented for the file system server.
- Concurrency control is implemented for multiple clients to access the file system server at the same time.

## Usage 
Check `typescript.md` for how to compile/run/use the project.
