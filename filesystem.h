#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#define NUM_SECTORS 100
#define ENTRIES_PER_SECTOR 25
#define DATA_SIZE 504

struct dirEntry{
    char type;
    char name[9];
    int link;
    short size;
};

struct directorySector{
    int back;
    int frwd;
    int free;
    struct dirEntry dir[ENTRIES_PER_SECTOR];
};

struct dataSector{
    int backf;
    int frwdf;
    char userdata[DATA_SIZE];
};

union block{
    struct directorySector directory;
    struct dataSector data;
};

struct nodeType{
    int info;
    struct nodeType *link;
};

/* The assignment allows only one file to be open at a time. */
struct openFileInfo{
    int isOpen;
    int directorySector;
    int directoryEntry;
    char mode;
    int position;
};

extern union block sector[NUM_SECTORS];
extern struct nodeType *freeList;
extern struct openFileInfo currentFile;

void initDir(int sectorNumber, int back, int frwd);
void buildList(void);
int Remove(void);
void Insert(int sectorNumber);
void releaseList(void);

int openFile(char mode, const char *name);
int closeFile(void);
int writeFile(int count, const char *data);
int readFile(int count, char *output, int outputSize);
int seekFile(int position);
int processCommand(char *line);
void commandLoop(void);

#endif
