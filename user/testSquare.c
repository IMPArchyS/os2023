#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char** argv) 
{
    printf("square of [%d] -> %d\n", atoi(argv[1]), square(atoi(argv[1])));
    exit(0);
}