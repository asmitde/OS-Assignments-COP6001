/*  mytree.c
 *
 *  This program emulates the ls command in linux with limited functionality.
 *
 *  Usage: ./mytree [<dirname>]
 *
 *  Author: Asmit De | U72377278
 *  Date: 03/30/2016
 */

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

/* Recursive method to generate the file tree */
void traverseRecursive(char *pwd, char *entryPrefix, long *numDirs, long *numFiles)
{
    struct stat fileInfo;
    struct dirent *dirEntry;
    DIR *directory;
    char filePath[BUFFER_SIZE];
    long dirOffset;
    int lastEntry;
    
    /* Open directory for reading and handle errors */
    if ((directory = opendir(pwd)) == NULL)
    {
        printf(" [error opening dir]");
        return;
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
        
        /* Display the entry */
        dirOffset = telldir(directory);
        if (readdir(directory) != NULL)
        {
            printf("\n%s|-- %s", entryPrefix, dirEntry->d_name);
            lastEntry = 0;
        }
        else
        {
            printf("\n%s`-- %s", entryPrefix, dirEntry->d_name);
            lastEntry = 1;
        }
        seekdir(directory, dirOffset);
        
        
        /* If entry is a directory, traverse recursively */
        if (S_ISDIR(fileInfo.st_mode))
        {
            (*numDirs)++;
            (lastEntry) ? strcat(entryPrefix, "    ") : strcat(entryPrefix, "|   ");
            traverseRecursive(filePath, entryPrefix, numDirs, numFiles);
        }
        else
        {
            (*numFiles)++;
        }
    }
    
    /* Close the directory pointer after finishing read */
    closedir(directory);
    entryPrefix[strlen(entryPrefix) - 4] = '\0';
}

int main(int argc, char *argv[])
{
    char pwd[BUFFER_SIZE], entryPrefix[BUFFER_SIZE];
    long treeDepth, numDirs, numFiles;

    if (argc < 2) /* No directory name specified */
    {
        /* Set the current working directory */
        strcpy(pwd, ".");
    }
    else /* Directory name specified */
    {
        /* Set the working directory to the specified directory */
        strcpy(pwd, argv[1]);
    }
    
    /* Print the working directory */
    printf("%s", pwd);
    
    /* Initialize the function parameters */
    treeDepth = 0;
    numDirs = 0; 
    numFiles = 0;
    strcpy(entryPrefix, "");
    
    /* Call recursively to traverse directory tree */
    traverseRecursive(pwd, entryPrefix, &numDirs, &numFiles);
    
    /* Print numeber of directories and files */
    printf("\n\n%ld directories, %ld files\n", numDirs, numFiles);
    
    return 0;
}
