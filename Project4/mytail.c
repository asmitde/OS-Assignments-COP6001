/*  mytail.c
 *
 *  This program emulates the tail command in linux with limited functionality.
 *
 *  Usage: ./mytail [-<n>] <filename>
 *
 *  Author: Asmit De | U72377278
 *  Date: 03/30/2016
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h> 
#include <sys/types.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
#define NUM_LINES 10

int main(int argc, char *argv[])
{
    struct stat fileInfo;
    int numLines, newlines, bytesRead, fd;
    char filepath[BUFFER_SIZE], byte[2];
    off_t offset;
    
    /* Handle argument errors */
    if (argc < 2)
    {
        printf("mytail: missing operand(s)\n");
        exit(EXIT_FAILURE);
    }
    
    if (argv[1][0] == '-') /* Number of lines is specified as argument */
    {
        /* Get the number of lines and handle errors */
        if ((numLines = atoi(&argv[1][1])) == 0)
        {
            printf("mytail: invalid number of lines -- '%s'\n", &argv[1][1]);
            exit(EXIT_FAILURE);
        }
        
        /* Handle argument errors */
        if (argc == 2)
        {
            printf("mytail: missing filepath\n");
            exit(EXIT_FAILURE);
        }
        
        /* Get the specified filepath */
        strcpy(filepath, argv[2]);
    }
    else /* Number of lines not specified */
    {
        /* Use default number of lines */
        numLines = NUM_LINES;
        
        /* Get the specified filepath */
        strcpy(filepath, argv[1]);
    }
    
    /* Open the fie in read mode */
    if ((fd = open(filepath, O_RDONLY)) == -1)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }
    
    /* Extract file statictics and handle errors */
    if (stat(filepath, &fileInfo) == -1)
    {
        perror("stat");
        exit(EXIT_FAILURE);
    }
    
    /* Initialize variables to start reading from the last character of file */
    offset = (off_t) fileInfo.st_size;
    newlines = 0;
    bytesRead = 0;
    
    /* Start reading from the last character of the file keep looping back */
    while (newlines <= numLines)
    {
        /* Decrement and reposition the read cursur one place back */
        if(lseek(fd, --offset, SEEK_SET) == -1)
        {
            perror("lseek");
            exit(EXIT_FAILURE);
        }
        
        /* Read a character */
        if(read(fd, byte, 1) == -1)
        {
            perror("read");
            exit(EXIT_FAILURE);
        }
        
        /* Keep a count of the newlines and the number of bytes read */
        if (!strcmp(byte, "\n"))
        {
            newlines++;
        }
        bytesRead++;
        
        /* If the cursur has reached the beginning of file, stop reading */
        if(bytesRead == (int) fileInfo.st_size)
        {
            if(lseek(fd, 0, SEEK_SET) == -1)
            {
                perror("lseek");
                exit(EXIT_FAILURE);
            }
            
            break;
        }
    }
    
    /* Read each character from this point till EOF and write it to STDOUT */
    while(read(fd, byte, 1) != 0)
    {
        if(write(1, byte, 1) == -1)
        {
            perror("write");
            exit(EXIT_FAILURE);
        }
    }
    
    /* Close the file */
    close(fd);
    
    return 0;
}
