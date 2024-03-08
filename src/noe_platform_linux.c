#include "noe.h"

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/time.h>

#define LOG_MESSAGE_MAXIMUM_LENGTH (32*1024)
static const char *logLevelsAsText[] = {
    "[INFO] ",
    "[WARNING] ",
    "[ERROR] ",
    "[FATAL] ",
};

void TraceLog(int logLevel, const char *fmt, ...)
{
    if(!(LOG_INFO <= logLevel && logLevel <= LOG_FATAL)) return;
    char logMessage[LOG_MESSAGE_MAXIMUM_LENGTH] = {0};
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(logMessage, LOG_MESSAGE_MAXIMUM_LENGTH, fmt, ap);
    va_end(ap);
    if(logLevel == LOG_FATAL || logLevel == LOG_ERROR) fprintf(stderr, "%s %s\n", logLevelsAsText[logLevel], logMessage);
    else printf("%s %s\n", logLevelsAsText[logLevel], logMessage);
}

void *MemoryAlloc(size_t nBytes)
{
    void *result = malloc(nBytes);
    if(!result) return NULL;
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
