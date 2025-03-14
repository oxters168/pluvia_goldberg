#ifndef LOGGER_H
#define LOGGER_H

#include <windows.h>
#include <cstdio>

void ClearLogFile();
void LogMessage(const char* format, ...);
void LogError(const char* operation);

#endif
