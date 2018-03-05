/*  mystat.c
 *
 *  This program emulates the stat command in linux with limited functionality.
 *
 *  Usage: ./mystat <filename1> [<filename2>...]
 *
 *  Author: Asmit De | U72377278
 *  Date: 03/29/2016
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

int main(int argc, char *argv[])
{
    struct stat fileInfo;
    int i;
    char permissions[11];
    unsigned long mode;
    
    /* Handle argument errors */
    if (argc < 2)
    {
        printf("mystat: missing operand\n");
        exit(EXIT_FAILURE);
    }
    
    /* Iterate over all the filenames passed as arguments */
    for (i = 1; i < argc; i++)
    {
        /* Extract file statictics and handle errors */
        if (stat(argv[i], &fileInfo) == -1)
        {
            perror("stat");
            continue;
        }
        
        /* Display file name */
        printf("  File: `%s'\n", argv[i]);
        
        /* Display file size */
        printf("Size: %lld\n", (long long) fileInfo.st_size);
        
        /* Display number of blocks allocated for file */
        printf("Blocks: %lld\n", (long long) fileInfo.st_blocks);
        
        /* Display reference (link) count for file */
        printf("Links: %ld\n", (long) fileInfo.st_nlink);
        
        /* Display file permissions */
        mode = 0;
        permissions[0] = (S_ISDIR(fileInfo.st_mode)) ? 'd' : '-';
        permissions[1] = (mode |= fileInfo.st_mode & S_IRUSR) ? 'r' : '-';
        permissions[2] = (mode |= fileInfo.st_mode & S_IWUSR) ? 'w' : '-';
        permissions[3] = (mode |= fileInfo.st_mode & S_IXUSR) ? 'x' : '-';
        permissions[4] = (mode |= fileInfo.st_mode & S_IRGRP) ? 'r' : '-';
        permissions[5] = (mode |= fileInfo.st_mode & S_IWGRP) ? 'w' : '-';
        permissions[6] = (mode |= fileInfo.st_mode & S_IXGRP) ? 'x' : '-';
        permissions[7] = (mode |= fileInfo.st_mode & S_IROTH) ? 'r' : '-';
        permissions[8] = (mode |= fileInfo.st_mode & S_IWOTH) ? 'w' : '-';
        permissions[9] = (mode |= fileInfo.st_mode & S_IXOTH) ? 'x' : '-';
        permissions[10] = '\0';
        mode |= (fileInfo.st_mode & S_ISUID) |
                (fileInfo.st_mode & S_ISGID) |
                (fileInfo.st_mode & S_ISVTX);
        printf("Access: (%04lo/%s)\n", (unsigned long) mode, permissions);
        
        /* Display file Inode */
        printf("Inode: %ld\n\n", (long) fileInfo.st_ino);
    }
    
    return 0;
}
