#include "kernel.h"
#include <stdint.h>
#include <stddef.h>
#include "idt/idt.h"
#include "io/io.h"
#include "memory/heap/kheap.h"
#include "memory/memory.h"
#include "memory/paging/paging.h"
#include "disk/disk.h"
#include "fs/pparser.h"
#include "string/string.h"
#include "disk/streamer.h"
#include "fs/file.h"
#include "task/tss.h"
#include "gdt/gdt.h"
#include "config.h"


uint16_t* video_mem = 0;
uint16_t terminal_row = 0;
uint16_t terminal_col = 0;
static struct paging_4gb_chunk* kernel_chunk = 0;


// ACTUALLY MAKES THE CHAR
uint16_t terminal_make_char (char c , char color){
    return (color<< 8) | c; // x89 is little endian , thus in memory char followed by color will be stored.
}

// DOES NOT KEEP TRACK OF CURSOR POSITION
void terminal_putchar(uint16_t x , uint16_t y , char c , char color){
    const size_t index = y * VGA_WIDTH + x;
    video_mem[index] = terminal_make_char(c, color);
}

//KEEPS TRACK OF CURSOR POSITION
void terminal_writechar(char c , char color){
    if (c == '\n'){
        terminal_col = 0;
        terminal_row ++;
    }
    else{  
        terminal_putchar(terminal_col , terminal_row , c , color);
        if (++terminal_col == VGA_WIDTH){
            terminal_col = 0;
            terminal_row ++;
        }
    }
}

// INITIALIZES TERMINAL
void terminal_initialize(void) {
    terminal_row = 0;
    terminal_col = 0;

    video_mem = (uint16_t*)(0xB8000);

    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            terminal_putchar(x, y, ' ', 0);
        }
    }
}


// PRINTS STRING TO TERMINAL
void print(const char* str){
    size_t len = strlen(str);
    for (size_t i = 0; i < len; i++)
        terminal_writechar(str[i], 15);
}

void panic (const char* msg)
{
    print (msg);
    while (1) {}
}

struct tss tss;
struct gdt gdt_real[PIZZAOS_TOTAL_GDT_SEGMENTS];

struct gdt_structured gdt_structured[PIZZAOS_TOTAL_GDT_SEGMENTS] = {
    {.base = 0x00, .limit = 0x00, .type = 0x00},                // NULL Segment
    {.base = 0x00, .limit = 0xffffffff, .type = 0x9a},           // Kernel code segment
    {.base = 0x00, .limit = 0xffffffff, .type = 0x92},            // Kernel data segment
    {.base = 0x00, .limit = 0xffffffff, .type = 0xf8},              // User code segment
    {.base = 0x00, .limit = 0xffffffff, .type = 0xf2},             // User data segment
    {.base = (uint32_t)&tss, .limit=sizeof(tss), .type = 0xE9}      // TSS Segment
};

// MAIN FUNCTION (CALLED BY KERNEL.ASM)
void kernel_main() {
    terminal_initialize();
    print("Hello World \n This is my os \n");
    
    memset(gdt_real, 0x00, sizeof(gdt_real));
    gdt_structured_to_gdt(gdt_real, gdt_structured, PIZZAOS_TOTAL_GDT_SEGMENTS);
    // Load the gdt
    gdt_load(gdt_real, sizeof(gdt_real));

    kheap_init();

    fs_init();

    disk_search_and_init();
    print("Initializing IDT...\n");
    idt_init();
    print("IDT initialized.\n");

    // Setup the TSS
    memset(&tss, 0x00, sizeof(tss));
    tss.esp0 = 0x600000;
    tss.ss0 = KERNEL_DATA_SELECTOR;
    // Load the TSS
    tss_load(0x28);


    // void* ptr1 = kmalloc(50);
    // void* ptr2 = kmalloc(5000);
    // void* ptr3 = kmalloc(5600);
    // if(ptr1){}
    // kfree(ptr1);
    // void* ptr4 = kmalloc(50);
    // if (ptr2 || ptr3 || ptr4){

    // }
// gdb
// add-symbol-file ./build/kernelfull.o 0x100000
// target remote | qemu-system-i386 -hda ./bin/os.bin -S -gdb stdio
// break kernel.c : 90
//continue

        // Setup paging
    kernel_chunk = paging_new_4gb(PAGING_IS_WRITEABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);

     // Switch to kernel paging chunk
    paging_switch(paging_4gb_chunk_get_directory(kernel_chunk));

    // Enable paging
    enable_paging();

    print("Enabling interrupts...\n");
    enable_interrupts();
    print("Interrupts enabled.\n");

    int fd = fopen("0:/hello.txt", "r");

    if (fd) {
        print("File hello.txt opened\n");
        char buf[14];
        fseek(fd , 2, SEEK_SET);
        fread(buf,11,1,fd);
        buf[13]=0x00;
        print(buf);
        print("\n");

        struct file_stat s;
        fstat(fd, &s);
        fclose(fd);
        print("\nss");
    }

    print("Kernel initialization complete.\n");
    while(1) {}

}
