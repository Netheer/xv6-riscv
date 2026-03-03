#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>


static void write_all(int fd, const void* buffer, size_t n) {
    const unsigned char* p = (const unsigned char*)buffer;
    size_t off = 0;

    while (off < n) {
        ssize_t m = write(fd, p + off, n - off);
        if (m < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("write");
            exit(1);
        }
        off += (size_t)m;
    }
}

int main(int argc, char* argv[]) {
    int pipefd[2];

    if (pipe(pipefd) < 0) {
        perror("pipe");
        return 1;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        close(pipefd[1]);

        unsigned char buffer[4096];

        for (;;) {
            ssize_t m = read(pipefd[0], buffer, sizeof(buffer));
            if (m < 0) {
                if (errno == EINTR) {
                    continue;
                }
                perror("read");
                exit(1);
            }
            if (m == 0) {
                break;
            }

            write_all(STDOUT_FILENO, buffer, (size_t)m);
        }

        close(pipefd[0]);
        exit(0);
    }

    close (pipefd[0]);

    unsigned char outbuffer[4096];
    size_t used = 0;

    for (int i = 1; i < argc; i++) {
        size_t len = strlen(argv[i]);

        if (len + 1 > sizeof(outbuffer)) {
            if (used > 0) {
                write_all(pipefd[1], outbuffer, used);
                used = 0;
            }
            write_all(pipefd[1], argv[i], len);
            write_all(pipefd[1], "\n", 1);
            continue;
        }

        if (used + len + 1 > sizeof(outbuffer)) {
            write_all(pipefd[1], outbuffer, used);
            used = 0;
        }

        memcpy(outbuffer + used, argv[i], len);
        used += len;
        outbuffer[used++] = '\n';
    }

    if (used > 0) {
        write_all(pipefd[1], outbuffer, used);
    }

    close(pipefd[1]);

    int status = 0;
    if (waitpid(pid, &status, 0) < 0) {
        perror("waitpid");
        return 1;
    }

    return 0;
}