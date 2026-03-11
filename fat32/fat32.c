#include "rpi.h"
#include "fat32.h"
#include "fat32-helpers.h"
#include "pi-sd.h"

// Print extra tracing info when this is enabled.  You can and should add your
// own.
static int trace_p = 1; 
static int init_p = 0;

fat32_boot_sec_t boot_sector;


fat32_fs_t fat32_mk(mbr_partition_ent_t *partition) {
  demand(!init_p, "the fat32 module is already in use\n");
  // TODO: Read the boot sector (of the partition) off the SD card.
  
  fat32_boot_sec_t *boot = pi_sec_read(partition->lba_start, 1);
  boot_sector = *boot;

  // TODO: Verify the boot sector (also called the volume id, `fat32_volume_id_check`)
  fat32_volume_id_check(boot);

  // TODO: Read the FS info sector (the sector immediately following the boot
  // sector) and check it (`fat32_fsinfo_check`, `fat32_fsinfo_print`)
  
  assert(boot_sector.info_sec_num == 1);
  struct fsinfo *info = pi_sec_read(partition->lba_start + boot_sector.info_sec_num, 1);
  fat32_fsinfo_check(info);
  // fat32_fsinfo_print("Printing info sector:", info);
  // fat32_volume_id_print("Printing vol info:", boot);
  
  // END OF PART 2
  // The rest of this is for Part 3:

  // TODO: calculate the fat32_fs_t metadata, which we'll need to return.
  unsigned lba_start = partition->lba_start;; // from the partition
  unsigned fat_begin_lba = lba_start + boot_sector.reserved_area_nsec; // the start LBA + the number of reserved sectors
  unsigned cluster_begin_lba = boot_sector.nfats * boot_sector.nsec_per_fat +  fat_begin_lba; // the beginning of the FAT, plus the combined length of all the FATs
  unsigned sec_per_cluster = boot_sector.sec_per_cluster; // from the boot sector
  unsigned root_first_cluster = boot_sector.first_cluster; // from the boot sector
  unsigned n_entries = boot_sector.nsec_per_fat; // from the boot sector
  

  

  /*
   * TODO: Read in the entire fat (one copy: worth reading in the second and
   * comparing).
   *
   * The disk is divided into clusters. The number of sectors per
   * cluster is given in the boot sector byte 13. <sec_per_cluster>
   *
   * The File Allocation Table has one entry per cluster. This entry
   * uses 12, 16 or 28 bits for FAT12, FAT16 and FAT32.
   *
   * Store the FAT in a heap-allocated array.
   */

  
  uint32_t *fat = (uint32_t *)kmalloc(boot_sector.nsec_per_fat * boot_sector.bytes_per_sec);
  pi_sd_read(fat, fat_begin_lba, boot_sector.nsec_per_fat);

  // Create the FAT32 FS struct with all the metadata
  fat32_fs_t fs = (fat32_fs_t) {
    .lba_start = lba_start,
      .fat_begin_lba = fat_begin_lba,
      .cluster_begin_lba = cluster_begin_lba,
      .sectors_per_cluster = sec_per_cluster,
      .root_dir_first_cluster = root_first_cluster,
      .fat = fat,
      .n_entries = n_entries,
  };

  if (trace_p) {
    trace("begin lba = %d\n", fs.fat_begin_lba);
    trace("cluster begin lba = %d\n", fs.cluster_begin_lba);
    trace("sectors per cluster = %d\n", fs.sectors_per_cluster);
    trace("root dir first cluster = %d\n", fs.root_dir_first_cluster);
  }

  init_p = 1;
  return fs;
}

// Given cluster_number, get lba.  Helper function.
static uint32_t cluster_to_lba(fat32_fs_t *f, uint32_t cluster_num) {
  // trace("cluster num is %d\n", cluster_num);
  assert(cluster_num >= 2);
  // TODO: calculate LBA from cluster number, cluster_begin_lba, and
  // sectors_per_cluster
  unsigned lba = f->cluster_begin_lba + (cluster_num - 2) * f->sectors_per_cluster;
  // if (trace_p) trace("cluster %x to lba: %x\n", cluster_num, lba);
  return lba;
}

