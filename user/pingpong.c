#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char** argv)
{
    if (argc != 1)
    {
        fprintf(2, "pingpong error!\n");
        exit(1);
    }
        
    int p[2];
    pipe(p);
    if (fork() == 0) // child
    {
        char buf;
        read(p[0], &buf, 1);
        printf("%d: received ping\n", getpid());
        write(p[1], "1", 1);
    }
    else // parent
    {
        write(p[1], "1", 1);
        char buf;
        read(p[0], &buf, 1);
        printf("%d: received pong\n", getpid());
    }

    exit(0);
}
