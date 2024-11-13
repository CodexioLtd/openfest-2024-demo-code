/*
* File: async-timers.c
* Created: 30.10.2024
* Description: Shows how you utilize Timers with Extended Windows which will activate the internal
*              Message Queue mechanism in Windows. It also shows how to poll messages when they are
*              ready and execute an appropriate callback upon task readiness.
*              in this case, file reading. Different Windows versions act differently.
*              This file can be run where Timers and Extended Windows are present,
*              most likely, in all Windows systems from 9x up to now.
*
*
* Last Modified By: Ivan Yonkov <ivan.yonkov@codexio.bg>
* Last Modified Date: 01.11.2024
*
* License: Apache License 2.0
*/

#include <windows.h>
#include <stdio.h>

struct TimerInfo {
    int timerId;
    int seconds;
    void (*callback)(char*);
    char* callbackArgs;
};

struct TimerInfo* timers;
int timersLeft;

void Print(char* message)
{
    printf("%s\n", message);
}

LRESULT CALLBACK TimerWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_TIMER)
    {
        int timerIndex = (int)wParam - 1;
        if (timerIndex >= 0 && timerIndex < sizeof(timers))
        {
            timers[timerIndex].callback(timers[timerIndex].callbackArgs);
            KillTimer(hwnd, timers[timerIndex].timerId);

            if (--timersLeft <= 0)
            {
                PostQuitMessage(0);
            }
        }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int main() {
    int timersCount;
    WNDCLASS wc = { 0 };
    HWND hwnd;
    MSG msg;
    int i;
    int currentTimerSeconds;
    char currentTimerCallbackMessage[256];

    printf("How many timers: ");
    scanf("%d", &timersCount);
    printf("\n");

    timersLeft = timersCount;
    timers = malloc(timersCount * sizeof(struct TimerInfo));

    wc.lpfnWndProc = TimerWindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = "TimerClass";
    RegisterClass(&wc);

    hwnd = CreateWindowEx(0, "TimerClass", NULL, 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, GetModuleHandle(NULL), NULL);

    for (i = 0; i < timersCount; i++)
    {
        printf("Scheduling timer: %d\n", i + 1);
        printf("How many seconds to wait for this timer: ");
        scanf("%d", &currentTimerSeconds);
        getchar();

        printf("What do you want to print after the timer elapses: ");
        scanf("%s", currentTimerCallbackMessage);

        timers[i].timerId = i + 1;
        timers[i].seconds = currentTimerSeconds;
        timers[i].callback = &Print;
        timers[i].callbackArgs = malloc(strlen(currentTimerCallbackMessage) + 1);
        strcpy(timers[i].callbackArgs, currentTimerCallbackMessage);

        printf("\n");
    }

    for (i = 0; i < timersCount; i++)
    {
        SetTimer(hwnd, timers[i].timerId, timers[i].seconds * 1000, NULL);
    }


    printf("Waiting for events...\n\n");
    while (GetMessage(&msg, NULL, 0, 0))
    {
        DispatchMessage(&msg);
    }

    printf("\nAll events completed.\n");
    return 0;
}
