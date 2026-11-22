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

/* Returns the data-sector number at a zero-based position in a file chain. */
static int dataSectorAt(int firstSector, int blockIndex){
    int current = firstSector;
    for(int i = 0; i < blockIndex && current != 0; i++){
        current = sector[current].data.frwdf;
    }
    return current;
}

/* Adds data sectors until the file has enough room for requiredBytes. */
static int ensureDataCapacity(struct dirEntry *file, int requiredBytes){
    int requiredBlocks = (requiredBytes + DATA_SIZE - 1) / DATA_SIZE;
    if(requiredBlocks == 0){
        return 0;
    }

    if(file->link == 0){
        int first = Remove();
        if(first == -1){
            return -1;
        }
        file->link = first;
        sector[first].data.backf = 0;
        sector[first].data.frwdf = 0;
        memset(sector[first].data.userdata, 0, DATA_SIZE);
    }

    int current = file->link;
    int blockCount = 1;
    while(blockCount < requiredBlocks){
        if(sector[current].data.frwdf == 0){
            int next = Remove();
            if(next == -1){
                return -1;
            }
            sector[current].data.frwdf = next;
            sector[next].data.backf = current;
            sector[next].data.frwdf = 0;
            memset(sector[next].data.userdata, 0, DATA_SIZE);
        }
        current = sector[current].data.frwdf;
        blockCount++;
    }
    return 0;
}

/* Writes data into the file's linked data sectors. */
int writeFile(int handle, const char *data){
    if(handle < 0 || handle >= MAX_OPEN_FILES || !openFiles[handle].used){
        printf("WRITE failed: invalid file handle.\n");
        return -1;
    }
    if(data == NULL){
        printf("WRITE failed: data cannot be NULL.\n");
        return -1;
    }

    struct dirEntry *file =
        &sector[openFiles[handle].directorySector].directory.dir[
            openFiles[handle].directoryEntry];
    int dataLength = (int)strlen(data);
    int position = openFiles[handle].position;
    int requiredBytes = position + dataLength;

    if(requiredBytes > 32767 || ensureDataCapacity(file, requiredBytes) == -1){
        printf("WRITE failed: not enough free disk space.\n");
        return -1;
    }

    for(int i = 0; i < dataLength; i++){
        int absolutePosition = position + i;
        int blockIndex = absolutePosition / DATA_SIZE;
        int offset = absolutePosition % DATA_SIZE;
        int dataSector = dataSectorAt(file->link, blockIndex);
        sector[dataSector].data.userdata[offset] = data[i];
    }

    openFiles[handle].position = requiredBytes;
    if(requiredBytes > file->size){
        file->size = (short)requiredBytes;
    }
    printf("Wrote %d bytes to '%s'.\n", dataLength, file->name);
    return dataLength;
}
