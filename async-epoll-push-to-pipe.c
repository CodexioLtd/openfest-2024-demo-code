/*
* File: async-epoll-push-to-pipe.c
* Created: 30.10.2024
* Description: Shows how you can push to a named pipe, so the consuming application
*              will receive the message.
*              This file can be run where UNISTD is present, most likely, in Unix systems
*
* Last Modified By: Ivan Yonkov <ivan.yonkov@codexio.bg>
* Last Modified Date: 01.11.2024
*
* License: Apache License 2.0
*/
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(void) {
    // you may need to create this manually beforehand
    // mkfifo /tmp/openfest-pipe
    // chmod 666 /tmp/openfest-pipe
    int fd = open("/tmp/openfest-pipe", O_WRONLY);

    char *greeting;

    while (1) {
        printf("Say something: ");
        scanf("%ms", &greeting);

        if (strcmp(greeting, "STOP") == 0) {
            printf("Oh, you stopped me:(\n");
            exit(0);
        }

        strcat(greeting, "\n");
        write(fd, greeting, strlen(greeting));
        printf("Success!\n");
        printf("=========\n");
    }
}