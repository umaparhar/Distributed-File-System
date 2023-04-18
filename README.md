# Distributed-File-System
Developed a working distributed file server.

File server is built as a stand-alone UDP-based server. It waits for a message and then processes the message as need be, replying to the given client.

The file server stores all of its data in an on-disk file which is referred to as the file system image. This image contains the on-disk representation of the data structures; I used these system calls to access it: open(), read(), write(), lseek(), close(), fsync().

To access the file server, I built a client library. The interface that the library supports is defined in mfs.h. This library is called libmfs.so, and any programs that wish to access my file server link with it and call its various routines.

