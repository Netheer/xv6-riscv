#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

static int write_all(int fd, const char* buffer, int n) {
    int off = 0;
    while (off < n) {
        int m = write(fd, buffer + off, n - off);
        if (m < 0) {
            return -1;
        }
        off += m;
    }
	return 0;
}

int main(int argc, char* argv[]) {
    int p[2];
    if (pipe(p) < 0) {
        fprintf(2, "task2: pipe failed\n");
        exit(1);
    }

    int pid = fork();
    if (pid < 0) {
        fprintf(2, "task2: fork failed\n");
    	if (close(p[0]) < 0)
    		fprintf(2, "task2: close read end failed\n");
    	if (close(p[1]) < 0)
    		fprintf(2, "task2: close write end failed\n");
        exit(1);
    }

    if (pid == 0) {
        if (close(p[1]) < 0) {
			fprintf(2, "task2: child close write end failed\n");
			exit(1);
    	}

		if (close(0) < 0) {
			fprintf(2, "task2: child close stdin failed\n");
			exit(1);
		}

		if (dup(p[0]) != 0) {
			fprintf(2, "task2: dup failed or did not return 0\n");
			if (close(p[0]) < 0)
				fprintf(2, "task2: child close read end failed\n");
			exit(1);
		}

		if (close(p[0]) < 0) {
			fprintf(2, "task2: child close read end failed\n");
			exit(1);
		}

		char *wc_argv[] = {"wc", 0};
		exec("wc", wc_argv);

		fprintf(2, "task2: exec wc failed\n");
		exit(1);
	}

	if (close(p[0]) < 0) {
		fprintf(2, "task2: parent close read end failed\n");
		if (close(p[1]) < 0)
			fprintf(2, "task2: parent close write end failed\n");
		wait(0);
		exit(1);
	}

    for (int i = 1; i < argc; ++i) {
        int n = strlen(argv[i]);
        if (write_all(p[1], argv[i], n) < 0) {
			fprintf(2, "task2: write failed\n");
			if (close(p[1]) < 0)
				fprintf(2, "task2: parent close write end failed\n");
			wait(0);
			exit(1);
		}
    	if (write_all(p[1], "\n", 1) < 0) {
    		fprintf(2, "task2: write newline failed\n");
    		if (close(p[1]) < 0)
    			fprintf(2, "task2: parent close write end failed\n");
    		wait(0);
    		exit(1);
    	}
    }

	if (close(p[1]) < 0) {
		fprintf(2, "task2: parent close write end failed\n");
		wait(0);
		exit(1);
	}

	if (wait(0) < 0) {
		fprintf(2, "task2: wait failed\n");
		exit(1);
	}

    exit(0);
}