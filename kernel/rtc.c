#include "types.h"
#include "memlayout.h"

#define RTCREG(addr) ((volatile uint32*)(addr))


static uint32 rtc_read_low(void) {
    return *RTCREG(RTC0_LOW);
}

static uint32 rtc_read_high(void) {
    return *RTCREG(RTC0_HIGH);
}

int64 rtc_read_time(void) {
    uint32 low1, high, low2;

    for (;;) {
        low1 = rtc_read_low();
        __sync_synchronize();
        high = rtc_read_high();
        __sync_synchronize();
        low2 = rtc_read_low();

        if (low2 >= low1) {
            return (int64)(((uint64)high << 32) | low1);
        }
    }
}