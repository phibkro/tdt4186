#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char const *argv[])
{
  printf("test ps\n");
  pdumps();
  return 0;
}
