#include "kernel/types.h"
#include "user/user.h"

static int is_leap(int year) {
    if (year % 400 == 0) return 1;
    if (year % 100 == 0) return 0;
    if (year % 4 == 0)   return 1;
    return 0;
}

static int days_in_month(int year, int month) {
    static const int days[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    if (month == 2 && is_leap(year)) return 29;
    return days[month - 1];
}

static int64 floordiv(int64 a, int64 b) {
    return a / b - (a % b != 0 && ((a < 0) != (b < 0)));
}

static void print2(int x) {
    if (x < 10) printf("0");
    printf("%d", x);
}

static void print9(int x) {
    int div = 100000000;
    while (div > 0) {
        printf("%d", x / div);
        x %= div;
        div /= 10;
    }
}

int main(void) {
    int64 time;
    int64 seconds64, days64;
    int nanoseconds, rem_seconds;
    int seconds, minutes, hour, year, month, day;

    if (getrtc(&time) < 0) {
        fprintf(2, "date: getrtc failed\n");
        exit(1);
    }

    seconds64   = floordiv(time, 1000000000LL);
    nanoseconds = (int)(time - seconds64 * 1000000000LL);

    days64      = floordiv(seconds64, 86400LL);
    rem_seconds = (int)(seconds64 - days64 * 86400LL);
    hour        = rem_seconds / 3600;
    minutes     = (rem_seconds % 3600) / 60;
    seconds     = rem_seconds % 60;

    int64 n400 = floordiv(days64, 146097);
    days64 -= n400 * 146097;
    year = (int)(1970 + n400 * 400);

    int n100 = (int)(days64 / 36524);
    if (n100 > 3) n100 = 3;
    days64 -= n100 * 36524;
    year += n100 * 100;

    int n4 = (int)(days64 / 1461);
    days64 -= n4 * 1461;
    year += n4 * 4;

    int n1 = (int)(days64 / 365);
    if (n1 > 3) n1 = 3;
    days64 -= n1 * 365;
    year += n1;

    month = 1;
    while (month <= 12) {
        int dim = days_in_month(year, month);
        if (days64 < dim) break;
        days64 -= dim;
        month++;
    }
    day = (int)days64 + 1;

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
    print9(nanoseconds);
    printf("\n");

    exit(0);
}
