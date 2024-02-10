
#ifndef LOG_H
#define LOG_H

enum LogLevel {
    // EMERGENCY = 0,
    // ALERT = 1,
    // CRITICAL = 2,
    ERROR = 3,
    WARNING = 4,
    // NOTIFICATION = 5,
    INFO = 6,
    DEBUG = 7,
};

void log_error(char *, ...);
void log_info(char *, ...);
void log_warning(char *, ...);
void log_debug(char *, ...);
void log_error(char *, ...);

#endif // !LOG_H
