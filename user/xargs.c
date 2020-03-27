#include "kernel/param.h"
#include "kernel/types.h"
#include "user/user.h"

char buf[512];

int read_line() {
  int i = 0;
  memset(buf, 0, sizeof(buf));
  while (read(0, buf + i, 1) && buf[i] != '\n') {
    if (buf[i] == ' ') {
      buf[i] = 0;
    }
    ++i;
  }
  int res = i == 0 && buf[0] == 0 ? -1 : i;
  buf[i] = 0;
  return res;
}

int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(2, "usage: xargs [command] [args...]\n");
    exit(1);
  }
  char *cmd = argv[1];
  char *args[MAXARG];
  int i = 0;
  for (; i + 1 < argc; ++i) {
    args[i] = argv[i + 1];
  }

  while (1) {
    int sz = read_line();
    if (sz == -1) {
      break;
    }
    // printf("LINE: \"%s\"\n", buf);
    int j = 0;
    int num_args = i;
    while (j < sz) {
      while (j < sz && buf[j] == 0) {
        ++j;
      }
      if (j == sz) {
        break;
      }
      args[num_args++] = buf + j;
      while (j < sz && buf[j]) {
        ++j;
      }
    }
    args[num_args] = 0;

    // for (int i = 0; args[i]; ++i) {
    //   printf("%s\n", args[i]);
    // }

    if (fork()) {
      wait(0);
    } else {
      exec(cmd, args);
      exit(0);
    }
  }

  exit(0);
}
