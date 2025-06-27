#ifndef KEYLOGGER_H
#define KEYLOGGER_H

#define MAX_PATH 260
#define LOG_FILE_NAME "system_logs.dat"

void start_keylogger();
void log_keystroke(const char* key);

#endif
