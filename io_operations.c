#include <stdio.h>
#include <string.h>
#include "filesystem.h"

/* Member 3: file access operations. */
struct openFileInfo currentFile = {0, 0, 0, '\0', 0};

/* Opens one regular file in the root directory. */
int openFile(char mode, const char *name){
    if(mode != 'I' && mode != 'O' && mode != 'U'){
        printf("OPEN failed: mode must be I, O, or U.\n");
        return -1;
    }
    if(currentFile.isOpen){
        printf("OPEN failed: another file is already open.\n");
        return -1;
    }
    if(name == NULL || strlen(name) == 0 || strlen(name) > 8){
        printf("OPEN failed: file name must contain 1 to 8 characters.\n");
        return -1;
    }

    int directoryEntry = -1;
    for(int i = 0; i < ENTRIES_PER_SECTOR; i++){
        if(sector[0].directory.dir[i].type == 'U' &&
           strcmp(sector[0].directory.dir[i].name, name) == 0){
            directoryEntry = i;
            break;
        }
    }

    if(directoryEntry == -1){
        printf("OPEN failed: regular file '%s' does not exist.\n", name);
        return -1;
    }

    currentFile.isOpen = 1;
    currentFile.directorySector = 0;
    currentFile.directoryEntry = directoryEntry;
    currentFile.mode = mode;
    currentFile.position = mode == 'O'
        ? sector[0].directory.dir[directoryEntry].size : 0;
    printf("Opened '%s' in mode %c.\n", name, mode);
    return 0;
}

/* Closes the one currently open file. */
int closeFile(void){
    if(!currentFile.isOpen){
        printf("CLOSE failed: no file is open.\n");
        return -1;
    }

    currentFile.isOpen = 0;
    currentFile.directorySector = 0;
    currentFile.directoryEntry = 0;
    currentFile.mode = '\0';
    currentFile.position = 0;
    printf("Closed the open file.\n");
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

/* Writes exactly count bytes at the current file position. */
int writeFile(int count, const char *data){
    if(!currentFile.isOpen){
        printf("WRITE failed: no file is open.\n");
        return -1;
    }
    if(currentFile.mode != 'O' && currentFile.mode != 'U'){
        printf("WRITE failed: file is not open for writing.\n");
        return -1;
    }
    if(count < 0){
        printf("WRITE failed: byte count cannot be negative.\n");
        return -1;
    }
    if(data == NULL || (int)strlen(data) < count){
        printf("WRITE failed: fewer data bytes were provided than requested.\n");
        return -1;
    }

    struct dirEntry *file =
        &sector[currentFile.directorySector].directory.dir[
            currentFile.directoryEntry];
    int position = currentFile.position;
    int requiredBytes = position + count;

    if(requiredBytes > 32767 || ensureDataCapacity(file, requiredBytes) == -1){
        printf("WRITE failed: not enough free disk space.\n");
        return -1;
    }

    for(int i = 0; i < count; i++){
        int absolutePosition = position + i;
        int blockIndex = absolutePosition / DATA_SIZE;
        int offset = absolutePosition % DATA_SIZE;
        int dataSector = dataSectorAt(file->link, blockIndex);
        sector[dataSector].data.userdata[offset] = data[i];
    }

    currentFile.position = requiredBytes;
    if(requiredBytes > file->size){
        file->size = (short)requiredBytes;
    }
    printf("Wrote %d bytes to '%s'.\n", count, file->name);
    return count;
}
