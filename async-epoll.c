/*
* File: async-epoll.c
* Created: 30.10.2024
* Description: Shows how you can utilize epoll to add different file descriptors
*              such as from pipes and timers and wait for their execution to happen.
*              This file can be run where EPOLL is present, most likely, in Linux.
*
* Last Modified By: Ivan Yonkov <ivan.yonkov@codexio.bg>
* Last Modified Date: 01.11.2024
*
* License: Apache License 2.0
*/
#include <sys/epoll.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/timerfd.h>
#include <sys/stat.h>

enum fd_type {
    PIPE,
    TIMER
};

struct fd_info {
    int fd;
    enum fd_type type;

};

int observed_fds_capacity = 8;
struct fd_info *observed_fds = NULL;
int observed_fds_size = 0;
int epoll_fd = -1;
struct epoll_event event;

void observe_fd(const int fd, const enum fd_type type) {
    printf("Pushing %d to epoll\n", fd);

    if (observed_fds_size >= observed_fds_capacity) {
        observed_fds_capacity *= 2;
        observed_fds = realloc(observed_fds, observed_fds_capacity * sizeof(struct fd_info));
    }

    observed_fds[observed_fds_size].fd = fd;
    observed_fds[observed_fds_size].type = type;
    event.data.ptr = &observed_fds[observed_fds_size];
    event.events = EPOLLIN | EPOLLET;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event);

    observed_fds_size++;
}

void repeat_seconds(int seconds, int timer_fd) {
    struct itimerspec spec;
    spec.it_value.tv_sec = seconds;
    spec.it_value.tv_nsec = 0;
    spec.it_interval.tv_sec = seconds;
    spec.it_interval.tv_nsec = 0;
    timerfd_settime(timer_fd, 0, &spec, NULL);
}

void set_nonblocking(int fd) {
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
}

unsigned long long times_fired = 0;

int main() {
    observed_fds = malloc(observed_fds_capacity * sizeof(struct fd_info));
    epoll_fd = epoll_create1(0);
    int timer_fd = timerfd_create(CLOCK_MONOTONIC, 0);
    repeat_seconds(2, timer_fd);
    set_nonblocking(timer_fd);
    observe_fd(timer_fd, TIMER);

    int anonymous_pipe_fds[2];
    pipe(anonymous_pipe_fds);
    set_nonblocking(anonymous_pipe_fds[0]);
    observe_fd(anonymous_pipe_fds[0], PIPE);

    int named_pipe_fd = open("/tmp/openfest-pipe", O_RDONLY | O_NONBLOCK);
    set_nonblocking(named_pipe_fd);
    observe_fd(named_pipe_fd, PIPE);

    const char *to_write = "Something to write before the loop.\n";
    write(anonymous_pipe_fds[1], to_write, strlen(to_write));

    struct epoll_event events[10];

    printf("Starting the event loop\n");

    while (1) {
        int n = epoll_wait(epoll_fd, events, 10, -1);
        for (int i = 0; i < n; i++) {
            struct fd_info *info = events[i].data.ptr;

            if (info->type == PIPE) {
                char buffer[1024];
                int bytes_read;
                while ((bytes_read = read(info->fd, buffer, sizeof(buffer) - 1)) > 0) {
                    buffer[bytes_read] = '\0';
                    printf("Read from pipe: %s", buffer);
                }
            } else if (info->type == TIMER) {
                //sleep(rand() % 10);
                uint64_t expirations;
                read(info->fd, &expirations, sizeof(expirations));
                const unsigned long long current_iter_times = expirations;
                times_fired += current_iter_times;
                printf("Current iteration timer fired %llu times to a total of %llu\n", current_iter_times, times_fired);

                if (times_fired > 0 && times_fired % 5 == 0) {
                    to_write = "You will get this every 5 timer ticks (~10 seconds)\n";
                    write(anonymous_pipe_fds[1], to_write, strlen(to_write));
                }
            }
        }
    }

    return 0;
}