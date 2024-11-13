# OpenFest Bulgaria 2024 Demo

This project contains the source code that was once demoed at [The Great Asynchronism](https://www.openfest.org/2024/en/full-schedule-en/#lecture-898)
session at OpenFest Bulgaria 2024 Conference.

It focuses on how to implement a very simple version of an "event loop" in different operating systems.

- `async-epoll*.c` - these files focus on how to observe in async fashion some file descriptors in Linux
- `async-iocp.c` - this one shows how to do almost the same thing as above, but in modern Windows
- `async-overlapped.c` - shows how to do the same as in IOCP, with less modern APIs, still for more modern Windows
- `async-timers.c` - shows a classic implementation of an event loop with `GetMessage` and `DispatchMessage` which
works for almost all versions of Windows from 9x up to 11