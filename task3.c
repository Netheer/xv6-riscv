#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>


static int write_all(int fd, const void* buffer, size_t n) {
    const unsigned char* p = (const unsigned char*)buffer;
    size_t off = 0;

    while (off < n) {
        ssize_t m = write(fd, p + off, n - off);
        if (m < 0) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        off += (size_t)m;
    }
	return 0;
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
		if (close(pipefd[0]) < 0)
			perror("cloes pipefd[0]");
		if (close(pipefd[1]) < 0)
			perror("close pipefd[1]");

        return 1;
    }

    if (pid == 0) {
    	if (close(pipefd[1]) < 0) {
    		perror("child close pipefd[1]");
    		exit(1);
    	}

        unsigned char buffer[4096];

        for (;;) {
            ssize_t m = read(pipefd[0], buffer, sizeof(buffer));
            if (m < 0) {
                if (errno == EINTR) {
                    continue;
                }
                perror("read");
            	if (close(pipefd[1]) < 0)
            		perror("child close pipefd[0]");
            	exit(1);
            }
            if (m == 0) {
                break;
            }

        	if (write_all(STDOUT_FILENO, buffer, (size_t)m) < 0) {
        		perror("write");
        		if (close(pipefd[0]) < 0)
        			perror("child close pipefd[0]");
        		exit(1);
        	}
        }

    	if (close(pipefd[0]) < 0) {
    		perror("child close pipefd[0]");
    		exit(1);
    	}

        exit(0);
    }

	if (close(pipefd[0]) < 0) {
		perror("parent close pipefd[0]");
		if (close(pipefd[1]) < 0)
			perror("parent close pipefd[1]");
		waitpid(pid, NULL, 0);
		return 1;
	}

    unsigned char outbuffer[4096];
    size_t used = 0;

    for (int i = 1; i < argc; i++) {
        size_t len = strlen(argv[i]);

        if (len + 1 > sizeof(outbuffer)) {
        	if (used > 0) {
        		if (write_all(pipefd[1], outbuffer, used) < 0) {
        			perror("write");
        			if (close(pipefd[1]) < 0)
        				perror("parent close pipefd[1]");
        			waitpid(pid, NULL, 0);
        			return 1;
        		}
        		used = 0;
        	}

    		if (write_all(pipefd[1], argv[i], len) < 0) {
    			perror("write");
    			if (close(pipefd[1]) < 0)
    				perror("parent close pipefd[1]");
    			waitpid(pid, NULL, 0);
    			return 1;
    		}

        	if (write_all(pipefd[1], "\n", 1) < 0) {
        		perror("write");
        		if (close(pipefd[1]) < 0)
        			perror("parent close pipefd[1]");
        		waitpid(pid, NULL, 0);
        		return 1;
        	}

			continue;
		}

        if (used + len + 1 > sizeof(outbuffer)) {
        	if (write_all(pipefd[1], outbuffer, used) < 0) {
        		perror("write");
        		if (close(pipefd[1]) < 0)
        			perror("parent close pipefd[1]");
        		waitpid(pid, NULL, 0);
        		return 1;
        	}
        	used = 0;
        }

        memcpy(outbuffer + used, argv[i], len);
        used += len;
        outbuffer[used++] = '\n';
    }

	if (used > 0) {
		if (write_all(pipefd[1], outbuffer, used) < 0) {
			perror("write");
			if (close(pipefd[1]) < 0)
				perror("parent close pipefd[1]");
			waitpid(pid, NULL, 0);
			return 1;
		}
	}

	if (close(pipefd[1]) < 0) {
		perror("parent close pipefd[1]");
		waitpid(pid, NULL, 0);
		return 1;
	}

	int status = 0;
	if (waitpid(pid, &status, 0) < 0) {
		perror("waitpid");
		return 1;
	}

	if (WIFEXITED(status)) {
		return WEXITSTATUS(status);
	}
	if (WIFSIGNALED(status)) {
		return 128 + WTERMSIG(status);
	}

	return 1;
}