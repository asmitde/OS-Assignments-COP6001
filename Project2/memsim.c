/*  memsim.c
 *
 *  This program simulates a memory system with VMS and LRU page replacements
 *
 *  Date: 02/04/2016
 */

#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ADDRESS_SPACE_BITS 32
#define PAGE_OFFSET_BITS 12 /* 4096 bytes = 2^12, assuming byte addressing */

typedef enum PageReplacementPolicy
{
    LRU,
    VMS
} PageReplacementPolicy;

typedef enum ExecutionMode
{
    DEBUG,
    QUIET
} ExecutionMode;

typedef struct PageTableEntry
{
    unsigned short isValid;
    unsigned short isDirty;
} PageTableEntry;

typedef struct Node
{
    unsigned int pageNumber;
    struct Node *next;
} Node;

typedef struct List
{
    Node *start, *end;
    int size;
} List;

Node *createNode()
{
    Node *node = (Node *) malloc(sizeof(Node));
    node->pageNumber = 0;
    node->next = NULL;
    
    return node;
}

void moveStartNodeToEnd(List *list)
{
    list->end->next = list->start;
    list->end = list->start;
    list->start = list->start->next;
    list->end->next = NULL;
}

void moveNodeWithPageToEnd(List *list, unsigned int pageNumber)
{
    Node *node = list->start;
    
    while (node->next->pageNumber != pageNumber)
    {
        node = node->next;
    }
    
    list->end->next = node->next;
    list->end = node->next;
    node->next = list->end->next;
    list->end->next = NULL;
}

int findAndRemove(List *list, unsigned int pageNumber)
{
    int found = 0;
    Node *node, *temp;
    
    if (list->start == NULL)
    {
        return found;
    }
    
    if (list->start->pageNumber == pageNumber)
    {
        found = 1;
        node = list->start;
        list->start = node->next;
        free(node);
        list->size--;
    }
    else
    {
        node = list->start;
        while (node->next != NULL && node->next->next != NULL)
        {
            if (node->next->pageNumber == pageNumber)
            {
                found = 1;
                temp = node->next;
                node->next = node->next->next;
                free(temp);
                list->size--;
                
                break;
            }
            
            node = node->next;
        }
    }
    
    return found;
}

/* The LRU algorithm */
void lru(PageTableEntry pageTable[], unsigned int pageNumber, List *residentSet, int nframes, int *diskReads, int *diskWrites, char accessType, ExecutionMode em)
{
    unsigned int pageToBeReplaced;
    Node *node;
    
    if (!pageTable[pageNumber].isValid) /* Case: Page Fault */
    {
        if (residentSet->size < nframes) /* Case: Empty frames available */
        {
            /* Get a frame number for an empty frame */
            node = createNode();
            if (residentSet->size > 0)
            {
                node->next = residentSet->start;
                residentSet->start = node;
            }
            else
            {
                residentSet->start = node;
                residentSet->end = residentSet->start;
            }
            
            residentSet->size++;
            
            if (em == DEBUG)
            {
                printf("\nPage Fault - assigned page to an empty frame");
            }
        }
        else /* Case: Page needs to be replaced */
        {
            /* Get the page number for the page to be replaced */
            pageToBeReplaced = residentSet->start->pageNumber;
            
            if (em == DEBUG)
            {
                printf("\nPage to be replaced: %u", pageToBeReplaced);
            }
            
            /* If page to be replaced is dirty, write page to disk and reset
             * the dirty bit
             */
            if (pageTable[pageToBeReplaced].isDirty)
            {
                (*diskWrites)++;
                pageTable[pageToBeReplaced].isDirty = 0;
                
                if (em == DEBUG)
                {
                    printf("\nPage dirty, wrote back - disk writes: %u", 
                        *diskWrites);
                }
            }
            
            /* Get the frame number to replace and invalidate the page */
            pageTable[pageToBeReplaced].isValid = 0;
            
        }
        
        /* Copy the page from disk to frame in memory */
        residentSet->start->pageNumber = pageNumber;
        (*diskReads)++;
        
        if (em == DEBUG)
        {
            printf("\nPage copied to memory - disk reads: %u", *diskReads);
        }
        
        /* Update the page table entry */
        pageTable[pageNumber].isValid = 1;
    }
    
    /* Access the frame */        
    if (accessType == 'W')
    {
        pageTable[pageNumber].isDirty = 1;
    }
    
    if (em == DEBUG)
    {
        printf("\nPage accessed, %s", (accessType == 'W') ? "Write" : "Read");
    }
    
    /* Move the recently accessed page to the end of the resident set */
    if (residentSet->start->pageNumber == pageNumber)
    {
        moveStartNodeToEnd(residentSet);
    }
    else if (residentSet->end->pageNumber != pageNumber)
    {
        moveNodeWithPageToEnd(residentSet, pageNumber);
    }
}
        
