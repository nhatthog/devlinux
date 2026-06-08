#include "logger.h"
#include <stdio.h>
#include <time.h>

static void write_to_file(const char *prefix, const char *msg) {
    FILE *log_file = fopen("app.log", "a");
    if (!log_file) return;

    time_t raw_time;
    struct tm *time_info;
    char time_buffer[20];

    time(&raw_time);
    time_info = localtime(&raw_time);
    strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", time_info);

    if (prefix) {
        fprintf(log_file, "[%s] %s %s\n", time_buffer, prefix, msg);
    } else {
        fprintf(log_file, "[%s] %s\n", time_buffer, msg);
    }

    fclose(log_file);
}

void log_write(const char *msg) {
    write_to_file(NULL, msg);
}

void log_timestamp(void) {
    write_to_file(NULL, "Timestamp triggered.");
}

void log_error(const char *msg) {
    write_to_file("[ERROR]", msg);
}