#ifndef DIRECTORY_H
#define DIRECTORY_H
#include "fs.h"
#define NUM_DIR_OPS	         4
#define MAX_FILENAME_LEN    32
#define MAX_FILES            8
extern int32_t dir_open (int32_t fd, uint8_t* fname, uint32_t length); /* open io operation */
extern int32_t dir_close(int32_t fd, uint8_t* fname, uint32_t length); /* close io operation */
extern int32_t dir_read  (int32_t fd, uint8_t* buffer, uint32_t length); /* read io operation */
extern int32_t dir_write (int32_t fd, uint8_t* buffer, uint32_t length); /* write io operation */

extern int32_t (*directory_ops[NUM_DIR_OPS])(int32_t, uint8_t*, uint32_t);
#endif
