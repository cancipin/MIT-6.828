#include "kernel/types.h"
#include "user/user.h"

int main() {
  const char from_parent[] = "ping";
  const char from_child[] = "pong";
  char buf[256];

  int parent_fd[2], child_fd[2];
  pipe(parent_fd);
  pipe(child_fd);

  if (fork() == 0) { // child
    read(parent_fd[0], buf, sizeof(buf));
    close(parent_fd[0]);
    printf("%d: received %s\n", getpid(), buf);
    write(child_fd[1], from_child, sizeof(from_child));
    close(child_fd[1]);
    exit(0);
  } else { // parent
    write(parent_fd[1], from_parent, sizeof(from_parent));
    close(parent_fd[1]);
    read(child_fd[0], buf, sizeof(buf));
    close(child_fd[0]);
    printf("%d: received %s\n", getpid(), buf);
    exit(0);
  }
}