pi_dirent_t fat32_get_root(fat32_fs_t *fs) {
  demand(init_p, "fat32 not initialized!");
  // TODO: return the information corresponding to the root directory (just
  // cluster_id, in this case)
  
  return (pi_dirent_t) {
    .name = "",
      .raw_name = "",
      .cluster_id = fs->root_dir_first_cluster, // fix this
      .is_dir_p = 1,
      .nbytes = 0,
  };
}

// Given the starting cluster index, get the length of the chain.  Helper
// function.
static uint32_t get_cluster_chain_length(fat32_fs_t *fs, uint32_t start_cluster) {
  // TODO: Walk the cluster chain in the FAT until you see a cluster where
  // `fat32_fat_entry_type(cluster) == LAST_CLUSTER`.  Count the number of
  // clusters.
  uint32_t count = 1;
  uint32_t cluster_number = start_cluster;

  while (fat32_fat_entry_type(cluster_number) != LAST_CLUSTER) {
    count++;
    cluster_number = fs->fat[cluster_number];
  }
  return count;
}

// Given the starting cluster index, read a cluster chain into a contiguous
// buffer.  Assume the provided buffer is large enough for the whole chain.
// Helper function.
static void read_cluster_chain(fat32_fs_t *fs, uint32_t start_cluster, uint8_t *data) {
  // TODO: Walk the cluster chain in the FAT until you see a cluster where
  // fat32_fat_entry_type(cluster) == LAST_CLUSTER.  For each cluster, copy it
  // to the buffer (`data`).  Be sure to offset your data pointer by the
  // appropriate amount each time.
  uint32_t cluster_number = start_cluster;
  uint32_t index = 0;
  // trace("start cluster is %x\n", cluster_number);


  while (fat32_fat_entry_type(cluster_number) != LAST_CLUSTER) {
    uint32_t lba = cluster_to_lba(fs, cluster_number);
    pi_sd_read(data + index * fs->sectors_per_cluster * NBYTES_PER_SECTOR, lba, fs->sectors_per_cluster);
    
    cluster_number = fs->fat[cluster_number];
   
    index++;
  }
}

// Converts a fat32 internal dirent into a generic one suitable for use outside
// this driver.
static pi_dirent_t dirent_convert(fat32_dirent_t *d) {
  pi_dirent_t e = {
    .cluster_id = fat32_cluster_id(d),
    .is_dir_p = d->attr == FAT32_DIR,
    .nbytes = d->file_nbytes,
  };
  // can compare this name
  memcpy(e.raw_name, d->filename, sizeof d->filename);
  // for printing.
  fat32_dirent_name(d,e.name);
  return e;
}

// Gets all the dirents of a directory which starts at cluster `cluster_start`.
// Return a heap-allocated array of dirents.
static fat32_dirent_t *get_dirents(fat32_fs_t *fs, uint32_t cluster_start, uint32_t *dir_n) {
  // TODO: figure out the length of the cluster chain (see
  // `get_cluster_chain_length`)
  uint32_t chain_len = get_cluster_chain_length(fs, cluster_start);
  // trace("chain length is %d\n", chain_len);
  *dir_n = chain_len * fs->sectors_per_cluster * NDIR_PER_SEC;


  // TODO: allocate a buffer large enough to hold the whole directory
  //fat32_dirent_t *buf = kmalloc((*dir_n) * sizeof(fat32_dirent_t));
  uint8_t *data = kmalloc(NBYTES_PER_SECTOR * chain_len * fs->sectors_per_cluster);

  // TODO: read in the whole directory (see `read_cluster_chain`)
  read_cluster_chain(fs, cluster_start, data);
 //buf = (fat32_dirent_t*)data;

  return (fat32_dirent_t *)data;
}

