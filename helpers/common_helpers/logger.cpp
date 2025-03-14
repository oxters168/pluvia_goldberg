#include "common_helpers/logger.h"
#include <windows.h>
#include <cstdio>

void ClearLogFile() {
  remove("steam_api.log");
}
void LogMessage(const char* format, ...) {
  FILE* logFile = fopen("steam_api.log", "a");
  if (logFile) {
    SYSTEMTIME st;
    GetLocalTime(&st);

    // Print timestamp
    fprintf(logFile, "[%02d:%02d:%02d.%03d] ",
      st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

    // Handle variable arguments
    va_list args;
    va_start(args, format);
    vfprintf(logFile, format, args);
    va_end(args);

    // Add newline, flush and close
    fprintf(logFile, "\n");
    fflush(logFile);
    fclose(logFile);
  }
}
void LogError(const char* operation) {
  DWORD error = WSAGetLastError();
  LogMessage("Steam API Socket Error: %s failed with error %lu", operation, error);
}
