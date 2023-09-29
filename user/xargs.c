#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"
#include "kernel/param.h"

// function copies args from origArgv to argv and adds newArgv to the end
void copyArgv(char** origArgv, int origArgc, char* newArgv, char** argv)
{
    int k = 0;
    for (int i = 0; i < origArgc; i++) 
    {
        argv[k] = malloc(strlen(origArgv[i]) + 1);
        memcpy(argv[k++], origArgv[i], strlen(origArgv[i]) + 1);
    }

    argv[k] = malloc(strlen(newArgv) + 1);
    memcpy(argv[k++], newArgv, strlen(newArgv) + 1);
}

int main(int argc, char** argv)
{
    // check for bad input
    if (argc <= 1) 
    {
        fprintf(2, "xargs [arg] error!\n");
        exit(1);
    }

    char args[512];
    char c;
    int i = 0;
    int stop = 0;
    // read by character till end of input
    while (read(0, &c, 1) > 0) 
    {
        // if end of argument, fork and exec
        if (c == '\n') 
        {
            // if no arguments, continue
            if (stop) 
            {
                i = 0;
                stop = 0;
                continue;
            }

            // add termination character to the end
            args[i] = 0;
            i = 0;

            if (fork() == 0) // child
            {
                int cmdArgc = argc;
                
                char* cmdArgv[MAXARG];

                copyArgv(argv + 1, argc - 1, args, cmdArgv);
                cmdArgv[cmdArgc] = 0;
                
                exec(cmdArgv[0], cmdArgv);

                exit(0);
            } 
            else // parent
            {
                wait(0);
            }
        }
        else 
        { 
            // if too many arguments, stop
            if (!stop && i >= 511) 
            {
                printf("xargs [length] error!\n");
                stop = 1;
            }
            // if not too many arguments, add to args
            if (!stop)
                args[i++] = c;
        }
    }

    exit(0);
}