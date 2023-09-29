#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char** argv)
{
    // check for edge cases and exit if error
    if (argc != 2)
    {
      fprintf(2, "sleep error\n");
      exit(1);
    }
    // convert string to int and check if is a valid number for sleep
    int tick = atoi(argv[1]);
    if (tick < 0) 
    {
      fprintf(2, "sleep [arg] error!\n");
      exit(1);
    }
    // call the syscall sleep 
    sleep(tick);
    exit(0);
}