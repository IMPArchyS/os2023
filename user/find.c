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

void find(char *path)
{
    int fd;

    if ((fd = open(path, O_RDONLY)) < 0)
    {
        fprintf(2, "find: cannot open %s\n", path);
        return;
    }

    struct stat st; 
    if (fstat(fd, &st) < 0)
    {
        fprintf(2, "find: cannot stat %s\n", path);
        return;
    }

    char buf[512];
    char* p;
    struct dirent de;

    switch (st.type)
    {
        case T_DEVICE:

        case T_FILE:
            printf("./%s\n", fmtname(path));
        break;        

        case T_DIR:
            if (strlen(path) + 1 + DIRSIZ + 1 > sizeof(buf))
            {
                printf("find: path too long\n");
                break;
            }
            strcpy(buf, path);
            p = buf + strlen(buf);
            *p++ = '/';
            while (read(fd, &st, sizeof(st)) == sizeof(st))
            {
                if (de.inum == 0)
                    continue;
                memmove(p, de.name, DIRSIZ);
                p[DIRSIZ] = 0;
                if (stat(buf, &st) < 0)
                {
                    printf("find: cannot stat %s\n", buf);
                    continue;
                }
                if (st.type == T_FILE)
                    printf("%s\n", fmtname(buf));
                else if (st.type == T_DIR && *p != '.')
                {
                    ls(p, path);
                }
                else if (strcmp(p, path) == 0) 
                {
                    printf("./%s\n", buf);
                }
            }
        break;
    }
    close(fd);
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        fprintf(2, "find [arg] error!\n");
        exit(1);        
    }
    for (int i=1; i < argc; i++)
    {
        find(argv[i]);
    }

	exit(0);
}
