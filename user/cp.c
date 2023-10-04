#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

#define BUFFER_SIZE 512

int main(int argc, char** argv)
{
    // check for valid arg count
    if (argc != 3)
    {
        fprintf(2, "cp [arg] error!\n");
        exit(1);
    }

    // open source folder
    int srcFD = open(argv[1], O_RDONLY);
    if (srcFD < 0)
    {
        fprintf(2, "cp [source] error!\n");
        exit(1);
    }

    // open destination folder, if doesnt exist create it
    int destFD = open(argv[2], O_WRONLY | O_CREATE);
    if (destFD < 0)
    {
        fprintf(2, "cp [destination] error!\n");
        exit(1);
    }

    // read bytes from srcFD to buffer of size Buffer if valid write to destFD
    int bytesRead;
    char buffer[BUFFER_SIZE];
    while ((bytesRead = read(srcFD, buffer, sizeof(buffer))) > 0) 
    {
        write(destFD, buffer, bytesRead);
    }

    close(srcFD);
    close(destFD);
    exit(0);
}