/*
* File: async-iocp.c
* Created: 30.10.2024
* Description: Shows how you utilize Overlapped IO to observe different IO operations,
*              in this case, file reading. Different Windows versions act differently.
*              This file can be run where Overlapped IO is present, most likely, in Windows 9x and NT.
*              The file will get compiled for almost all Windows versions from 9x up to now,
*              however, reading files in asyncrhonous manner, might be available only in latter
*              Windows versions, meaning a segmentation fault may appear in systems like Windows 98.
*
*
* Last Modified By: Ivan Yonkov <ivan.yonkov@codexio.bg>
* Last Modified Date: 01.11.2024
*
* License: Apache License 2.0
*/

#define WINVER 0x0400
#define _WIN32_WINNT 0x0400
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#define BUFFER_SIZE 8
#define NUM_FILES 3

typedef struct {
    HANDLE hFile;
    OVERLAPPED ol;
    char* fileBuffer;
    BOOL isComplete;
    DWORD totalBytesRead;
    DWORD fileSize;
} FileContext;

char* AllocateBuffer(DWORD size)
{
    DWORD i;
    char* buffer = (char*)malloc(size + 1);
    for (i = 0; i < size + 1; i++)
    {
        buffer[i] = '\0';
    }
    return buffer;
}

void InitFileContext(FileContext* ctx, const char* fileName)
{
    int i;
    ctx->hFile = CreateFileA(fileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);

    for (i = 0; i < sizeof(OVERLAPPED); i++)
    {
        ((char*)&(ctx->ol))[i] = 0;
    }

    ctx->totalBytesRead = 0;
    ctx->isComplete = FALSE;

    ctx->fileSize = GetFileSize(ctx->hFile, NULL);

    ctx->fileBuffer = AllocateBuffer(ctx->fileSize);
}

void ReadFileAsync(FileContext* ctx)
{
    DWORD bytesRead = 0;
    ReadFile(ctx->hFile, ctx->fileBuffer + ctx->totalBytesRead, BUFFER_SIZE, &bytesRead, &(ctx->ol));

    ctx->totalBytesRead += bytesRead;

    if (ctx->totalBytesRead >= ctx->fileSize)
    {
        ctx->isComplete = TRUE;
    }
}

void CheckForCompletion(FileContext* ctx)
{
    DWORD bytesTransferred = 0;

    if (GetOverlappedResult(ctx->hFile, &(ctx->ol), &bytesTransferred, FALSE))
    {
        ctx->totalBytesRead += bytesTransferred;

        if (ctx->totalBytesRead < ctx->fileSize)
        {
            ctx->ol.Offset += bytesTransferred;
            ReadFileAsync(ctx);
        }
        else
        {
            ctx->isComplete = TRUE;
        }
    }
}

int main()
{
    int i;
    BOOL allComplete;
    const char* fileNames[NUM_FILES] = {
        "C:\\file1.txt",
        "C:\\file2.txt",
        "C:\\file3.txt"
    };

    FileContext files[NUM_FILES];

    for (i = 0; i < NUM_FILES; ++i)
    {
        InitFileContext(&files[i], fileNames[i]);
        ReadFileAsync(&files[i]);
    }

    allComplete = FALSE;
    while (!allComplete)
    {
        allComplete = TRUE;
        for (i = 0; i < NUM_FILES; ++i)
        {
            if (!files[i].isComplete)
            {
                CheckForCompletion(&files[i]);
                allComplete = FALSE;
            }
        }
        Sleep(10);
    }

    for (i = 0; i < NUM_FILES; ++i)
    {
        if (files[i].isComplete)
        {
            printf("%s\n\n", files[i].fileBuffer);
        }
    }

    for (i = 0; i < NUM_FILES; ++i)
    {
        CloseHandle(files[i].hFile);
        free(files[i].fileBuffer);
    }

    return 0;
}
