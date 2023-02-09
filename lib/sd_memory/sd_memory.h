#ifndef _SD_MEMORY_H_
#define _SD_MEMORY_H_

// TODO: TATSIZ
#include "hw_config.h"

#define sdm_gets(buf) f_gets(buf, sizeof(buf), sdm_get_cur_fil())
#define sdm_change_dir(dir) f_chdir(dir);

FIL*    sdm_get_cur_fil();
FRESULT sdm_mount();
FRESULT sdm_unmount();
FRESULT sdm_open_read(char *fname);
FRESULT sdm_open_write(char* fname);
FRESULT sdm_close_file();
FRESULT sdm_read_dir(TCHAR *path, uint8_t *fnames, size_t n);
FRESULT sdm_read_until(char *buffer, char delim, int16_t len);
FRESULT sdm_remove_payload_file(TCHAR *filename);
FRESULT sdm_write(char *str, UINT size);

/**
 * Check file extension.
 * 
 * @param fname File name with extension to be check. (Ex. payloadname.txt)
 * @param ext   Extension for check with. (Ex. txt, exe, opt vs.)
 * @return      TODO:
 */
bool    sdm_check_file_ext(TCHAR *fname, const TCHAR *ext);

#endif