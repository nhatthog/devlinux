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

    // Thiết lập cấu trúc fcntl để khóa toàn bộ file
    struct flock fl = {
        .l_type   = F_WRLCK,   // Exclusive Write lock
        .l_whence = SEEK_SET,
        .l_start  = 0,
        .l_len    = 0,         // 0 có nghĩa là khóa cho đến hết file (EOF)
    };

    // Acquire lock (Blocking)
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
    strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", timeinfo);

    char log_buffer[512];
    int len = snprintf(log_buffer, sizeof(log_buffer), "[PID:%d] [%s] [%s] %s\n",
                       getpid(), time_buffer, "INFO", argv[1]);

    usleep(50000); // 50ms race-condition test simulation

    if (write(fd, log_buffer, len) != len) {
        perror("write");
    }

    // Unlock
    fl.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &fl);

    close(fd);
    return EXIT_SUCCESS;
}