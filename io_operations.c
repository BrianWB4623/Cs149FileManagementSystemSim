#include <stdio.h>
#include <stdlib.h>
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

/* Reads up to count bytes from the current file position. */
int readFile(int count, char *output, int outputSize){
    if(!currentFile.isOpen){
        printf("READ failed: no file is open.\n");
        return -1;
    }
    if(currentFile.mode != 'I' && currentFile.mode != 'U'){
        printf("READ failed: file is not open for reading.\n");
        return -1;
    }
    if(count < 0 || output == NULL || outputSize <= 0){
        printf("READ failed: invalid count or output buffer.\n");
        return -1;
    }

    struct dirEntry *file =
        &sector[currentFile.directorySector].directory.dir[
            currentFile.directoryEntry];
    int remaining = file->size - currentFile.position;
    int bytesToRead = count < remaining ? count : remaining;
    if(bytesToRead >= outputSize){
        bytesToRead = outputSize - 1;
    }

    for(int i = 0; i < bytesToRead; i++){
        int absolutePosition = currentFile.position + i;
        int blockIndex = absolutePosition / DATA_SIZE;
        int offset = absolutePosition % DATA_SIZE;
        int dataSector = dataSectorAt(file->link, blockIndex);
        output[i] = sector[dataSector].data.userdata[offset];
    }

    output[bytesToRead] = '\0';
    currentFile.position += bytesToRead;
    printf("Read %d bytes from '%s'.\n", bytesToRead, file->name);
    return bytesToRead;
}

/* Moves the current file position without changing the file size. */
int seekFile(int position){
    if(!currentFile.isOpen){
        printf("SEEK failed: no file is open.\n");
        return -1;
    }

    struct dirEntry *file =
        &sector[currentFile.directorySector].directory.dir[
            currentFile.directoryEntry];
    if(position < 0 || position > file->size){
        printf("SEEK failed: position must be between 0 and %d.\n",
               file->size);
        return -1;
    }

    currentFile.position = position;
    printf("File position moved to %d.\n", position);
    return 0;
}

/* Processes one command. Returns 1 when the EXIT command is entered. */
int processCommand(char *line){
    if(line == NULL){
        return 0;
    }

    line[strcspn(line, "\n")] = '\0';
    if(strcmp(line, "EXIT") == 0){
        return 1;
    }
    if(strcmp(line, "CLOSE") == 0){
        closeFile();
        return 0;
    }

    char mode;
    char name[9];
    if(sscanf(line, "OPEN %c %8s", &mode, name) == 2){
        openFile(mode, name);
        return 0;
    }

    int count;
    if(sscanf(line, "READ %d", &count) == 1){
        if(count < 0){
            printf("READ failed: byte count cannot be negative.\n");
            return 0;
        }
        char *output = malloc((size_t)count + 1);
        if(output == NULL){
            printf("READ failed: unable to allocate output buffer.\n");
            return 0;
        }
        int result = readFile(count, output, count + 1);
        if(result >= 0){
            printf("Data read: %s\n", output);
        }
        free(output);
        return 0;
    }

    if(sscanf(line, "SEEK %d", &count) == 1){
        seekFile(count);
        return 0;
    }

    if(strncmp(line, "WRITE ", 6) == 0){
        char *numberStart = line + 6;
        char *dataStart;
        long requested = strtol(numberStart, &dataStart, 10);
        if(dataStart == numberStart || requested < 0 || requested > 32767){
            printf("Usage: WRITE <byte count> <data>\n");
            return 0;
        }
        if(*dataStart == ' '){
            dataStart++; /* Remove the separator after the byte count. */
        }
        if((int)strlen(dataStart) < requested){
            printf("WRITE failed: fewer data bytes were provided than requested.\n");
            return 0;
        }
        writeFile((int)requested, dataStart);
        return 0;
    }

    printf("Unknown command.\n");
    return 0;
}

/* Runs the Member 3 command loop until EXIT is entered. */
void commandLoop(void){
    char line[512];
    while(1){
        printf("fs> ");
        if(fgets(line, sizeof(line), stdin) == NULL){
            break;
        }
        if(processCommand(line)){
            break;
        }
    }
}
