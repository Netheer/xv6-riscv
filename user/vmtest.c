#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/riscv.h"
#include "user/user.h"

int global_value = 1;
char global_array[1024];
volatile int sink = 0;

static void die(const char* message) {
    fprintf(2, "vmtest: %s\n", message);
    exit(1);
}

static void must_clear(const char* name, void* buf, int len, int mask) {
    if (pgclearflags(buf, len, mask) < 0) {
        fprintf(2, "vmtest: pgclearflags failed for %s\n", name);
        exit(1);
    }
}

static int must_info(const char* name, void* buf, int len, int mask) {
    int r;

    r = pgaccessinfo(buf, len, mask);
    if (r < 0) {
        fprintf(2, "vmtest: pgaccessinfo failed for %s\n", name);
        exit(1);
    }
    return r;
}

static void show_region_flags(const char* name, void* buf, int len) {
    int a, d;

    a = must_info(name, buf, len, PTE_A);
    d = must_info(name, buf, len, PTE_D);

    printf("%s: A=%d D=%d\n", name, a, d);
}

static void show_all_flags(int* stack_value, char* stack_array, char* heap, int heap_len) {
    show_region_flags("global_value", &global_value, sizeof(global_value));
    show_region_flags("global_array", global_array, sizeof(global_array));
    show_region_flags("stack_value", stack_value, sizeof(*stack_value));
    show_region_flags("stack_array", stack_array, 512);
    show_region_flags("heap", heap, heap_len);
}

static void clear_all_flags(int* stack_value, char* stack_array, char* heap, int heap_len) {
    int mask = PTE_A | PTE_D;

    must_clear("global_value", &global_value, sizeof(global_value), mask);
    must_clear("global_array", global_array, sizeof(global_array), mask);
    must_clear("stack_value", stack_value, sizeof(*stack_value), mask);
    must_clear("stack_array", stack_array, 512, mask);
    must_clear("heap", heap, heap_len, mask);
}

static void read_all(int* stack_value, char* stack_array, char* heap, int heap_len) {
    int sum = 0;

    sum += global_value;
    sum += global_array[0];
    sum += *stack_value;
    sum += stack_array[0];

    for(int i = 0; i < heap_len; i += PGSIZE)
        sum += heap[i];

    sink += sum;
}

static void write_all(int* stack_value, char* stack_array, char* heap, int heap_len) {
    global_value += 1;
    global_array[0] += 1;
    *stack_value += 1;
    stack_array[0] += 1;

    for (int i = 0; i < heap_len; i += PGSIZE)
        heap[i] += 1;
}

int main() {
    int stack_value = 10;
    char stack_array[512];
    char* heap;
    int heap_len;

    stack_array[0] = 1;

    printf("== startup ==\n");
    vmprint();
    show_region_flags("global_value", &global_value, sizeof(global_value));
    show_region_flags("global_array", global_array, sizeof(global_array));
    show_region_flags("stack_value", &stack_value, sizeof(stack_value));
    show_region_flags("stack_array", stack_array, sizeof(stack_array));

    heap_len = 2 * PGSIZE;
    heap = malloc(heap_len);
    if (heap == 0)
        die("malloc failed");

    heap[0] = 1;
    heap[PGSIZE] = 2;

    printf("\n== after allocation ==\n");
    vmprint();
    show_all_flags(&stack_value, stack_array, heap, heap_len);

    clear_all_flags(&stack_value, stack_array, heap, heap_len);
    printf("\n== after clear A/D ==\n");
    vmprint();
    show_all_flags(&stack_value, stack_array, heap, heap_len);

    read_all(&stack_value, stack_array, heap, heap_len);
    printf("\n== after read ==\n");
    vmprint();
    show_all_flags(&stack_value, stack_array, heap, heap_len);

    clear_all_flags(&stack_value, stack_array, heap, heap_len);
    printf("\n== after second clear A/D ==\n");
    vmprint();
    show_all_flags(&stack_value, stack_array, heap, heap_len);

    write_all(&stack_value, stack_array, heap, heap_len);
    printf("\n== after write ==\n");
    vmprint();
    show_all_flags(&stack_value, stack_array, heap, heap_len);

    free(heap);
    printf("\n== after free ==\n");
    vmprint();
    show_region_flags("global_value", &global_value, sizeof(global_value));
    show_region_flags("global_array", global_array, sizeof(global_array));
    show_region_flags("stack_value", &stack_value, sizeof(stack_value));
    show_region_flags("stack_array", stack_array, sizeof(stack_array));

    exit(0);
}