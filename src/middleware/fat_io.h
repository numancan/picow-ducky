#pragma once

#include <stddef.h>

#include "f_util.h"
#include "stdbool.h"

#define fat_io_change_dir(dir) f_chdir(dir)
#define fat_io_mkdir(path) f_mkdir(path)

typedef enum {
    FAT_IO_OK                  = FR_OK,
    FAT_IO_DISK_ERR            = FR_DISK_ERR,
    FAT_IO_INT_ERR             = FR_INT_ERR,
    FAT_IO_NOT_READY           = FR_NOT_READY,
    FAT_IO_NO_FILE             = FR_NO_FILE,
    FAT_IO_NO_PATH             = FR_NO_PATH,
    FAT_IO_INVALID_NAME        = FR_INVALID_NAME,
    FAT_IO_DENIED              = FR_DENIED,
    FAT_IO_EXIST               = FR_EXIST,
    FAT_IO_INVALID_OBJECT      = FR_INVALID_OBJECT,
    FAT_IO_WRITE_PROTECTED     = FR_WRITE_PROTECTED,
    FAT_IO_INVALID_DRIVE       = FR_INVALID_DRIVE,
    FAT_IO_NOT_ENABLED         = FR_NOT_ENABLED,
    FAT_IO_NO_FILESYSTEM       = FR_NO_FILESYSTEM,
    FAT_IO_MKFS_ABORTED        = FR_MKFS_ABORTED,
    FAT_IO_TIMEOUT             = FR_TIMEOUT,
    FAT_IO_LOCKED              = FR_LOCKED,
    FAT_IO_NOT_ENOUGH_CORE     = FR_NOT_ENOUGH_CORE,
    FAT_IO_TOO_MANY_OPEN_FILES = FR_TOO_MANY_OPEN_FILES,
    FAT_IO_INVALID_PARAMETER   = FR_INVALID_PARAMETER,

    /* Local additions, not part of FRESULT. */
    FAT_IO_ERROR,
    FAT_IO_EOF,
    FAT_IO_BUFFER_SIZE_ERROR,
} FatIoResult;

// Mounts the SD card filesystem. Call once before any other fat_io_* function.
FatIoResult fat_io_init();
// Is the filesystem currently mounted?
bool fat_io_is_mounted();
// Unmounts the filesystem.
FatIoResult fat_io_unmount();
// Opens filename for appending, creating it if it doesn't exist.
FatIoResult fat_io_open_write(FIL* fil, const char* const filename);
// Opens filename for reading.
FatIoResult fat_io_open_read(FIL* fil, const char* const filename);
// Closes a file opened via fat_io_open_read/write.
FatIoResult fat_io_close(FIL* fil);
// Reads one line (including the newline) into buf. Returns
// FAT_IO_BUFFER_SIZE_ERROR if the line didn't fit.
FatIoResult fat_io_read_line(FIL* file, char* buf, size_t buf_size);
// Writes size bytes from str to fil.
FatIoResult fat_io_write(FIL* fil, char* str, uint32_t size);
// Counts files under path matching ext (NULL for any) and max_fname_len (0 for no limit).
uint32_t fat_io_count_files(const char* path, const char* ext, uint32_t max_fname_len);
// Reads into buffer until delim is found (exclusive) or len bytes are copied.
FatIoResult fat_io_read_until(FIL* fil, char* buffer, char delim, int16_t len);
// Deletes filename.
FatIoResult fat_io_remove_file(const char* filename);
// Renames/moves old_path to new_path.
FatIoResult fat_io_rename(const char* old_path, const char* new_path);
// Checks whether fname exists.
FatIoResult fat_io_check_file_exist(const char* fname);
// Does name end with ext (case-insensitive)?
bool fat_io_has_ext(const char* name, const char* ext);
// Opens path for iteration via fat_io_read_dir.
FatIoResult fat_io_open_dir(DIR* dir, const char* path);
// Closes a directory opened via fat_io_open_dir.
FatIoResult fat_io_close_dir(DIR* dir);
// Reads the next directory entry matching ext (NULL for any) into buf.
// Returns FAT_IO_EOF once no entries remain.
FatIoResult fat_io_read_dir(DIR* dir, const char* ext, char* buf, size_t buf_size);
// Advances the file cursor to the start of the next line (or EOF), discarding
// everything skipped.
FatIoResult fat_io_skip_to_newline(FIL* file);