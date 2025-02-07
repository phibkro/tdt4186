#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char const *argv[])
{
  if (argc <= 1) {
    printf("Hello World\n");
  } else {
    printf("Hello %s, nice to meet you!\n", argv[1]);
  }
  return 0;
}
