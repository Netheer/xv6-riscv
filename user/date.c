#include "kernel/types.h"
#include "user/user.h"

static int is_leap(int year) {
    if (year % 400 == 0)
        return 1;
    if (year % 100 == 0)
        return 0;
    if (year % 4 == 0)
        return 1;
    return 0;
}

static int days_in_year(int year) {
    return is_leap(year) ? 366 : 365;
}

static int days_in_month(int year, int month) {
    static const int days[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    if (month == 2 && is_leap(year))
        return 29;
    return days[month - 1];
}

static void print2(int x) {
    if (x < 10) {
        printf("0");
    }
    printf("%d", x);
}

static void print9(int x) {
    int div;

    div = 100000000;
    while (div > 0) {
        printf("%d", x / div);
        x %= div;
        div /= 10;
    }
}

int main(void) {
    uint64 time;
    uint64 seconds64, nanoseconds64;
    int seconds, minutes, hour, year, month, day;
    uint64 days64;

    if (getrtc(&time) < 0) {
        fprintf(2, "date: getrtc failed\n");
        exit(1);
    }

    seconds64 = time / 1000000000ULL;
    nanoseconds64 = time % 1000000000ULL;
    days64 = seconds64 / 86400ULL;
    seconds = seconds64 % 60ULL;
    minutes = (seconds64 / 60ULL) % 60ULL;
    hour = (seconds64 / 3600ULL) % 24ULL;

    year = 1970;
    while (days64 >= (uint64)days_in_year(year)) {
        days64 -= days_in_year(year);
        year++;
    }

    month = 1;

    while (days64 >= (uint64)days_in_month(year, month)) {
        days64 -= days_in_month(year, month);
        month++;
    }

    day = days64 + 1;

    printf("%d-", year);
    print2(month);
    printf("-");
    print2(day);
    printf(" ");
    print2(hour);
    printf(":");
    print2(minutes);
    printf(":");
    print2(seconds);
    printf(".");
    print9(nanoseconds64);
    printf("\n");

    exit(0);    
}