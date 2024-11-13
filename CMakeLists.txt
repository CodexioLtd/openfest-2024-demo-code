cmake_minimum_required(VERSION 3.29)
project(openfest2024)

set(CMAKE_CXX_STANDARD 20)

add_executable(async-epoll async-epoll.c)
add_executable(async-epoll-push-to-pipe async-epoll-push-to-pipe.c)
add_executable(async-iocp async-iocp.c)
add_executable(async-overlapped async-overlapped.c)
add_executable(async-timers async-timers.c)