pi_directory_t fat32_readdir(fat32_fs_t *fs, pi_dirent_t *dirent) {
  demand(init_p, "fat32 not initialized!");
  demand(dirent->is_dir_p, "tried to readdir a file!");
  // TODO: use `get_dirents` to read the raw dirent structures from the disk
  uint32_t n_dirents;
  fat32_dirent_t *dirents = get_dirents(fs, dirent->cluster_id, &n_dirents);
  
  // trace("got to pi dirent\n");
  // TODO: allocate space to store the pi_dirent_t return values
  pi_dirent_t *dir = kmalloc(sizeof(pi_dirent_t) * n_dirents);

  // TODO: iterate over the directory and create pi_dirent_ts for every valid
  // file.  Don't include empty dirents, LFNs, or Volume IDs.  You can use
  // `dirent_convert`.

  uint32_t i = 0;
  // uint32_t cluster_number = dirent->cluster_id;

  for (int j = 0; j < n_dirents; j++) {
      if (fat32_dirent_free(&dirents[j]) || (fat32_dirent_is_lfn(&dirents[j]) || dirents[j].attr & FAT32_VOLUME_LABEL)) {
        continue;
      }
      dir[i++] = dirent_convert(&dirents[j]);   
  }
  

  // TODO: create a pi_directory_t using the dirents and the number of valid
  // dirents we found

  return (pi_directory_t) {
    .dirents = dir,
    .ndirents = i,
  };
}

static int find_dirent_with_name(fat32_dirent_t *dirents, int n, char *filename) {
  // TODO: iterate through the dirents, looking for a file which matches the
  // name; use `fat32_dirent_name` to convert the internal name format to a
  // normal string.
  // unimplemented();
  // trace("looking for %s\n",filename);
  for (int j = 0; j < n; j++) {
      
      char buf[16];
      fat32_dirent_name(&dirents[j], buf);
      // trace("file name is %s\n", dirents[j].filename);
      if (strcmp(buf, filename) == 0) {
        // trace("file name is %s\n", dirents[j].filename);
        return j;
      }

  }
  return -1;
}

pi_dirent_t *fat32_stat(fat32_fs_t *fs, pi_dirent_t *directory, char *filename) {
  demand(init_p, "fat32 not initialized!");
  demand(directory->is_dir_p, "tried to use a file as a directory");

  // TODO: use `get_dirents` to read the raw dirent structures from the disk
  // unimplemented();
  uint32_t n_dirents;
  fat32_dirent_t *dirents = get_dirents(fs, directory[0].cluster_id, &n_dirents);
  
  // trace("looing thru %d entries\n", n_dirents);

  // TODO: Iterate through the directory's entries and find a dirent with the
  // provided name.  Return NULL if no such dirent exists.  You can use
  // `find_dirent_with_name` if you've implemented it.
  // unimplemented();
  // trace("looing for index\n");

  uint32_t index = find_dirent_with_name(dirents, n_dirents, filename);
  if (index == -1) {
    return NULL;
  }

  // TODO: allocate enough space for the dirent, then convert
  // (`dirent_convert`) the fat32 dirent into a Pi dirent.

  pi_dirent_t *dirent = kmalloc(sizeof(pi_dirent_t));
  *dirent = dirent_convert(&dirents[index]);
  return dirent;
}

