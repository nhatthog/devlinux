/*
| Property | flock | fcntl |
|---|---|---|
| Lock granularity | Whole file only | Byte range supported |
| Works over NFS | No | Yes |
| Inherited across fork | Yes | No |
| Best used when | Simple local file locking | Network FS or byte-range locking |
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/file.h>
#include <string.h>

#define LOG_FILE "system.log"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <message>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int fd = open(LOG_FILE, O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (fd < 0) {
        perror("open");
        return EXIT_FAILURE;
    }

    // Acquire Exclusive Lock (Blocking)
    if (flock(fd, LOCK_EX) < 0) {
        perror("flock LOCK_EX");
        close(fd);
        return EXIT_FAILURE;
    }

    // Lấy thời gian hiện tại
    time_t rawtime;
    struct tm *timeinfo;
    char time_buffer[20];

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", timeinfo);

    // Format log string
    char log_buffer[512];
    int len = snprintf(log_buffer, sizeof(log_buffer), "[PID:%d] [%s] [%s] %s\n",
                       getpid(), time_buffer, "INFO", argv[1]);

    // Giả lập xử lý nặng một chút để kiểm tra tính tranh chấp (race condition)
    usleep(50000); // 50ms

    if (write(fd, log_buffer, len) != len) {
        perror("write");
    }

    // Release Lock
    flock(fd, LOCK_UN);
    close(fd);

    return EXIT_SUCCESS;
}