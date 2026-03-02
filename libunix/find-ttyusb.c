// engler, cs140e: your code to find the tty-usb device on your laptop.
#include <assert.h>
#include <fcntl.h>
#include <string.h>

#include "libunix.h"
#include <sys/stat.h>

#define _SVID_SOURCE
#include <dirent.h>
static const char *ttyusb_prefixes[] = {
    "ttyUSB",	// linux
    "ttyACM",   // linux
    // "cu.SLAB_USB", // mac os
    "cu.usbserial", // mac os
    // if your system uses another name, add it.

};

static int filter(const struct dirent *d) {
    // scan through the prefixes, returning 1 when you find a match.
    // 0 if there is no match.
    if (strncmp(d->d_name, ttyusb_prefixes[2], strlen(ttyusb_prefixes[2])) == 0) {
        return 1;
    } 
    return 0;
}

// static int filter_time(const struct dirent *d) {
//     // scan through the prefixes, returning 1 when you find a match.
//     // 0 if there is no match.
//     if (strncmp(d->d_name, ttyusb_prefixes[3], strlen(ttyusb_prefixes[3])) == 0) {
//         return 1;
//     } 
//     return 0;
// }

// find the TTY-usb device (if any) by using <scandir> to search for
// a device with a prefix given by <ttyusb_prefixes> in /dev
// returns:
//  - device name.
// error: panic's if 0 or more than 1 devices.
char *find_ttyusb(void) {
    // use <alphasort> in <scandir>
    // return a malloc'd name so doesn't corrupt.
    struct dirent **names;
    char *folder = "/dev";
    int num = scandir(folder, &names, filter, alphasort);
    if (num != 1) {
        panic("found 0 or more than 1 entry!");
    }
    

    // asprintf(&path, "%s/%s", folder, names[0]->d_name);

   return strdupf("/dev/%s", names[0]->d_name);
}

// return the most recently mounted ttyusb (the one
// mounted last).  use the modification time 
// returned by state.
char *find_ttyusb_last(void) {
    struct dirent **names;
    char *folder = "/dev";
    int num = scandir("/dev", &names, filter, alphasort);
    if (num < 0) return NULL;
    time_t last = 0;
    int index = 0;
    char path[10000];
    struct stat stats;
    for (int i = 0; i < num; i++) {
        snprintf(path, sizeof(path), "%s/%s", folder, names[i]->d_name);
        stat(path, &stats);
        if (stats.st_mtime > last) {
            last = stats.st_mtime;
            index = i;
        }
    }
    char *result;
    asprintf(&result, "%s/%s", folder, names[index]->d_name);
    return result;
    
}

// return the oldest mounted ttyusb (the one mounted
// "first") --- use the modification returned by
// stat()
char *find_ttyusb_first(void) {
    struct dirent **names;
    char *folder = "/dev";
    int num = scandir("/dev", &names, filter, alphasort);
    if (num < 0) return NULL;
    time_t first = 0;
    int index = -1;
    char path[10000];
    struct stat stats;
    for (int i = 0; i < num; i++) {
        
        snprintf(path, sizeof(path), "%s/%s", folder, names[i]->d_name);
        stat(path, &stats);
        if (index < 0 || stats.st_mtime < first) {
            first = stats.st_mtime;
            index = i;
        }
    }
    char *result;
    asprintf(&result, "%s/%s", folder, names[index]->d_name);
    return result;
}