pi_file_t *fat32_read(fat32_fs_t *fs, pi_dirent_t *directory, char *filename) {
  // This should be pretty similar to readdir, but simpler.
  demand(init_p, "fat32 not initialized!");
  demand(directory->is_dir_p, "tried to use a file as a directory!");

  // TODO: read the dirents of the provided directory and look for one matching the provided name
  // unimplemented();
  uint32_t n_dirents;
  uint32_t cluster_start = directory[0].cluster_id;
  fat32_dirent_t *dirents = get_dirents(fs, cluster_start, &n_dirents);
  

  uint32_t index = find_dirent_with_name(dirents, n_dirents, filename);
  if (index == -1) {
    return NULL;
  }

  // TODO: figure out the length of the cluster chain
  // unimplemented();
  pi_dirent_t dirent = dirent_convert(&dirents[index]);
  // trace("cluster id is %x\n", dirent.cluster_id);
  uint32_t chain_len = get_cluster_chain_length(fs, dirent.cluster_id);
  // trace("chain len is %d\n", chain_len);

  // TODO: allocate a buffer large enough to hold the whole file
  // unimplemented();
  uint32_t bytes = NBYTES_PER_SECTOR * chain_len * fs->sectors_per_cluster;
  uint8_t *data = kmalloc(bytes);
  

  // TODO: read in the whole file (if it's not empty)
  // unimplemented();
  read_cluster_chain(fs, dirent.cluster_id, data);

  // pi_dirent_t *dirent = fat32_stat(fs, directory, filename);

  // TODO: fill the pi_file_t
  pi_file_t *file = kmalloc(sizeof(pi_file_t));
  *file = (pi_file_t) {
    .data = data,
    .n_data = dirent.nbytes,
    .n_alloc = bytes,
  };
  return file;
}

/******************************************************************************
 * Everything below here is for writing to the SD card (Part 7/Extension).  If
 * you're working on read-only code, you don't need any of this.
 ******************************************************************************/

static uint32_t find_free_cluster(fat32_fs_t *fs, uint32_t start_cluster) {
  // TODO: loop through the entries in the FAT until you find a free one
  // (fat32_fat_entry_type == FREE_CLUSTER).  Start from cluster 3.  Panic if
  // there are none left.
  if (start_cluster < 3) start_cluster = 3;
  // unimplemented();
  uint32_t entries = (fs->n_entries * NBYTES_PER_SECTOR) / sizeof(uint32_t);

  for (uint32_t i = start_cluster; i < entries; i++) {
    if (fat32_fat_entry_type(fs->fat[i]) == FREE_CLUSTER) {
      trace("found free cluster %d\n", i);
      return i;
    }
  }

  if (trace_p) trace("failed to find free cluster from %d\n", start_cluster);
  panic("No more clusters on the disk!\n");
}

static void write_fat_to_disk(fat32_fs_t *fs) {
  // TODO: Write the FAT to disk.  In theory we should update every copy of the
  // FAT, but the first one is probably good enough.  A good OS would warn you
  // if the FATs are out of sync, but most OSes just read the first one without
  // complaining.
  if (trace_p) trace("syncing FAT\n");
  // unimplemented();
  pi_sd_write(fs->fat, fs->fat_begin_lba, fs->n_entries);
}

