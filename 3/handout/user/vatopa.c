#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Usage: %s virtual_address [pid]\n", argv[0]);
        exit(1);
    }
    
    int addr = atoi(argv[1]);
    int pid = 0; // if no pid is specified
    if (argc > 2) pid = atoi(argv[2]); // if pid is specified

    int pa = va2pa(addr, pid); // syscall

    printf("0x%x\n", pa);

    exit(0);
}