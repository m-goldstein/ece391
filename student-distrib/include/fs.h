/*
 * References:
 *  Understanding The Linux Kernel ~ 3rd Edition
 *	The Ext2 and Ext3 Filesystems.
 */

#ifndef FS_H
#define FS_H

#include "lib.h"
#include "terminal.h"
#include "sys_call.h"
#define BLOCK_SIZE	      4096
#define FILE_BUFFER_SIZE      (2)*(BLOCK_SIZE)
#define NUM_BLOCKS	      128
#define MAX_BLOCKS_PER_INODE  1023
#define MAX_NUM_FD	      8
#define MAX_NUM_FILE_TABLES   10
#define FS_ERROR	      -1
#define FS_SUCCESS	      0
#define MAX_DIRECTORIES       64
#define MAX_DIRECTORY_ENTRIES 63
#define MAX_FILENAME_LENGTH   32
#define BLOCK_RESERVED_BYTES  52
#define DIR_RESERVED_BYTES    24
#define NUM_IO_OPS            4
/* Array of function pointers for file I/O operations. */
extern int32_t (*io[NUM_IO_OPS])(int32_t, uint8_t*, uint32_t);

/* Enumeration structure to convey the state of the file */
enum flags {
    _CLOSE     = 0x000,   /* file is closed		      */
    _OPEN      = 0x001,   /* file is open for I/O             */
    _READ      = 0x002,   /* file is being read               */
    _WRITE     = 0x004,   /* file is being written            */
    _EOF       = 0x008,   /* the end of file has been reached */
    _ERR       = 0x010    /* file is in an error state        */
};

/* Enumeration structure to assign integer values to I/O operations */
enum file_io_operations {
     OPEN      = 0x0,      /* open operation is entry zero      */
     CLOSE     = 0x1,      /* close is entry one                */
     READ      = 0x2,      /* read  is entry two                */
     WRITE     = 0x3       /* read is entry three               */
};

/* data structure representing an filesystem inode */
typedef struct inode {
    uint32_t length;		    /* length of associated file in bytes */
    /* pointers to data blocks associated with the inode */
    uint32_t data_blocks[MAX_BLOCKS_PER_INODE];
} inode_t;

/* data structure representing a block in the filesystem */
typedef struct bg_descriptor {
    uint32_t num_dirs;	     /* number of directories */
    uint32_t num_inodes;     /* number of inodes      */
    uint8_t padding[BLOCK_RESERVED_BYTES];     /* 52B padding           */
    uint8_t directories[MAX_DIRECTORIES]; /* allocated space for the
				             maximum number of directories */
} block_t;


/* data structure representing a directory entry */
typedef struct directory {
    uint8_t  file_name[MAX_FILENAME_LENGTH]; /* Max chracters for a file name */
    uint32_t file_type;
    uint32_t inode_num;                      /* associated inode number       */
    uint8_t  reserved[DIR_RESERVED_BYTES];   /* 24B reserved                  */
} dentry_t;

/* data structure representing a file in the file system and
 * its resources */
typedef struct fs_io
{
    /* array of function points for filesystem I/O operations */
    io_table_t *op_ptr;
    uint8_t* buffer;	       /* the buffer associated with the file           */
    terminal_session_t* sess;  /* the terminal session associated with the file */
    inode_t* inode;            /* the inode associated with the file            */
    uint32_t inode_num;        /* the dentry associated with the file   */
    uint32_t f_pos;            /* the current file position after the beginning */
    uint32_t flags;            /* the state of the current file           */
} fs_t;
/* data structure for the boot block */
typedef struct boot_block {
    uint32_t num_dirs;        /* number directories present in the filesystem */
    uint32_t num_inodes;      /* number inodes present in the filesystem      */
    uint32_t num_data_blocks; /* number data blocks present in the filesystem */
    uint8_t padding[BLOCK_RESERVED_BYTES];      /* 52B padding                                  */
		              /* filesystem supports a maximum of 63 entries  */
    dentry_t directory[MAX_DIRECTORY_ENTRIES];
} boot_block_t;

/* boot block for the OS                                 */
block_t boot_block;

/* array of files statically sized                       */
struct file_table {
    fs_t files[MAX_NUM_FD];
    uint8_t bitmap;
};
typedef struct file_table file_table_t;

/* the starting address of the filesystem image          */
unsigned int fs_img_addr;
/* initialize the filesystem                             */
extern void init_fs();

/* seek to a position in the file                         */
extern void seek(fs_t* fs, int32_t src, int32_t dst);

 /* populate a file descriptor based on a directory entry */
extern int32_t read_fd_inode_by_dentry(fs_t* file, dentry_t* dentry);

/* populate a directory entry based on a file name        */
extern int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry);

/* populate a direcotry entry based on an inode number    */
extern int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry);

/* read data from an inode to a buffer                    */
extern int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

/* write data from a buffer to an inode                    */
extern int32_t write_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

/* set the current position in the file to offset          */
extern int32_t seek_to_position (fs_t* file, int32_t offset);

/* modify the current position in the file by offset       */
extern int32_t seek_from_position(fs_t* file, int32_t offset);

/* initialize the file descriptors array                   */
extern void init_file_table(file_table_t* file_table);

/* set the starting address of the filesystem image        */
extern void set_fs_start_addr(unsigned int addr);

/* array of buffers used by each file                      */
//extern uint8_t file_buffers[MAX_NUM_FD][FILE_BUFFER_SIZE];
extern file_table_t file_table[MAX_NUM_FILE_TABLES];
extern uint32_t file_table_bitmap;
extern file_table_t* root_file_table;
extern file_table_t* curr_file_table;
/* file I/O operation function declarations */
extern int32_t fs_open  (int32_t fd, uint8_t* fname, uint32_t length);
extern int32_t fs_close (int32_t fd, uint8_t* fname, uint32_t length);
extern int32_t fs_read  (int32_t fd, uint8_t* buffer, uint32_t length);
extern int32_t fs_write (int32_t fd, uint8_t* buffer, uint32_t length);
extern int32_t invalid_func(int32_t fd, uint8_t* buffer, uint32_t length);
extern file_table_t* get_curr_file_table();
extern void set_curr_file_table(uint32_t proc_num);
extern int16_t next_free_file_table();
#endif
