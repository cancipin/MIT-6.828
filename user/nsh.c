#include "kernel/types.h"
#include "user/user.h"
#include <kernel/fcntl.h>

#define LINE_LEN 512
#define NUM_CMD 32
#define NUM_ARGS 64

char buf[LINE_LEN];

struct Command {
  char *name;
  char *args[NUM_ARGS];
  char *infile, *outfile;
} cmds[NUM_CMD];

int read_line() {
  char c;
  int i = 0;
  memset(buf, 0, sizeof(buf));
  while (read(0, &c, 1) == 1 && c != '\n') {
    buf[i++] = c;
  }
  int ret = (i == 0 && buf[0] == 0) ? -1 : i;
  return ret;
}

void next_word(int *i, int *j) {
  while (buf[*i] == ' ') {
    ++(*i);
  }
  if (buf[*i] == 0) {
    return;
  }
  *j = *i;
  while (buf[*j] != 0 && buf[*j] != ' ') {
    ++(*j);
  }
}

int parse_cmd(struct Command *cmd, int *begin) {
  int i = *begin, j;
  next_word(&i, &j);
  if (buf[i] == 0) {
    return -1;
  }
  cmd->name = buf + i;
  buf[j] = 0;
  i = j + 1;
  cmd->args[0] = cmd->name;
  int num_args = 1;
  cmd->infile = cmd->outfile = 0;

  while (1) {
    next_word(&i, &j);
    if (buf[i] == 0) {
      break;
    }
    if (buf[i] == '<' || buf[i] == '>') {
      char c = buf[i];
      ++i;
      next_word(&i, &j);
      buf[j] = 0;
      if (c == '<') {
        cmd->infile = buf + i;
      } else {
        cmd->outfile = buf + i;
      }
      i = j + 1;
    } else if (buf[i] == '|') {
      ++i;
      break;
    } else {
      cmd->args[num_args++] = buf + i;
      buf[j] = 0;
      i = j + 1;
    }
  }
  cmd->args[num_args] = 0;
  *begin = i;
  // fprintf(2, "CMD NAME: \"%s\"\n", cmd->name);
  // for (int i = 0; cmd->args[i]; ++i) {
  //   fprintf(2, "CMD ARG #%d: \"%s\"\n", i, cmd->name);
  // }
  // fprintf(2, "CMD INFILE: \"%s\"\n", cmd->infile);
  // fprintf(2, "CMD OUTFILE: \"%s\"\n", cmd->outfile);
  return 0;
}

int main() {
  while (1) {
    printf("@ ");
    if (read_line() < 0) {
      break;
    }
    int begin = 0, num_cmd = 0;
    while (parse_cmd(&cmds[num_cmd], &begin) >= 0) {
      ++num_cmd;
    }
    int in_fd[2], out_fd[2];
    pipe(in_fd);
    pipe(out_fd);
    for (int i = 0; i < num_cmd; ++i) {
      if (fork()) {
        close(in_fd[0]);
        close(in_fd[1]);
        wait(0);
        in_fd[0] = out_fd[0];
        in_fd[1] = out_fd[1];
        pipe(out_fd);
      } else {
        if (cmds[i].infile) {
          close(0);
          open(cmds[i].infile, O_RDONLY);
        } else if (i != 0) {
          close(0);
          dup(in_fd[0]);
        }
        if (cmds[i].outfile) {
          close(1);
          open(cmds[i].outfile, O_CREATE | O_WRONLY);
        } else if (i != num_cmd - 1) {
          close(1);
          dup(out_fd[1]);
        }
        close(in_fd[0]);
        close(in_fd[1]);
        close(out_fd[0]);
        close(out_fd[1]);
        exec(cmds[i].name, cmds[i].args);
      }
    }
  }
  exit(0);
}
