#ifndef CONFIG_H
#define CONFIG_H

#define PIZZAOS_TOTAL_INTERRUPTS 256

#define KERNEL_CODE_SEGMENT 0x08
#define KERNEL_DATA_SEGMENT 0x10

//100mb heap size
#define PIZZAOS_HEAP_SIZE_BYTES 104857600
#define PIZZAOS_HEAP_BLOCK_SIZE 4096
#define PIZZAOS_HEAP_ADDRESS 0x01000000 
#define PIZZAOS_HEAP_TABLE_ADDRESS 0x00007E00


#define PIZZAOS_SECTOR_SIZE 512
#define PIZZAOS_SECTOR_TYPE 512

#endif