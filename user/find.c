#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

// this function takes a directory and a file name, constructs a full path
char* fmtname(char* path, char* dir, char* name)
{
    static char buf[DIRSIZ+1];
    char* p;
    int i = 0;

    // Construct full path.
    if (dir != 0 && name != 0) 
    {
        // Copy characters from 'dir' to 'path' and add /
        while (*dir != '\0') 
            path[i++] = *dir++;

        path[i++] = '/';

        // Copy characters from 'name' to 'path'. and ad \0 to the end
        while (*name != '\0') 
            path[i++] = *name++;
            
        path[i] = '\0';
    }

    // Find first character after last slash.
    for (p = path + strlen(path); p >= path && *p != '/'; p--);
    p++;

    // Return blank-padded name.
    if (strlen(p) >= DIRSIZ)
        return p;

    // copy p to buf and return the formatted name
    memmove(buf, p, strlen(p) + 1);
    memset(buf + strlen(p), ' ', DIRSIZ - strlen(p));
    return buf;
}

void find(int fd, char* dir, char* name)
{
    struct dirent de;
    // find all files with the same name as name in the current directory dir
    while (read(fd, &de, sizeof(de)) == sizeof(de))
    {
        // check if dirent was properly copied / its in the directory
        if (de.inum == 0)
            continue;
        if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
            continue;

        // construct the full path using dir, and file name  
        char path[512];
        fmtname(path, dir, de.name);

        // get the stat of the file
        struct stat st;
        if (stat(path, &st) < 0)
        {
            printf("find [stat] error!\n");
            continue;
        }
        // if type is a file and is the searched name print to console else search in sub directories
        if (st.type == T_FILE && (strcmp(de.name, name) == 0))
            printf("%s\n", path);
        else if (st.type == T_DIR) 
        {
            int sub;
            if ((sub = open(path, O_RDONLY)) < 0)
            {
                fprintf(2, "find [open] error!\n");
                continue;
            }
            find(sub, path, name);
        }
    }
}

// this functions checks all edge cases, if error, return -1 else return 0
int checkFind(char* dir, char* name)
{
    if (strlen(dir) > DIRSIZ || strlen(name) > DIRSIZ)
    {
        fprintf(2, "find [length] error!\n");
        return -1;  
    }

    int fd;
    if ((fd = open(dir, O_RDONLY)) < 0)
    {
        fprintf(2, "find [open] error!\n");
        return -1;
    }

    struct stat st;
    if (fstat(fd, &st) < 0)
    {
        fprintf(2, "find [stat] error!\n");
        return -1;
    }

    if (st.type != T_DIR)
        printf("not a dir!\n");
    else
        find(fd, dir, name);

    return 0;
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