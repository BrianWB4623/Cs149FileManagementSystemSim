#include "filesystem.h"
#include <stdio.h>
#include <string.h>

//will display the contents of directory

void listDirectory(int sectorNumber) {

    //goes through all of the 25 directories 
    //checks if entry has name the prints name 
    for (int i = 0; i < ENTRIES_PER_SECTOR; i++) {
        if (sector[sectorNumber].directory.dir[i].name[0] != '\0' ) {
            printf("%s\n", sector[sectorNumber].directory.dir[i].name);
        }
    }

}

// searches for a filename in a directory sector
// returns entry index, or -1 if not found
int findEntry(int sectorNumber, const char *name) {

    for (int i = 0; i < ENTRIES_PER_SECTOR; i++) {

        if (strcmp(sector[sectorNumber].directory.dir[i].name,
                   name) == 0) {
            return i;
        }
    }

    return -1;
}

int createFile(int sectorNumber, const char *name) {

    // Check if the filename is valid (1 to 8 characters)
    if (name == NULL || strlen(name) == 0 || strlen(name) > 8) {
        printf("Invalid filename.\n");
        return -1;
    }

    // Check if the file already exists
    if (findEntry(sectorNumber, name) != -1) {
        printf("File already exists.\n");
        return -1;
    }

    // Search for an empty directory entry
    for (int i = 0; i < ENTRIES_PER_SECTOR; i++) {

        if (sector[sectorNumber].directory.dir[i].name[0] == '\0') {

            // Store the new file information
            struct dirEntry *entry =
                &sector[sectorNumber].directory.dir[i];

            entry->type = 'U';
            strcpy(entry->name, name);
            entry->link = 0;
            entry->size = 0;

            // updates the number of available entries
            sector[sectorNumber].directory.free--;

            printf("File '%s' created successfully.\n", name);
            return 0;
        }
    }

    printf("Directory is full.\n");
    return -1;
}

int deleteFile(int sectorNumber, const char *name) {

    // makes sure the filename is valid
    if (name == NULL) {
        printf("Invalid filename.\n");
        return -1;
    }

    // searches for the file
    int index = findEntry(sectorNumber, name);

    // does it exist
    if (index == -1) {
        printf("File not found.\n");
        return -1;
    }

    // get the directory entry
    struct dirEntry *entry =
        &sector[sectorNumber].directory.dir[index];

    // selectively deletes empty files (temp)
    if (entry->type != 'U' ||
        entry->link != 0 ||
        entry->size != 0) {
        printf("Cannot delete this entry yet.\n");
        return -1;
    }

    // clears the file info
    entry->type = 'F';
    entry->name[0] = '\0';
    entry->link = 0;
    entry->size = 0;

    // increase the number of available entries
    sector[sectorNumber].directory.free++;

    printf("File '%s' deleted successfully.\n", name);

    return 0;
}

void goToRoot(int *currentDirectory) {

    // sets the current directory to sector 0
    *currentDirectory = 0;

    printf("Returned to root directory.\n");
}
