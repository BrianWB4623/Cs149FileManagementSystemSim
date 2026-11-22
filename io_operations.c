#include <stdio.h>
#include <string.h>
#include "filesystem.h"

/* Member 3: file access operations. */
struct openFileEntry openFiles[MAX_OPEN_FILES];

/*
 * Opens a file in the root directory (sector 0).
 * The returned integer is the open-file handle, or -1 on failure.
 */
int openFile(const char *name){
    if(name == NULL || strlen(name) == 0 || strlen(name) > 8){
        printf("OPEN failed: file name must contain 1 to 8 characters.\n");
        return -1;
    }

    int directoryEntry = -1;
    for(int i = 0; i < ENTRIES_PER_SECTOR; i++){
        if(sector[0].directory.dir[i].name[0] != '\0' &&
           strcmp(sector[0].directory.dir[i].name, name) == 0){
            directoryEntry = i;
            break;
        }
    }

    if(directoryEntry == -1){
        printf("OPEN failed: file '%s' does not exist.\n", name);
        return -1;
    }

    for(int i = 0; i < MAX_OPEN_FILES; i++){
        if(!openFiles[i].used){
            openFiles[i].used = 1;
            openFiles[i].directorySector = 0;
            openFiles[i].directoryEntry = directoryEntry;
            openFiles[i].position = 0;
            printf("Opened '%s' with handle %d.\n", name, i);
            return i;
        }
    }

    printf("OPEN failed: maximum number of open files reached.\n");
    return -1;
}

/*
 * Closes an active open-file handle and makes its table entry available
 * for another OPEN operation.
 */
int closeFile(int handle){
    if(handle < 0 || handle >= MAX_OPEN_FILES || !openFiles[handle].used){
        printf("CLOSE failed: invalid file handle.\n");
        return -1;
    }

    openFiles[handle].used = 0;
    openFiles[handle].directorySector = 0;
    openFiles[handle].directoryEntry = 0;
    openFiles[handle].position = 0;
    printf("Closed file handle %d.\n", handle);
    return 0;
}
