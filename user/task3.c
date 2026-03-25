#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

static void ok(char* name) {
    printf("[ok] %s\n", name);
}

static void fail(char* name) {
    printf("[failed] %s\n", name);
}

static void check(int condition, char* name) {
    if (condition) {
        ok(name);
    } else {
        fail(name);
    }
}

static void test_read_write_mutex(){
    char buf[8];
    int m = mutex();
    if (m < 0) {
        fail("mutex() in test_read_write_mutex");
        return;
    }

    int r1 = read(m, buf, sizeof(buf));
    int r2 = write(m, "x", 1);

    check(r1 < 0, "read(mutex) returns error");
    check(r2 < 0, "write(mutex) returns error");

    close(m);
}

static void test_fstat_mutex(void)
{
  struct stat st;
  int m = mutex();
  if(m < 0){
    fail("mutex() in test_fstat_mutex");
    return;
  }

  int rc = fstat(m, &st);
  check(rc < 0, "fstat(mutex) returns error");
  close(m);
}

static void test_close_locked_by_owner(void) {
    int m = mutex();
    if (m < 0) {
        fail("mutex() in test_close_locked_by_owner");
        return;
    }

    if (mutex_lock(m) < 0) {
        fail("mutex_lock() in test_close_locked_by_owner");
        close(m);
        return;
    }

    int rc = close(m);
    check(rc == 0, "close(locked mutex by owner) succeeds");
}

static void test_close_locked_by_other(void) {
    int m = mutex();
    if (m < 0) {
        fail("mutex() in test_close_locked_by_other");
        return;
    }

    if (mutex_lock(m) < 0) {
        fail("parent mutex_lock in test_close_locked_by_other");
        close(m);
        return;
    }

    int pid = fork();
    if (pid < 0) {
        fail("fork in test_close_locked_by_other");
        mutex_unlock(m);
        close(m);
        return;
    }

    if (pid == 0) {
        int rc = close(m);
        if (rc == 0)
            exit(0);
        else 
            exit(1);
    }

    int st = 0;
    wait(&st);

    int rc = mutex_unlock(m);
    check(st == 0, "child close(locked mutex owned by parent) succeeds");
    check(rc == 0, "parent still can unlock after child closes its fd");

    close(m);
}

static void test_unlock_by_other(void) {
    int m = mutex();
    if (m < 0) {
        fail("mutex() in test_unlock_by_other");
        return;
    }

    if (mutex_lock(m) < 0) {
        fail("parent mutex_lock in test_unlock_by_other");
        close(m);
        return;
    }

    int pid = fork();
    if (pid < 0) {
        fail("fork in test_unlock_by_other");
        mutex_unlock(m);
        close(m);
        return;
    }

    if (pid == 0) {
        int rc = mutex_unlock(m);
        if (rc < 0)
            exit(0);
        else 
            exit(1);
    }

    int st = 0;
    wait(&st);
    check(st == 0, "mutex_unlock by non-owner returns error");

    mutex_unlock(m);
    close(m);
}

static void test_exit_releases_mutex(void) {
    int m = mutex();
    if (m < 0) {
        fail("mutex() in test_exit_releases_mutex");
        return;
    }

    int pid = fork();
    if (pid < 0) {
        fail("fork #1 in test_exit_releases_mutex");
        close(m);
        return;
    }

    if (pid == 0) {
        if (mutex_lock(m) < 0)
            exit(1);
        exit(0);
    }

    wait(0);

    int rc = mutex_lock(m);
    check(rc == 0, "exit releases locked mutex");

    if (rc == 0)
        mutex_unlock(m);
    close(m);
}

static void test_waiter_wakes_after_owner_exit(void) {
    int sync[2];
    if (pipe(sync) < 0) {
        fail("pipe in test_waiter_wakes_after_owner_exit");
        return;
    }

    int m = mutex();
    if (m < 0) {
        fail("mutex() in test_waiter_wakes_after_owner_exit");
        close(sync[0]);
        close(sync[1]);
        return;
    }

    int pid = fork();
    if (pid < 0) {
        fail("fork in test_waiter_wakes_after_owner_exit");
        close(sync[0]);
        close(sync[1]);
        close(m);
        return;
    }

    if (pid == 0) {
        close(sync[0]);
        if (mutex_lock(m) < 0)
            exit(1);
        if (write(sync[1], "x", 1) != 1)
            exit(1);
        exit(0);
    }
    close(sync[1]);

    char ch;
    if (read(sync[0], &ch, 1) != 1) {
        fail("sync read in test_waiter_wakes_after_owner_exit");
        close(sync[0]);
        close(m);
        wait(0);
        return;
    }
    close(sync[0]);

    int rc = mutex_lock(m);
    check(rc == 0, "waiting locker wakes after owner exit");

    if (rc == 0)
        mutex_unlock(m);

    wait(0);
    close(m);
}

static void test_many_mutex_create_close(void) {
    int all_ok = 1;
    for (int i = 0; i < 64; i++) {
        int m = mutex();
        if (m < 0) {
            all_ok = 0;
            break;
        }
        if (close(m) < 0) {
            all_ok = 0;
            break;
        }
    }

    check(all_ok, "create and close many mutexes");
}

int main(void) {
    printf("task3: mutex test starts\n");

    test_read_write_mutex();
    test_fstat_mutex();
    test_close_locked_by_owner();
    test_close_locked_by_other();
    test_unlock_by_other();
    test_exit_releases_mutex();
    test_waiter_wakes_after_owner_exit();
    test_many_mutex_create_close();

    printf("task3: mutex test ends\n");
    exit(0);
}