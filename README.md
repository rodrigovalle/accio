# CS118 Project 1

Rodrigo Valle, 104 494 120

## Design
Several different designs were explored for the server portion of this project,
the first being an event-driven design based on Linux's epoll and timerfd.
epoll would listen to a set of sockets representing active client connections
and return when either a socket was ready for reading, or the timerfd object
for a connection expired. If a socket was ready for reading, we would read from
the socket, perform a blocking write() to its corresponding file (nonblocking
writes would require a more fully fledged event loop implementation --
potentially out of this project's scope), and reset the socket's corresponding
timerfd object for another 10 seconds. If a timerfd object has expired, we
would identify which connection the timerfd belonged to and close that
connection.

A simpler approach was later developed, which uses a 1:1 threading model (where
there is 1 kernel thread for every client connection) and blocking sockets with
the corresponding SO_SNDTIMEO and SO_RCVTIMEO options set so the OS can handle
the 10 second timeouts for us. For the most part, this design is simple but the
difficulties arise from potentially shoddy implementations of the SO_SNDTIMEO
and SO_RCVTIMEO options and the handling threads and exceptions. Currently,
threads are "handled" by creating and detaching a thread for every client. On
exit we pray that the threads weren't in the middle of anything important. Each
thread will be numbered by the main thread, and open up a file for the client
corresponding to that number. Then this thread begins receiving data from the
client and writing it out to disk until:

 1. The timeout is reached (replace file with error message and exit thread)
 2. The client disconnects (assume they were done and exit thread)

## Issues
Use of the C language's exit() function will terminate the program immediately,
without cleaning up any C++ objects. Because of this, its use is marginalized
in C++ code; we want any open files to be flushed to disk after all. This issue
was overcome by throwing an exception on error and catching it in main(),
returning from main with an exit code of 1 so as to allow objects to destroy
themselves before program termination.

Sending a signal to the server currently causes the entire process to die
immediately, without any cleanup. This is because exceptions arising from other
threads are difficult to coordinate (there is a thread dedicated to signal
handling, the main thread is dedicated to accepting connections).

Let's talk about a strange SO_SNDTIMEO bug; this option is set in the client so
that the connection to the server will either succeed, or timeout in 10
seconds. Unfortunately, setting SO_SNDTIMEO causes the client to timeout in 20
seconds (I measured it, with a stopwatch and everything). Setting the timeout
to 5 causes it to timeout in 10 seconds. A strange bug for sure, but I found a
workaround.

The real trouble comes from handling multiple threads, which are notoriously
difficult to coordinate once you spawn them. Threads don't work well with
signals, which our server is required to catch, and they don't work well with
exceptions, which allow us to safely unwind the stack. This is a bit of a
headache since without implementing your own threadpool, the best you can do is
simply detach() all threads and on exit, kill everything and hope the OS isn't
too unhappy with having to clean up after you (and the clients don't mind
losing their connections mid-write).

## Additional Libraries
No additional libraries were used. Boost is huge...

## Notes
### Where are all the merge commits, didn't you use feature branches?
I avoided a lot of them through gratuitous use of fast-foward merges and
rebasing.

### How could this project be improved?
I could introduce a class hierarchy instead of relying on friend classes; that
is, ListeningSocket and ConnectedSocket should use FileDescriptor objects to
keep track of their socket file descriptors, instead of handling them on their
own.

I could also implement a threadpool to handle each client connection and avoid
the case where a massive number of clients causes problems for the OS because
currently we scale at 1:1. This threadpool would take tasks in the form of
tuples of functions and their arguments, which sit in a queue until a thread
signals that it's ready. There would also be a proper destructor for the object
which gives threads the chance to clean up before exiting.

Lastly, if we're really going for scalable, we could use epoll() (or kqueue()
if we're targeting BSD; libuv/libev if we want platform indpendence) to
implement an event-based system which processes ready events on connected
sockets and files that are being written to so we only service them when the
corresponding system calls would not block. This event loop could be
replicated across multiple processes, and the evented design would make sure we
make the most out of each process. To my knowledge, NGINX uses this design.

## Docker
To get started, run
``` bash
# build the docker image from the Dockerfile
$ docker build . -t accio

# run in a container whose port 3000 is mapped to the host's port 3000
$ docker run --name accio-container -d -p 3000:3000 accio
```

Now we can open another terminal and start sending files to the server by
connecting to the host's port 3000 (e.g. `./client 3000 file.txt` or
`telnet localhost 3000`). The server will store them locally inside the
container. If we want to see these files, we can run

``` bash
$ docker exec -it accio-container /bin/bash
```
to get a shell inside the running container and `cat` our files. Next, run

``` bash
$ docker stop accio-container
```
to stop the container.

## Acknowledgements
[cplusplus.com](http://www.cplusplus.com/reference)  
[C++ Reference](http://en.cppreference.com/w/cpp)  
[How to end C++ code](http://stackoverflow.com/questions/30250934)  
[Beej's Guide to Network Programming](http://beej.us/guide/bgnet)  
[AOSA: Nginx](http://www.aosabook.org/en/nginx.html)  
Linux Man Pages: I used these extensively and drew some inpsiration and best
practices from code examples when they were given.
