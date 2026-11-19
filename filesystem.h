#ifndef FILESYSTEM_H
#define FILESYSTEM_H
/*
 * File System Implementation Project
 * Shared constants, structures, and function declarations
 */

 #define NUM_SECTORS 100
 #define ENTRIES_PER_SECTOR 25
 #define DATA_SIZE 504

 //Represents one itme inside a directory
 struct dirEntry{
    char type; //Item type
    char name[9];//File or directory name
    int link;//Stores sector number where files data or subdirectory begins
    short size;//Stores file size

 };
 //Represents a sector being used as a directory
 struct directorySector{
    int back;//Previous sector in same directory chain
    int frwd;//Next sector in same directory chain
    int free;//How many of 25 directory entries are unused/free
    struct dirEntry dir[25];//Creates 25 directory etnries in the directory sector
 };
 // Represents a sector to store file contents
 struct dataSector{
    int backf;//Prevuiys data sectir belobnging to sane file
    int frwdf;//Next data sector belonging to same file
    char userdata[504];//Stores up to 504 characters from the file
 };
 //Represents one sector on the fake disk
 union block{
    struct directorySector directory;//Sector for directory info
    struct dataSector data;//Sector to store file data
 };
 //Represents one avaiable sector in the free sector linked list
 struct nodeType{
    int info;//Stores number of an available sector
    struct nodeType *link;//Pointer to next free sector node
 };

 extern union block sector [NUM_SECTORS];
 extern struct nodeType *freeList;

 void initDir(int sectorNumber, int back, int frwd);//Initizalizes a sector as a directory
 void buildList(void);//Creates initial free sector list sectors 1-99
 int Remove(void);//Removes an available sector from free list and returns its number
 void Insert(int sectorNumber);//Adds a released sector back into the free list
 void releaseList(void);//Frees all the linked list nodes when program exits

 #endif