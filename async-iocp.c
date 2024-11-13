/*
* File: async-iocp.c
* Created: 30.10.2024
* Description: Shows how you utilize I/O Completion Ports (IOCP) to observe different
*              IO operations, in this case, file reading.
*              This file can be run where IOCP is present, most likely, in Windows NT 3.5+.
*
* Last Modified By: Ivan Yonkov <ivan.yonkov@codexio.bg>
* Last Modified Date: 01.11.2024
*
* License: Apache License 2.0
*/

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#define BUFFER_SIZE 8
#define NUM_FILES 3

typedef struct {
    HANDLE hFile;
    OVERLAPPED ol;
    char* fileBuffer;
    DWORD fileSize;
    DWORD bytesRead;
} FileContext;

FileContext files[NUM_FILES];
HANDLE hIOCP;

int ReadAsync(FileContext* ctx)
{
    return ReadFile(ctx->hFile, ctx->fileBuffer + ctx->bytesRead, BUFFER_SIZE, NULL, &(ctx->ol));
}

void InitFileContext(FileContext* ctx, const wchar_t* fileName)
{
    ctx->hFile = CreateFileW(fileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);

    ctx->fileSize = GetFileSize(ctx->hFile, NULL);
    ctx->bytesRead = 0;

    ctx->fileBuffer = (char*)malloc(ctx->fileSize + 1);

    ZeroMemory(&(ctx->ol), sizeof(OVERLAPPED));

    CreateIoCompletionPort(ctx->hFile, hIOCP, (ULONG_PTR)ctx, 0);

    ReadAsync(ctx);
}

void ContinueReading(FileContext* ctx)
{
    ctx->ol.Offset += BUFFER_SIZE;

    if (ctx->bytesRead < ctx->fileSize)
    {
        ReadAsync(ctx);
    }
}

int main()
{
    hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

    const wchar_t* fileNames[NUM_FILES] =
    {
        L"\\\\?\\C:\\Users\\User\\Documents\\Dev\\Async\\AsyncDemo\\file1.txt",
        L"\\\\?\\C:\\Users\\User\\Documents\\Dev\\Async\\AsyncDemo\\file2.txt",
        L"\\\\?\\C:\\Users\\User\\Documents\\Dev\\Async\\AsyncDemo\\file3.txt"
    };

    for (int i = 0; i < NUM_FILES; ++i)
    {
        InitFileContext(&files[i], fileNames[i]);
    }

    DWORD bytesTransferred;
    FileContext* ctx;
    OVERLAPPED* ol;

    int completedFiles = 0;

    while (completedFiles < NUM_FILES)
    {
        if (GetQueuedCompletionStatus(hIOCP, &bytesTransferred, (PULONG_PTR)&ctx, &ol, INFINITE))
        {
            ctx->bytesRead += bytesTransferred;

            if (ctx->bytesRead >= ctx->fileSize)
            {
                ctx->fileBuffer[ctx->fileSize] = '\0';
                completedFiles++;

                printf("%s\n\n", ctx->fileBuffer);
            }
            else
            {
                ContinueReading(ctx);
            }
        }
    }

    CloseHandle(hIOCP);

    return 0;
}
