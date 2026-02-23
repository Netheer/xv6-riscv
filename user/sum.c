#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

static int atoi_signed(const char* s, int* out) {
	if (s == 0 || *s == '\0') return -1;

	int sign = 1;
	if (*s == '-') {
		sign = -1;
		s++;
		if (*s == '\0') return -1;
	}

	int val = atoi(s);
	*out = sign * val;
	return 0;
}

static int is_valid_int_number(const char* s) {
	if (s == 0 || *s == '\0') return 0;

	int i = 0;
	if (s[i] == '-') {
		i++;
		if (s[i] == '\0') return 0;
	}

	for ( ; s[i] != '\0'; i++) {
		if (s[i] < '0' || s[i] > '9') return 0;
	}
	return 1;
}

static int read_line(int fd, char* buf, int size) {
	if (size <= 0) return -2;

	int n = 0;
	while (1) {
		char c;
		int r = read(fd, &c, 1);

		if (r < 0) return -1;
		if (r == 0) {
			if (n == 0) return -3;
			break;
		}

		if (c == '\n') break;

		if (n >= size - 1) return -2;
		buf[n++] = c;
	}

	buf[n] = '\0';
	return n;
}

int main(int argc, char* argv[]) {
	(void) argc;
	(void) argv;

	char buf[128];

	int rc = read_line(0, buf, sizeof(buf));
	if (rc == -1) {
		printf("Error: reading failed\n");
		exit(1);
	}
	if (rc == -2) {
		printf("Error: buffer overflow\n");
		exit(1);
	}
	if (rc == -3) {
		printf("Error: empty input\n");
		exit(1);
	}
	if (rc == 0) {
		printf("Error: empty line\n");
		exit(1);
	}
	printf("|%s|\n", buf);

	char* p = buf;
	while (*p == ' ') p++;

	if (*p == '\0') {
		printf("Error: empty line\n");
		exit(1);
	}

	char* space = p;
	while (*space != '\0' && *space != ' ') space++;
	if (*space == '\0') {
		printf("Error: expected two numbers separated by a space\n");
		exit(1);
	}
	
	*space = '\0';
	char *a_str = p;

	char *q = space + 1;
	while (*q == ' ') q++;

	if (*q == '\0') {
		printf("Error: second number is missing\n");
		exit(1);
	}

	char *end = q;
	while (*end != '\0') end++;
	while (end > q && *(end - 1) == ' ') {
		*(end - 1) = '\0';
		end--;
	}

	char *b_str = q;

	if (!is_valid_int_number(a_str)) {
		printf("Error: first number is not a valid integer: %s\n", a_str);
		exit(1);
	}
	if (!is_valid_int_number(b_str)) {
		printf("Error: second number is not a valid integer: %s\n", b_str);
		exit(1);
	}

	int a, b;
	if (atoi_signed(a_str, &a) < 0) { printf("Error: first number is not a valid integer: %s\n", a_str); exit(1); }
	if (atoi_signed(b_str, &b) < 0) { printf("Error: second number is not a valid integer: %s\n", b_str); exit(1); }

	int s = add(a, b);
	printf("%d\n", s);
	exit(0);
}