// Given the starting cluster index, write the data in `data` over the
// pre-existing chain, adding new clusters to the end if necessary.
static void write_cluster_chain(fat32_fs_t *fs, uint32_t start_cluster, uint8_t *data, uint32_t nbytes) {
  // Walk the cluster chain in the FAT, writing the in-memory data to the
  // referenced clusters.  If the data is longer than the cluster chain, find
  // new free clusters and add them to the end of the list.
  // Things to consider:
  //  - what if the data is shorter than the cluster chain?
  //  - what if the data is longer than the cluster chain?
  //  - the last cluster needs to be marked LAST_CLUSTER
  //  - when do we want to write the updated FAT to the disk to prevent
  //  corruption?
  //  - what do we do when nbytes is 0?
  //  - what about when we don't have a valid cluster to start with?
  //
  //  This is the main "write" function we'll be using; the other functions
  //  will delegate their writing operations to this.

  // TODO: As long as we have bytes left to write and clusters to write them
  // to, walk the cluster chain writing them out.
  // unimplemented();
  uint32_t cluster = start_cluster;
  if (nbytes == 0) {
    trace("no bytes to write\n");
    return;
  }
  
  // TODO: If we run out of bytes to write before using all the clusters, mark
  // the final cluster as "LAST_CLUSTER" in the FAT, then free all the clusters
  // later in the chain.
  while (fat32_fat_entry_type(fs->fat[cluster]) != LAST_CLUSTER) {
    uint32_t next_cluster = fs->fat[cluster];
    if (nbytes > 0) {
      pi_sd_write(data, cluster_to_lba(fs, cluster), fs->sectors_per_cluster);
      data += fs->sectors_per_cluster * NBYTES_PER_SECTOR;
      if (fs->sectors_per_cluster * NBYTES_PER_SECTOR >= nbytes) {
        fs->fat[cluster] = LAST_CLUSTER;
        nbytes = 0;
      } else {
        nbytes -= fs->sectors_per_cluster * NBYTES_PER_SECTOR;
      }
    } else {
      fs->fat[cluster] = FREE_CLUSTER;
    }
    cluster = next_cluster;
  }
  if (fat32_fat_entry_type(fs->fat[cluster]) == LAST_CLUSTER) {
    if (nbytes > 0) {
      pi_sd_write(data, cluster_to_lba(fs, cluster), fs->sectors_per_cluster);
      if (fs->sectors_per_cluster * NBYTES_PER_SECTOR >= nbytes) {
        nbytes = 0;
      } else {
        nbytes -= fs->sectors_per_cluster * NBYTES_PER_SECTOR;
      }
      data += fs->sectors_per_cluster * NBYTES_PER_SECTOR;
    } else{
      fs->fat[cluster] = FREE_CLUSTER;
    }
  }
  
  // TODO: If we run out of clusters to write to, find free clusters using the
  // FAT and continue writing the bytes out.  Update the FAT to reflect the new
  // cluster.
  while (nbytes > 0) {
    uint32_t free_cluster = find_free_cluster(fs, 3);
    fs->fat[cluster] = free_cluster;
    cluster = free_cluster;
    fs->fat[cluster] = LAST_CLUSTER;
    pi_sd_write(data, cluster_to_lba(fs, cluster), fs->sectors_per_cluster);
    data += fs->sectors_per_cluster * NBYTES_PER_SECTOR;
    if (fs->sectors_per_cluster * NBYTES_PER_SECTOR >= nbytes) {
        nbytes = 0;
    } else {
      nbytes -= fs->sectors_per_cluster * NBYTES_PER_SECTOR;
    }
  }
  
  // unimplemented();
  // TODO: Ensure that the last cluster in the chain is marked "LAST_CLUSTER".
  // The one exception to this is if we're writing 0 bytes in total, in which
  // case we don't want to use any clusters at all.
  // unimplemented();
  assert(fat32_fat_entry_type(fs->fat[cluster]) == LAST_CLUSTER);
  write_fat_to_disk(fs);

}

int fat32_rename(fat32_fs_t *fs, pi_dirent_t *directory, char *oldname, char *newname) {
  // TODO: Get the dirents `directory` off the disk, and iterate through them
  // looking for the file.  When you find it, rename it and write it back to
  // the disk (validate the name first).  Return 0 in case of an error, or 1
  // on success.
  // Consider:
  //  - what do you do when there's already a file with the new name?
  demand(init_p, "fat32 not initialized!");
  if (trace_p) trace("renaming %s to %s\n", oldname, newname);
  if (!fat32_is_valid_name(newname)) return 0;

  // TODO: get the dirents and find the right one
  pi_dirent_t * new_result = fat32_stat(fs, directory, newname);
  if (new_result != NULL) {
    return 0;
  }

  uint32_t n_dirents;
  fat32_dirent_t * dirents = get_dirents(fs, directory->cluster_id, &n_dirents);
  
  // TODO: Iterate through the directory's entries and find a dirent with the
  // provided name.  Return NULL if no such dirent exists.  You can use
  // `find_dirent_with_name` if you've implemented it.
  uint32_t i = find_dirent_with_name(dirents, n_dirents, oldname);
  if (i == -1) {
    return 0;
  }

  filename_to_8dot3(dirents[i].filename, newname);
  // TODO: update the dirent's name


  trace("writing with %d bytes at cluster %d\n", n_dirents * sizeof(fat32_dirent_t), directory->cluster_id);
  // TODO: write out the directory, using the existing cluster chain (or
  // appending to the end); implementing `write_cluster_chain` will help
  write_cluster_chain(fs, directory->cluster_id, (uint8_t *)dirents, n_dirents * sizeof(fat32_dirent_t));
  // write_cluster_chain(fs, directory->cluster_id, ); 
  return 1;

}

