#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

char* fmtname(char *path)
{
    static char buf[DIRSIZ+1];
    char *p;

    // Find first character after last slash.
    for(p=path+strlen(path); p >= path && *p != '/'; p--);
    
    p++;

    // Return blank-padded name.
    if(strlen(p) >= DIRSIZ)
        return p;

    memmove(buf, p, strlen(p));
    memset(buf+strlen(p), ' ', DIRSIZ-strlen(p));
    return buf;
}

int checkFind(char* dir, char* name)
{
    if (strlen(dir) > DIRSIZ || strlen(name) > DIRSIZ)
    {
        fprintf(2, "find [lenght] error!\n");
        return -1;
    }
    int fd;

    if ((fd = open(dir, O_RDONLY) < 0))
    {
        fprintf(2, "find [open] error!\n");
        return -2;
    }
    struct stat st;
    
    if (fstat(fd, &st) < 0)
    {
        fprintf(2, "find [fstat] error!\n");
        close(fd);
        return -3;
    }

    if (st.type != T_DIR) 
        printf("find: not a dir!\n");
    else
        find(fd, dir, name);
    return 0;
}

void find(int fd, char* dir, char* name)
{

}

int main(int argc, char** argv)
{
    if (argc != 3)
    {
        fprintf(2, "find [arg] error!\n");
        exit(1);        
    }
    int err = checkFind(argv[1], argv[2]);
    
    if (err < 0)
        exit(1);
    else
	    exit(0);
}
