# CS118 Project 1

Rodrigo Valle, 104 494 120

## Design

## Issues
Use of the C language's exit() function will terminate the program immediately,
without cleaning up any C++ objects. Because of this, its use is marginalized
in C++ code; we want any open files to be flushed to disk after all. This issue
was overcome by throwing an exception on error and catching it in main(),
returning from main with an exit code of 1 so as to allow objects to destroy
themselves before program termination.

## Additional Libraries

## Notes
### Where are all the merge commits, didn't you use feature branches?
I avoided a lot of them through gratuitous use of fast-foward merges and
rebasing.

### How could this project be improved?
I could introduce a class hierarchy instead of relying on friend classes; that
is, ListeningSocket and ConnectedSocket should use FileDescriptor objects to
keep track of their socket file descriptors, instead of handling them on their
own.

## Acknowledgements

[cplusplus.com](http://www.cplusplus.com/reference)  
[C++ Reference](http://en.cppreference.com/w/cpp)  
[How to end C++ code](http://stackoverflow.com/questions/30250934)  
[Beej's Guide to Network Programming](http://beej.us/guide/bgnet)  
[AOSA: Nginx](http://www.aosabook.org/en/nginx.html)

`socat - tcp-connect:<HOST>:<PORT>`