static void filename_to_8dot3(char out[11], const char *filename) {
  memset(out, ' ', 11);

  const char *dot = strchr(filename, '.');

  if(dot) {
      int base_len = dot - filename;
      if(base_len > 8) base_len = 8;
      memcpy(out, filename, base_len);

      const char *ext = dot + 1;
      int ext_len = strlen(ext);
      if(ext_len > 3) ext_len = 3;
      memcpy(out + 8, ext, ext_len);
  } else {
      int base_len = strlen(filename);
      if(base_len > 8) base_len = 8;
      memcpy(out, filename, base_len);
  }

}


// Create a new directory entry for an empty file (or directory).
pi_dirent_t *fat32_create(fat32_fs_t *fs, pi_dirent_t *directory, char *filename, int is_dir) {
  demand(init_p, "fat32 not initialized!");
  if (trace_p) trace("creating %s\n", filename);
  if (!fat32_is_valid_name(filename)) return NULL;

  // TODO: read the dirents and make sure there isn't already a file with the
  // same name
  // unimplemented();
  pi_dirent_t * old_result = fat32_stat(fs, directory, filename);
  if (old_result != NULL) {
    panic("already exists\n");
  }

  // TODO: look for a free directory entry and use it to store the data for the
  // new file.  If there aren't any free directory entries, either panic or
  // (better) handle it appropriately by extending the directory to a new
  // cluster.
  // When you find one, update it to match the name and attributes
  // specified; set the size and cluster to 0.
  uint32_t n_dirents;
  uint32_t cluster_start = directory->cluster_id;
  fat32_dirent_t * dirents = get_dirents(fs, cluster_start, &n_dirents);
  fat32_dirent_t * created = NULL;
  for (int i = 0; i < n_dirents; i ++) {
    if (fat32_dirent_free(&dirents[i])) {
      filename_to_8dot3(dirents[i].filename, filename);
      if (is_dir) {
        dirents[i].attr = FAT32_DIR;
      }
      else {
        dirents[i].attr = FAT32_SYSTEM_FILE;
      }
      dirents[i].file_nbytes = 0;
      dirents[i].hi_start = 0;
      dirents[i].lo_start = 0;
      created = &dirents[i];
      trace("created at index %d\n", i);
      break;
    }
  }
  if (created == NULL) {
    panic("no space for new file\n");
  }

  // TODO: write out the updated directory to the disk
  trace("writing with %d bytes at cluster %d\n", n_dirents * sizeof(fat32_dirent_t), directory->cluster_id);
  write_cluster_chain(fs, directory->cluster_id, (uint8_t *)dirents, n_dirents * sizeof(fat32_dirent_t));

  // TODO: convert the dirent to a `pi_dirent_t` and return a (kmalloc'ed)
  // pointer
  pi_dirent_t *dirent = kmalloc(sizeof(pi_dirent_t));
  *dirent = dirent_convert(created);
  return dirent;
}

