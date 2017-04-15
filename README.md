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

## Acknowledgements

[cplusplus.com](http://www.cplusplus.com/reference)  
[C++ Reference](http://en.cppreference.com/w/cpp)  
[How to end C++ code](http://stackoverflow.com/questions/30250934)  
[Beej's Guide to Network Programming](http://beej.us/guide/bgnet)  
[AOSA: Nginx](http://www.aosabook.org/en/nginx.html)

`socat - tcp-connect:<HOST>:<PORT>`
