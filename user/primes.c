#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
  int fd[2], back[2];
  pipe(fd);
  pipe(back);
  int size = 35 - 2 + 1;
  write(fd[1], &size, 4);
  for (int i = 2; i <= 35; ++i) {
    write(fd[1], &i, 4);
  }

  for (;;) {
    if (fork()) {
      wait(0);
      int ret;
      read(back[0], &ret, 4);
      if (ret == 0) {
        break;
      }
    } else {
      int num_input;
      read(fd[0], &num_input, 4);
      if (num_input == 0) {
        int ret = 0;
        write(back[1], &ret, 4);
        exit(0);
      }
      int p;
      read(fd[0], &p, 4);
      printf("prime %d\n", p);
      int nums[50], cnt = 0;
      for (int i = 1; i < num_input; ++i) {
        int tmp;
        read(fd[0], &tmp, 4);
        if (tmp % p) {
          nums[cnt++] = tmp;
        }
      }
      write(fd[1], &cnt, 4);
      for (int i = 0; i < cnt; ++i) {
        write(fd[1], nums + i, 4);
      }
      int ret = 1;
      write(back[1], &ret, 4);
      exit(0);
    }
  }
  exit(0);
}
