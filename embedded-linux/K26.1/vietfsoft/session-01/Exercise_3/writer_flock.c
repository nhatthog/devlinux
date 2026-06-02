/*
 * writer_flock.c — log writer using flock() for mutual exclusion
 *
 * Comparison: flock vs fcntl
 * +---------------------------------+------------------+-------------------------+
 * | Property                        | flock            | fcntl                   |
 * +---------------------------------+------------------+-------------------------+
 * | Lock granularity                | Whole file only  | Byte range supported    |
 * | Works over NFS                  | No               | Yes                     |
 * | Inherited across fork           | Yes (shared)     | No (per open-file-desc) |
 * | Automatically released on crash | Yes              | Yes                     |
 * | Best used when                  | Simple local FS  | Network FS / byte range |
 * +---------------------------------+------------------+-------------------------+
 */

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/file.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <message>\n", argv[0]);
        exit(1);
    }

    int fd = open("system.log", O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (fd < 0) { perror("open"); exit(1); }

    // Acquire exclusive lock — blocks until available
    if (flock(fd, LOCK_EX) < 0) { perror("flock"); close(fd); exit(1); }

    // Format log line
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char timebuf[32];
    strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", tm_info);

    char logline[256];
    int len = snprintf(logline, sizeof(logline),
                       "[PID:%d] [%s] [INFO] %s\n",
                       getpid(), timebuf, argv[1]);

    // Write log line
    if (write(fd, logline, len) != len)
        perror("write");

    // Release lock
    flock(fd, LOCK_UN);
    close(fd);
    return 0;
}
