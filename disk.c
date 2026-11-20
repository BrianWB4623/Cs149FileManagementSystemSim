/*
 * File System Implementation Project
 * Disk and free-sector management
 *
 * Responsible member: Member 2
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "filesystem.h"

// 100 sector Fake disk
union block sector[100];
//Free sector linked list to keep track of which disk sector are unused
struct nodeType *freeList=NULL;

void initDir(int sectorNumber, int back, int frwd){
    sector[sectorNumber].directory.back=back;
    sector[sectorNumber].directory.frwd=frwd;
    sector[sectorNumber].directory.free=ENTRIES_PER_SECTOR;
    for(int i=0;i<ENTRIES_PER_SECTOR;i++){
        sector[sectorNumber].directory.dir[i].type='F';
        sector[sectorNumber].directory.dir[i].name[0]='\0';
        sector[sectorNumber].directory.dir[i].link=0;
        sector[sectorNumber].directory.dir[i].size=0;
    }
}

void buildList(void){
    freeList=NULL;
    //Build all free nodes starting at 99
    for(int i=NUM_SECTORS-1;i>=1;i--){
        Insert(i);
    }
}
void Insert(int sectorNumber){
    if(sectorNumber<1){//Invalid sectornUmber
        return;
    }
    if(sectorNumber>=100){//invalid Sector Number
        return;
    }
    struct nodeType *newNode;
    newNode=malloc(sizeof(struct nodeType));
    if(newNode==NULL){
        printf("Failed to create node");
        exit(1);
    }
    newNode->info=sectorNumber;
    newNode->link=freeList;
    freeList=newNode;
}
int Remove(void){
    struct nodeType *removedNode;
    int sectorNumber;
    if(freeList==NULL){
        return -1;
    }
    removedNode=freeList;
    sectorNumber=removedNode->info;
    freeList=removedNode->link;
    free(removedNode);
    return sectorNumber;
}
void releaseList(void){
    while(freeList){
        struct nodeType *temporary=freeList;
        freeList=freeList->link;
        free(temporary);
    }
}