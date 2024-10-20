#ifndef DISK_H
#define DISK_H
#include "../fs/file.h"

typedef unsigned int PIZZAOS_DISK_TYPE;

//represents a real physical hard disk
#define PIZZAOS_DISK_TYPE_REAL 0
struct  disk
{
    /* data */
    PIZZAOS_DISK_TYPE type;
    int sector_size;
    struct filesystem* filesystem;
    void* fs_private;
    int id;
};


void disk_search_and_init();
struct disk* disk_get(int index);
int disk_read_block(struct disk* idisk , unsigned int lba, int total , void* buf);

#endif