/*  myls.c
 *
 *  This program emulates the ls command in linux with limited functionality.
 *
 *  Usage: ./myls [-l] [<dirname>]
 *
 *  Author: Asmit De | U72377278
 *  Date: 03/29/2016
 */

#include <dirent.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
#define TIME_SIZE 13

int main(int argc, char *argv[])
{
    struct stat fileInfo;
    struct dirent *dirEntry;
    struct passwd *owner;
    struct group *grp;
    struct tm *timestamp;
    DIR *directory;
    int longList = 0;
    char permissions[11], pwd[BUFFER_SIZE], filePath[BUFFER_SIZE], time[TIME_SIZE];
    

    /* Check if the second argument is the long list option */
    if (argc > 1 && !strcmp(argv[1], "-l"))
    {
        longList = 1;
    }
    
    if (argc == 1 || (argc == 2 && longList == 1)) /* No directory name specified */
    {
        /* Get the current working directory and handle errors */
        if (getcwd(pwd, BUFFER_SIZE - 1) == NULL)
        {
            perror("getcwd");
            exit(EXIT_FAILURE);
        }
    }
    else /* Directory name specified */
    {
        /* Set the working directory to the specified directory */
        strcpy(pwd, argv[argc - 1]);
    }
    
    /* Open directory for reading and handle errors */
    if ((directory = opendir(pwd)) == NULL)
    {
        perror("opendir");
        exit(EXIT_FAILURE);
    }
    
    /* Iterate over all the entries in the directory structure */
    while ((dirEntry = readdir(directory)) != NULL)
    {
        /* Skip the hidden files and entries for current and previous directories */
        if (dirEntry->d_name[0] == '.') continue;
        
        /* Extract file statictics and handle errors */
        sprintf(filePath, "%s/%s", pwd, dirEntry->d_name);
        if (stat(filePath, &fileInfo) == -1)
        {
            perror("stat");
            continue;
        }
        
        /* Print the long listing if option is given */
        if (longList == 1)
        {
            /* Display file permissions */
            permissions[0] = (S_ISDIR(fileInfo.st_mode)) ? 'd' : '-';
            permissions[1] = (fileInfo.st_mode & S_IRUSR) ? 'r' : '-';
            permissions[2] = (fileInfo.st_mode & S_IWUSR) ? 'w' : '-';
            permissions[3] = (fileInfo.st_mode & S_IXUSR) ? 'x' : '-';
            permissions[4] = (fileInfo.st_mode & S_IRGRP) ? 'r' : '-';
            permissions[5] = (fileInfo.st_mode & S_IWGRP) ? 'w' : '-';
            permissions[6] = (fileInfo.st_mode & S_IXGRP) ? 'x' : '-';
            permissions[7] = (fileInfo.st_mode & S_IROTH) ? 'r' : '-';
            permissions[8] = (fileInfo.st_mode & S_IWOTH) ? 'w' : '-';
            permissions[9] = (fileInfo.st_mode & S_IXOTH) ? 'x' : '-';
            permissions[10] = '\0';
            printf("%s ", permissions);
            
            /* Display reference (link) count for file */
            printf("%ld ", (long) fileInfo.st_nlink);
            
            /* Display owner of file */
            owner = getpwuid(fileInfo.st_uid);
            printf("%s ", owner->pw_name);
            
            /* Display group owner of file */
            grp = getgrgid(fileInfo.st_gid);
            printf("%s ", grp->gr_name);
            
            /* Display file size */
            printf("%lld ", (long long) fileInfo.st_size);
            
            /* Display last modified time for file */
            timestamp = localtime(&fileInfo.st_mtime);
            strftime(time, TIME_SIZE, "%b %e %R", timestamp);
            printf("%s ", time);
        }
        
        /* Display the entry */
        printf("%s\n", dirEntry->d_name);
    }
    
     /* Close the directory pointer after finishing read */
     closedir(directory);
    
    return 0;
}
