#include "kernel/types.h"
#include "user/user.h"
// behind types.h
#include "kernel/fcntl.h"
#include "kernel/fs.h"
#include "kernel/stat.h"

void find(char *path, char *fname, char *pattern) {
  int fd;
  struct stat st;
  if ((fd = open(path, O_RDONLY)) < 0) {
    fprintf(2, "find: cannot open %s\n", path);
    return;
  }
  if (fstat(fd, &st) < 0) {
    fprintf(2, "find: cannot stat %s\n", path);
    return;
  }

  if (strcmp(fname, pattern) == 0) {
    printf("%s\n", path);
  }

  char buf[512];
  if (st.type == T_DIR) {
    if (strlen(path) + 1 + DIRSIZ + 1 > sizeof(buf)) {
      fprintf(2, "find: path too long\n");
      return;
    }
    strcpy(buf, path);
    char *p = buf + strlen(buf);
    *(p++) = '/';
    struct dirent de;
    while (read(fd, &de, sizeof(de)) == sizeof(de)) {
      if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0) {
        continue;
      }
      if (de.inum == 0) {
        continue;
      }
      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = 0;
      find(buf, p, pattern);
    }
  }
  close(fd);
}

int main(int argc, char **argv) {
  if (argc < 3) {
    fprintf(2, "usage: find [path] [pattern]\n");
    exit(1);
  }
  find(argv[1], argv[1], argv[2]);
  exit(0);
}