/* The VMS algorithm */
void vms(PageTableEntry pageTable[], unsigned int pageNumber, List *residentSet, List *cleanList, List *dirtyList, int nframes, int *diskReads, int *diskWrites, char accessType, ExecutionMode em)
{
    unsigned int pageToBeReplaced;
    Node *node;
    int found = 0;
    
    if (!pageTable[pageNumber].isValid) /* Case: Page Fault */
    {
        if (residentSet->size == nframes) /* Case: Page needs to be replaced */
        {
            /* Get the page number for the page to be replaced */
            pageToBeReplaced = residentSet->start->pageNumber;
            
            if (em == DEBUG)
            {
                printf("\nPage to be replaced: %u", pageToBeReplaced);
            }
            
            /* If page to be replaced is dirty, transfer page to dirty list */
            if (pageTable[pageToBeReplaced].isDirty)
            {
                if (dirtyList->size == nframes / 2) /* Case: Dirty list is full - Kick out by FIFO and write to disk */
                {
                    node = dirtyList->start;
                    dirtyList->start = dirtyList->start->next;
                    
                    (*diskWrites)++;
                    pageTable[node->pageNumber].isDirty = 0;
                    
                    if (em == DEBUG)
                    {
                        printf("\nPage evicted, wrote back - disk writes: %u", 
                            *diskWrites);
                    }
                    
                    free(node);
                    dirtyList->size--;
                }
                
                node = createNode();
                node->pageNumber = pageToBeReplaced;
                if (dirtyList->size > 0)
                {
                    dirtyList->end->next = node;
                    dirtyList->end = dirtyList->end->next;
                }
                else
                {
                    dirtyList->end = node;
                    dirtyList->start = dirtyList->end;
                }
                
                dirtyList->size++;
                
                if (em == DEBUG)
                {
                    printf("\nPage transferred to dirty list");
                }
            }
            else /* Case: Page to be replaced is clean, transfer to clean list */
            {
                if (cleanList->size == nframes / 2) /* Case: Clean list is full - Kick out by FIFO */
                {
                    node = cleanList->start;
                    cleanList->start = cleanList->start->next;
                    
                    if (em == DEBUG)
                    {
                        printf("\nPage evicted from clean list");
                    }
                    
                    free(node);
                    cleanList->size--;
                }
                
                node = createNode();
                node->pageNumber = pageToBeReplaced;
                if (cleanList->size > 0)
                {
                    cleanList->end->next = node;
                    cleanList->end = cleanList->end->next;
                }
                else
                {
                    cleanList->end = node;
                    cleanList->start = cleanList->end;
                }
                
                cleanList->size++;
                
                if (em == DEBUG)
                {
                    printf("\nPage transferred to clean list");
                }
            }
            
            /* Remove the page and invalidate the page in the page table */
            node = residentSet->start;
            residentSet->start = residentSet->start->next;
            residentSet->size--;
            free(node);
            
            pageTable[pageToBeReplaced].isValid = 0;
        }
        
        /* Search if the page is available in the clean list and remove*/
        found = findAndRemove(cleanList, pageNumber);
        
        /* If not found, search if the page is available in the dirty list and remove */
        if (!found)
        {
            found = findAndRemove(dirtyList, pageNumber);
        }
        
        /* If still not found, copy the page from disk to frame in memory */
        if (!found)
        {            
            (*diskReads)++;
            
            if (em == DEBUG)
            {
                printf("\nPage copied to memory - disk reads: %u", *diskReads);
            }
        }
        
        /* Place the page at the end of the resident set */
        node = createNode();
        node->pageNumber = pageNumber;
        if (residentSet->size > 0)
        {
            residentSet->end->next = node;
            residentSet->end = residentSet->end->next;
        }
        else
        {
            residentSet->end = node;
            residentSet->start = residentSet->end;
        }
        
        residentSet->size++;
        
        /* Update the page table entry */
        pageTable[pageNumber].isValid = 1;
    }
    
    /* Access the frame */        
    if (accessType == 'W')
    {
        pageTable[pageNumber].isDirty = 1;
    }
    
    if (em == DEBUG)
    {
        printf("\nPage accessed, %s", (accessType == 'W') ? "Write" : "Read");
    }
}

