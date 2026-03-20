#include "../../libpi/rpi.h"
#include "../filesystems/boot_server.h"
#include "../heap/allocator.h"

static char heap[64 * 1500];
static size_t heap_size = sizeof(heap);
static void *heap_start = heap;

void notmain(void) {
    myinit(heap_start, heap_size);

    pi_dirent_t root;
    fat32_fs_t fs = initialize_fs(&root);

    int deleted = delete_boot_file(0, &root, &fs);
    int deleted1 = delete_boot_file(1, &root, &fs);
    int deleted2 = delete_boot_file(2, &root, &fs);
    if (deleted) {
        printk("Deleted SERVER0.bin\n");
    } else {
        printk("SERVER0.bin not found or already deleted\n");
    }
    if (deleted1) {
        printk("Deleted SERVER1.bin\n");
    } else {
        printk("SERVER1.bin not found or already deleted\n");
    }
        if (deleted2) {
        printk("Deleted SERVER2.bin\n");
    } else {
        printk("SERVER2.bin not found or already deleted\n");
    }
}
