#ifndef DIRECTORY_C
#define DIRECTORY_C
#include "include/directory.h"
#include "include/sched.h"
dentry_t __dentry;
/*
 * dir_open
 *   DESCRIPTION: Opens a file in task
 *   INPUTS: fd: file descriptor
 *           dname: directory name
 *           lenght: size of dentry
 *   OUTPUTS: 0 on success, -1 on failure
 *   RETURN VALUE: integer
 *   SIDE EFFECTS: Opens the file at the fd
 */
int32_t dir_open (int32_t fd, uint8_t* dname, uint32_t length)
{
    dentry_t* dentry = &__dentry;
    fs_t* file = &current_proc->open_files->files[fd]; /* ptr to current file in current file table */
    if (file->flags & _ERR) {
	return FS_ERROR;
    }
    if (read_dentry_by_name(dname, dentry) == FS_ERROR) /* populate dentry */
	return FS_ERROR;
    read_fd_inode_by_dentry(file, dentry); /* populate inode ptr in file struct according to denty */
    file->flags = _OPEN; /* set flags to OPEN state */
    file->f_pos = 0;     /* offset into file initially 0 */
    file->op_ptr = &dir_ops_table;
    current_proc->open_files->bitmap |= (1 << fd); /* set corresponding bit in file bitmap to indicate file no longer available for assignment*/
    return FS_SUCCESS;
}
/*
 * dir_close
 *   DESCRIPTION: Closes a file in task
 *   INPUTS: fd: file descriptor
 *           dname: directory name
 *           lenght: size of dentryo
 *   OUTPUTS: 0 on success, -1 on failure
 *   RETURN VALUE: integer
 *   SIDE EFFECTS: Closes the file in the task at the fd
 */

int32_t dir_close (int32_t fd, uint8_t* dname, uint32_t length)
{
    fs_t* file = &current_proc->open_files->files[fd]; /* pointer to file  in file table */
    if (file == NULL || fd < 0) /* check for invalid input or file pointer */
	return FS_ERROR;
    if (file->flags & _CLOSE) /* check status of file */
	return FS_ERROR;
    while ( (file->flags & _WRITE) || (file->flags & _READ)); /* wait until READ/WRITE operations finish */
    if (file->sess != NULL) {
	if (file->sess->op_ptr != NULL) {
	    file->sess->op_ptr = NULL; /* dissassociate terminal IO operations */
	}
	if (file->sess->buffer != NULL) {
	    file->sess->buffer = NULL; /* dissassociate terminal buffer from file */
	}
    }
    file->flags     = _CLOSE; /* set file flags to closed state */
    file->f_pos     = 0;      /* reset offset into file */
    file->buffer    = NULL;   /* dissassociate file buffer */
    file->inode     = NULL;   /* disassociate file inode   */
    file->sess      = NULL;   /* disassociate file terminal session */
    file->inode_num = 0;      /* reset inode_num associated to file */
    current_proc->open_files->bitmap &= ~(1<<fd); /* clear bit in file table bitmap to mark file as available for assignment */
    return FS_SUCCESS;
}
/*
 * dir_Read
 *   DESCRIPTION: Reads a file from the task
 *   INPUTS: fd: file descriptor
 *           dname: directory name
 *           lenght: size of dentry
 *   OUTPUTS: # of bytes read, -1 on failure
 *   RETURN VALUE: integer
 *   SIDE EFFECTS: Reads from a file in the task and returns the # of bytes read
 */

int32_t dir_read (int32_t fd, uint8_t* buffer, uint32_t length)
{
    if (buffer == NULL) /* validate input */
	return FS_ERROR;

    boot_block_t* directory_info = (boot_block_t*)(fs_img_addr);
    if (directory_info == NULL) /* verify filesystem has been initialized */
	return FS_ERROR;

    uint32_t current_inode_num;
    inode_t* current_inode;
    //uint8_t* name;
    uint32_t index;
    int32_t name_len = 0;

    if (CHECK_MSB(fd) == 1) { /* verify nonnegative FD# */
    	return FS_ERROR;
    }
    if (CHECK_MSB(length) == 1) { /* check for negative length */
    	return FS_ERROR;
    }

    /* iterate over the directory entries present */
    index = current_proc->open_files->files[fd].f_pos;

    if (index < 0 || index > directory_info->num_dirs) {
    	index = 0;
    	current_proc->open_files->files[fd].f_pos = 0;
    	return FS_SUCCESS;
    }
    __dentry = (dentry_t)(directory_info->directory[index]);
    for (; __dentry.file_name[name_len] != '\0' && __dentry.file_name[name_len] != '\n' && name_len< MAX_FILENAME_LEN; name_len++);
    current_inode_num = __dentry.inode_num;
    current_inode = (inode_t*)((uint8_t*)fs_img_addr + ((current_inode_num+1) * BLOCK_SIZE));
    //ustrcpy(buffer, __dentry.file_name, name_len);
    memcpy((void*)buffer, (const void*)__dentry.file_name, name_len);
    current_proc->open_files->files[fd].f_pos += 1; // cont number of files read
    return name_len;
}
/*
 * dir_write
 *   DESCRIPTION: Writes to a directory
 *   INPUTS: fd: file descriptor
 *           dname: directory name
 *           lenght: size of dentry
 *   OUTPUTS: # of bytes read, -1 on failure
 *   RETURN VALUE: integer
 *   SIDE EFFECTS: impossible to do so return -1
 */
int32_t dir_write(int32_t fd, uint8_t* buffer, uint32_t length)
{
    return FS_ERROR;
}

int32_t (*directory_ops[NUM_DIR_OPS])(int32_t fd, uint8_t* buffer, uint32_t length) = {
    dir_open,
    dir_close,
    dir_read,
    dir_write
};
io_table_t dir_ops_table = { &dir_open, &dir_close, &dir_read, &dir_write };
#endif