// Delete a file, including its directory entry.
int fat32_delete(fat32_fs_t *fs, pi_dirent_t *directory, char *filename) {
  demand(init_p, "fat32 not initialized!");
  if (trace_p) trace("deleting %s\n", filename);
  if (!fat32_is_valid_name(filename)) return 0;
  // TODO: look for a matching directory entry, and set the first byte of the
  // name to 0xE5 to mark it as free
  uint32_t n_dirents;
  fat32_dirent_t * dirents = get_dirents(fs, directory->cluster_id, &n_dirents);
  
  // TODO: Iterate through the directory's entries and find a dirent with the
  // provided name.  Return NULL if no such dirent exists.  You can use
  // `find_dirent_with_name` if you've implemented it.
  uint32_t i = find_dirent_with_name(dirents, n_dirents, filename);
  if (i == -1) {
    trace("did not find file\n");
    return 0;
  }
  dirents[i].filename[0] = 0xE5;
  // TODO: free the clusters referenced by this dirent
  // unimplemented();
  uint32_t cluster =  fat32_cluster_id(&dirents[i]);
  uint32_t length = 1;
  while (cluster >= 2 && fat32_fat_entry_type(fs->fat[cluster]) != LAST_CLUSTER) {
    trace("at cluster %d\n", cluster);
    uint32_t next_cluster = fs->fat[cluster];
    fs->fat[cluster] = FREE_CLUSTER;
    cluster = next_cluster;
    length ++;
  }
  if (cluster >= 2) {
    fs->fat[cluster] = FREE_CLUSTER;
  }
  trace("freed %d clusters\n", length);
  write_fat_to_disk(fs);

  // TODO: write out the updated directory to the disk
  trace("writing with %d bytes at cluster %d\n", n_dirents * sizeof(fat32_dirent_t), directory->cluster_id);
  write_cluster_chain(fs, directory->cluster_id, (uint8_t *)dirents, n_dirents * sizeof(fat32_dirent_t));
  return 1;
}

int fat32_truncate(fat32_fs_t *fs, pi_dirent_t *directory, char *filename, unsigned length) {
  demand(init_p, "fat32 not initialized!");
  if (trace_p) trace("truncating %s\n", filename);

  uint32_t n_dirents;
  fat32_dirent_t * dirents = get_dirents(fs, directory->cluster_id, &n_dirents);

  uint32_t i = find_dirent_with_name(dirents, n_dirents, filename);
  if (i == -1) {
    trace("did not find file\n");
    return 0;
  }

  // TODO: edit the directory entry of the file to list its length as `length` bytes,
  // then modify the cluster chain to either free unused clusters or add new
  // clusters.
  uint32_t cluster = 0;
  uint32_t old_length = dirents[i].file_nbytes;
  dirents[i].file_nbytes = length;
  if (length == 0) {
    dirents[i].hi_start = 0;
    dirents[i].lo_start = 0;
    trace("edge case where length is 0\n");
    uint32_t old_cluster = fat32_cluster_id(&dirents[i]);
    while (old_cluster >= 2 && fat32_fat_entry_type(fs->fat[old_cluster]) != LAST_CLUSTER) {
      uint32_t next = fs->fat[old_cluster];
      fs->fat[old_cluster] = FREE_CLUSTER;
      old_cluster = next;
  }
  if (old_cluster >= 2)
      fs->fat[old_cluster] = FREE_CLUSTER;
  } else if (old_length == 0 && length > 0) {
    trace("edge case where the old length was 0\n");
    cluster = find_free_cluster(fs, 3);
    fs->fat[cluster] = LAST_CLUSTER;
    dirents[i].lo_start = cluster & 0xFFFF;
    dirents[i].hi_start = cluster >> 16;
    if (fs->sectors_per_cluster * NBYTES_PER_SECTOR >= length) {
      length = 0;
    } else {
      length -= fs->sectors_per_cluster * NBYTES_PER_SECTOR;
    }
  }
  else {
    trace("no edge case\n");
    cluster = fat32_cluster_id(&dirents[i]);
  }
  
  while (fat32_fat_entry_type(fs->fat[cluster]) != LAST_CLUSTER && cluster >= 2) {
    trace("currently dealing with cluster %d \n", cluster);
    uint32_t next_cluster = fs->fat[cluster];
    if (length > 0) {
      if (fs->sectors_per_cluster * NBYTES_PER_SECTOR >= length) {
        fs->fat[cluster] = LAST_CLUSTER;
        length = 0;
      } else {
        length -= fs->sectors_per_cluster * NBYTES_PER_SECTOR;
      }
    } else {
      fs->fat[cluster] = FREE_CLUSTER;
    }
    cluster = next_cluster;
  }

  if (cluster >= 2 && fat32_fat_entry_type(fs->fat[cluster]) == LAST_CLUSTER) {
    if (length > 0) {
        if (fs->sectors_per_cluster * NBYTES_PER_SECTOR >= length) {
            length = 0;
        } else {
            length -= fs->sectors_per_cluster * NBYTES_PER_SECTOR;
        }
    } else {
        fs->fat[cluster] = FREE_CLUSTER;
    }
}
  
 
  trace("finished dealing with everything that already existed\n");
  while (length > 0) {
    uint32_t free_cluster = find_free_cluster(fs, 3);
    fs->fat[cluster] = free_cluster;
    cluster = free_cluster;
    fs->fat[cluster] = LAST_CLUSTER;
    if (fs->sectors_per_cluster * NBYTES_PER_SECTOR >= length) {
        length = 0;
    } else {
      length -= fs->sectors_per_cluster * NBYTES_PER_SECTOR;
    }
  }

  write_fat_to_disk(fs);

  // Consider: what if the file we're truncating has length 0? what if we're
  // truncating to length 0?
  // unimplemented();

  // TODO: write out the directory entry
  trace("writing with %d bytes at cluster %d\n", n_dirents * sizeof(fat32_dirent_t), directory->cluster_id);
  write_cluster_chain(fs, directory->cluster_id, (uint8_t *)dirents, n_dirents * sizeof(fat32_dirent_t));
  return 1;
}