int main(int argc, char *argv[])
{
    FILE *tracefile;
    int nframes;
    PageReplacementPolicy prp;
    ExecutionMode em;
    const int PAGE_NUMBER_BITS = ADDRESS_SPACE_BITS - PAGE_OFFSET_BITS;
    const int PAGE_TABLE_SIZE = pow(2, PAGE_NUMBER_BITS);
    const int PAGE_NUMBER_MASK = PAGE_TABLE_SIZE - 1;
    PageTableEntry *pageTable;
    List residentSet, cleanList, dirtyList;
    unsigned int pageNumber;
    unsigned int virtualAddress;
    char accessType;
    int eventsInTrace = 0, diskReads = 0, diskWrites = 0;
    

    /* Parse command-line parameters */
    if (argc != 5)
    {
        printf("\nError: Invalid number of arguments passed\n");
        exit(EXIT_FAILURE);
    }

    /* Open the tracefile in read mode */
    if ((tracefile = fopen(argv[1], "r")) == NULL)
    {
        perror(argv[1]);
        exit(EXIT_FAILURE);
    }

    /* Get the number of frames in physical memory */
    if ((nframes = atoi(argv[2])) <= 0)
    {
        printf("%s: Invalid number of frames\n", argv[2]);
        exit(EXIT_FAILURE);
    }

    /* Get the page replacement policy */
    if (strcmp(argv[3], "lru") == 0)
    {
        prp = LRU;
    }
    else if (strcmp(argv[3], "vms") == 0)
    {
        prp = VMS;
    }
    else
    {
        printf("%s: Invalid page replacement policy\n", argv[3]);
        exit(EXIT_FAILURE);
    }

    /* Get the execution mode */
    if (strcmp(argv[4], "debug") == 0)
    {
        em = DEBUG;
    }
    else if (strcmp(argv[4], "quiet") == 0)
    {
        em = QUIET;
    }
    else
    {
        printf("%s: Invalid execution mode\n", argv[4]);
        exit(EXIT_FAILURE);
    }

    /* Create the page table */
    if ((pageTable = (PageTableEntry *) calloc(PAGE_TABLE_SIZE, 
        sizeof(PageTableEntry))) == NULL)
    {
        printf("Error: Unable to create page table\n");
        exit(EXIT_FAILURE);
    }
    
    /* Create the resident set */
    residentSet.size = 0; residentSet.start = NULL; residentSet.end = NULL;
    
    /* Create the clean list */
    cleanList.size = 0; cleanList.start = NULL; cleanList.end = NULL;
    
    /* Create the dirty list */
    dirtyList.size = 0; dirtyList.start = NULL; dirtyList.end = NULL;
    


    /* Run the trace */
    while (fscanf(tracefile, "%x %c", &virtualAddress, &accessType) != EOF)
    {
        eventsInTrace++;

        /* Consult the page table to check if the page is present in memory */
        pageNumber = (virtualAddress >> PAGE_OFFSET_BITS) & PAGE_NUMBER_MASK;

        if (em == DEBUG)
        {
            printf("\n\n<== Event# %d ==>", eventsInTrace);
            printf("\nVirtual Address: %x", virtualAddress);
        }

        if (prp == LRU)
        {
            lru(pageTable, pageNumber, &residentSet, nframes, &diskReads, &diskWrites, accessType, em);
        }
        else
        {
            vms(pageTable, pageNumber, &residentSet, &cleanList, &dirtyList, nframes, &diskReads, &diskWrites, accessType, em);
        }
    }
    
    /* Close the file and do necessary cleanups */
    fclose(tracefile);
    free(pageTable);
    
    /* Print the simulation statistics */
    printf("Total memory frames: %d", nframes);
    printf("\nEvents in trace: %d", eventsInTrace);
    printf("\nTotal disk reads: %d", diskReads);
    printf("\nTotal disk writes: %d\n", diskWrites);
    
    return 0;
}


