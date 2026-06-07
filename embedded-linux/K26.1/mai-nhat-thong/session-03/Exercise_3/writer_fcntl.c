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
#include <string.h>

#define LOG_FILE "system.log"
#define SLEEP_DURATION_US 50000

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

    struct flock fl = {
        .l_type   = F_WRLCK,
        .l_whence = SEEK_SET,
        .l_start  = 0,
        .l_len    = 0,
    };

    if (fcntl(fd, F_SETLKW, &fl) < 0) {
        perror("fcntl F_SETLKW");
        close(fd);
        return EXIT_FAILURE;
    }

    time_t rawtime;
    struct tm *timeinfo;
    char time_buffer[20];

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    if (timeinfo == NULL) {
        perror("localtime");
        fl.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, &fl);
        close(fd);
        return EXIT_FAILURE;
    }
    strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", timeinfo);

    char log_buffer[512];
    int len = snprintf(log_buffer, sizeof(log_buffer), "[PID:%d] [%s] [%s] %s\n",
                       getpid(), time_buffer, "INFO", argv[1]);

    usleep(SLEEP_DURATION_US);

    if (write(fd, log_buffer, len) != len) {
        perror("write");
    }

    /* Đảm bảo toàn bộ data thực sự ghi xuống đĩa trước khi unlock */
    if (fsync(fd) < 0) {
        perror("fsync failed");
    }

    /* Check return value của việc UNLOCK */
    fl.l_type = F_UNLCK;
    if (fcntl(fd, F_SETLK, &fl) < 0) {
        perror("fcntl F_SETLK (unlock) failed");
    }

    if (close(fd) < 0) {
        perror("close fd failed");
    }

    return EXIT_SUCCESS;
}