int fat32_write(fat32_fs_t *fs, pi_dirent_t *directory, char *filename, pi_file_t *file) {
  demand(init_p, "fat32 not initialized!");
  demand(directory->is_dir_p, "tried to use a file as a directory!");

  // TODO: Surprisingly, this one should be rather straightforward now.
  // - load the directory
  // - exit with an error (0) if there's no matching directory entry
  // - update the directory entry with the new size
  // - write out the file as clusters & update the FAT
  // - write out the directory entry
  // Special case: the file is empty to start with, so we need to update the
  // start cluster in the dirent

  // unimplemented();
  uint32_t n_dirents;
  fat32_dirent_t * dirents = get_dirents(fs, directory->cluster_id, &n_dirents);

  uint32_t i = find_dirent_with_name(dirents, n_dirents, filename);
  if (i == -1) {
    trace("did not find file\n");
    return 0;
  }

  uint8_t *data = file->data;
  size_t length = file->n_data;
  
  uint32_t cluster = fat32_cluster_id(&dirents[i]);
  if (dirents[i].file_nbytes == 0) {
    trace("edge case where the old length was 0\n");
    cluster = find_free_cluster(fs, 3);
    fs->fat[cluster] = LAST_CLUSTER;
    dirents[i].lo_start = cluster & 0xFFFF;
    dirents[i].hi_start = cluster >> 16;
  }

  write_cluster_chain(fs, cluster, data, length);
  dirents[i].file_nbytes = length;
  trace("writing with %d bytes at cluster %d\n", n_dirents * sizeof(fat32_dirent_t), directory->cluster_id);
  write_cluster_chain(fs, directory->cluster_id, (uint8_t *)dirents, n_dirents * sizeof(fat32_dirent_t));
  return 1;


}

int fat32_flush(fat32_fs_t *fs) {
  demand(init_p, "fat32 not initialized!");
  // no-op
  return 0;
}
