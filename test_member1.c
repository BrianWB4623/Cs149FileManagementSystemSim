
#include <stdio.h>
#include "filesystem.h"

// Functions from file_operations.c
void listDirectory(int sectorNumber);
void goToRoot(int *currentDirectory);
int createFile(int sectorNumber, const char *name);
int deleteFile(int sectorNumber, const char *name);

int main(void) {

    // Initialize the root directory
    initDir(0, -1, -1);

    // Create two files
    printf("Test 1: Create files\n");
    createFile(0, "hello");
    createFile(0, "notes");

    // Display files before deletion
    printf("\nTest 2: Before deletion\n");
    listDirectory(0);

    // Delete one file
    printf("\nTest 3: Delete hello\n");
    deleteFile(0, "hello");

    // Display files after deletion
    printf("\nTest 4: After deletion\n");
    listDirectory(0);

    // Try deleting a nonexistent file
    printf("\nTest 5: Delete nonexistent file\n");
    deleteFile(0, "missing");

    // Check remaining free entries
    printf("\nFree entries: %d\n",
           sector[0].directory.free);

    
    // Test ROOT command
    printf("\nTest 6: Return to root\n");

    int currentDirectory = 5;

    printf("Before ROOT: Sector %d\n", currentDirectory);

    goToRoot(&currentDirectory);

    printf("After ROOT: Sector %d\n", currentDirectory);

    return 0;
}
