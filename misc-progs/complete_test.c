#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

/**
 * test program for complete module
 * mknod /dev/completion c 246 0
 * the program will block until you execute
 * echo "test" > /dev/completion
 */

int main(int argc, char *argv[])
{
  int fd = open("/dev/completion", O_RDONLY);
  if (fd == -1) {
    printf("open /dev/complete error!:%s\n", strerror(errno));
    return -1;
  }

  char buf[100];
  memset(buf, 0, 100);
  /*ssize_t count =*/ read(fd, buf, 100);

  printf("read finished!\n");

  return 0;
}
