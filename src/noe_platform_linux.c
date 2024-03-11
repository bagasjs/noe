#include "noe.h"
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/time.h>

#define LOG_MESSAGE_MAXIMUM_LENGTH (32*1024)
static const char *logLevelsAsText[] = {
    "[FATAL] ",
    "[ERROR] ",
    "[WARNING] ",
    "[INFO] ",
    "[DEBUG] ",
};

void ExitProgram(int status)
{
    DeinitApplication();
    exit(status);
}

void TraceLog(int logLevel, const char *fmt, ...)
{
    if(!(LOG_FATAL <= logLevel && logLevel <= LOG_DEBUG)) return;
    char logMessage[LOG_MESSAGE_MAXIMUM_LENGTH] = {0};
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(logMessage, LOG_MESSAGE_MAXIMUM_LENGTH, fmt, ap);
    va_end(ap);
    switch(logLevel) {
        case LOG_FATAL:
            {
                fprintf(stderr, "%s %s\n", logLevelsAsText[logLevel], logMessage);
                ExitProgram(-1);
            } break;
        case LOG_ERROR:
            {
                fprintf(stderr, "%s %s\n", logLevelsAsText[logLevel], logMessage);
            } break;
        default:
            {
                printf("%s %s\n", logLevelsAsText[logLevel], logMessage);
            } break;
    }
}

void *MemoryAlloc(size_t nBytes)
{
    void *result = malloc(nBytes);
    if(!result) return NULL;
    MemorySet(result, 0, nBytes);
    return result;
}

void MemoryFree(void *ptr)
{
    if(ptr) free(ptr);
}

uint64_t GetTimeMilis(void)
{
    return 0;
}

char *LoadFileText(const char *filePath, size_t *fileSize)
{
    FILE *f = fopen(filePath, "r");
    if(!f) return NULL;

    fseek(f, 0L, SEEK_END);
    size_t filesz = ftell(f);
    fseek(f, 0L, SEEK_SET);
    char *result = MemoryAlloc(sizeof(char) * (filesz + 1));
    if(!result) {
        TraceLog(LOG_ERROR, "Failed to load file `%s` text content", filePath);
        fclose(f);
        return NULL;
    }

    size_t readLength = fread(result, sizeof(char), filesz, f);
    result[readLength] = '\0';
    if(fileSize) *fileSize = readLength;
    fclose(f);
    return result;
}

void UnloadFileText(char *text)
{
    MemoryFree(text);
}

uint8_t *LoadFileData(const char *filePath, size_t *fileSize)
{
    FILE *f = fopen(filePath, "rb");
    if(!f) return NULL;

    fseek(f, 0L, SEEK_END);
    size_t filesz = ftell(f);
    fseek(f, 0L, SEEK_SET);
    uint8_t *result = MemoryAlloc(sizeof(uint8_t) * (filesz + 1));
    if(!result) {
        TraceLog(LOG_ERROR, "Failed to load file `%s` data", filePath);
        fclose(f);
        return NULL;
    }

    size_t readLength = fread(result, sizeof(uint8_t), filesz, f);
    if(fileSize) *fileSize = readLength;
    fclose(f);
    return result;
}

void UnloadFileData(uint8_t *data)
{
    MemoryFree(data);
}
