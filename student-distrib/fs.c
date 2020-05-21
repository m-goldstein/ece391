#ifndef FS_C
#define FS_C

#include "include/fs.h"
#include "include/task.h"
#include "include/sys_call.h"
#include "include/sched.h"
#define BLOCKS_PER_GROUP  16
#define stdin		  0
#define stdout		  1
#define NUM_INODES_OFFSET 0x4
#define NUM_DIRS_OFFSET   0x8
#define MAX_FILENAME_LEN  32
//uint8_t file_buffers[MAX_NUM_FD][FILE_BUFFER_SIZE];
file_table_t file_table[MAX_NUM_FILE_TABLES];
uint32_t file_table_bitmap;
file_table_t* root_file_table;
file_table_t* curr_file_table;
dentry_t dummy_dentry;
/*
 * init_fs
 *
 *  DESCRIPTION : sets num_inodes and num_dirs field for boot block and
 *		fields in file descriptors. Also, opens stdin and stdout as 0th
 *		and 1st file descriptors.
 *  INPUTS      : None
 *  OUTPUTS     : None
 *  RETURNS     : None
 *  SIDE-EFFECTS: opens stdout and stdin and calls helper function
 *		  to initialize file descriptors
 */
void init_fs()
{
    /* initialize boot block fields dynamicall */
    boot_block.num_inodes        =  (uint32_t) *((uint8_t*)fs_img_addr + NUM_INODES_OFFSET);
    boot_block.num_dirs          =  (uint32_t) *((uint8_t*)fs_img_addr + NUM_DIRS_OFFSET);
    root_file_table = &file_table[0];
    curr_file_table = &file_table[0];
    set_curr_file_table(0);
    file_table_bitmap = 1;
    /* initialize the array of files           */
    init_file_table(root_file_table);

    /* open stdin and stdout for I/O           */
    root_file_table->files[stdin].op_ptr->open(stdin, NULL, 0);
    root_file_table->files[stdin].op_ptr->open(stdout, NULL, 0);

}


/*
 * read_dentry_by_name
 *
 *  DESCRIPTION : populates fields of a dentry_t based on a filename
 *  INPUTS      : fname -- pointer representation of filename to be read
 *		 as a directory entry.
 *		 dentry -- pointer to dentry_t struct to be initalized with
 *		 the fields that correspond to the filename.
 *  OUTPUTS     : none
 *  RETURNS     : returns 0 if dentry was successfully populated.
 *		 returns -1 if dentry was not populated or filename not found.
 *  SIDE EFFECTS: populates fields of dentry object parameter passed
 *                as a pointer
 */
uint8_t fname_buf[MAX_FILENAME_LEN];
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry)
{
    boot_block_t* directory_info = (boot_block_t*)(fs_img_addr);
    uint32_t current_inode_num;
    inode_t* current_inode;
    uint8_t* name;
    uint32_t index;
    int32_t cmp_flag;
    int32_t fname_len = 0;
    int32_t name_len;
    strcpy((int8_t*)fname_buf, (const int8_t*)fname);
    /* calculate the filename length before the comparison step */
    for (; fname_buf[fname_len] != '\0' && fname_buf[fname_len] != '\n' && fname_len < MAX_FILENAME_LEN+1; fname_len++);
    if(fname_len > MAX_FILENAME_LEN)
      return -1;

    /* iterate over the directory entries present */
    for (index = 0; index < directory_info->num_dirs; index++) {
	dummy_dentry   = (dentry_t)(directory_info->directory[index]);
	current_inode_num = dummy_dentry.inode_num;     /* determine current inode number */

	/* determine the current inode */
	current_inode = (inode_t*)((uint8_t*)fs_img_addr + ((current_inode_num+1) * BLOCK_SIZE));

	name = dummy_dentry.file_name; /* set the filename of the current entry */

	//Compare string based on longer string
	name_len = strlen((const int8_t*)name);
	if(name_len > fname_len)
	    fname_len = name_len;

	/* string compre to see if the parameter filename is same as current filename */
	if (fname_len >= MAX_FILENAME_LEN) {
	    fname_len = MAX_FILENAME_LEN - 1;
	}
	cmp_flag = strncmp((const int8_t*)fname_buf,(const int8_t*)name, fname_len);
	if (cmp_flag == 0) {
	    *dentry = dummy_dentry; /* set the data of the dentry pointer parameter to
				       the current directory entry */
	    return FS_SUCCESS; /* directory entry found */
	}
	continue;
    }
    return FS_ERROR; /* directory entry not found */
}

/*
 * read_dentry_by_index
 *
 *  DESCRIPTION : populates fields of a dentry_t based on a inode number
 *  INPUTS      : index -- inode number of file to be read
 *		  as a directory entry.
 *		  dentry -- pointer to dentry_t struct to be initalized with
 *		  the fields that correspond to the inode number (index).
 *  OUTPUTS     : none
 *  RETURNS     : returns 0 if dentry was successfully populated.
 *		  returns -1 if dentry was not populated or inode not found.
 *  SIDE EFFECTS: populates fields of dentry object parameter passed
 *		  as a pointer
 */
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry)
{

    boot_block_t* directory_info = (boot_block_t*)(fs_img_addr); /* fill the boot block structue */
    //dentry_t current_dentry;
    //uint32_t current_inode_num;
    if (directory_info == NULL)
	return FS_ERROR;

    /* set the current inode number to the inode_num field of the corresponding directory entry */
    dentry    = (dentry_t*)(&directory_info->directory[index]);
    //current_inode_num = current_dentry.inode_num;

    /* set the data of the dentry pointer parameter to the data of the corresponding
     * directory entry */
    //dentry = &current_dentry;
    return FS_SUCCESS;
}

/*
 * seek_to_position
 *  DESCRIPTION  : set the current positon within the file to an offset
 *  INPUTS       : file   -- pointer to file we want to access
 * 		   offset -- offset to set the current positon in file to
 *  OUTPUTS      : an error message if one occurs
 *  RETURNS      : returns -1 if an error occured
 *                 returns new position in file on success
 *  SIDE EFFECTS : sets position in file to offset given as parameter
 */
int32_t seek_to_position (fs_t* file, int32_t offset)
{
    if (file == NULL) {
	return FS_ERROR;
    }
    if (file->inode == NULL) {
	return FS_ERROR;
    }
    if (offset > file->inode->length) {
	return FS_ERROR;
    }

    /* check if file is in a seekable state */
    if (file->flags == _ERR) {
	return FS_ERROR;
    }
    file->f_pos = offset; /* set current position in file to offset */



    if (file->f_pos >= file->inode->length) {
	file->flags |= _EOF;      /* turn on EOF state since position exceeds length    */
    } else {
	file->flags &= ~_EOF;  /* turn off EOF state since position is within bounds */
    }

    return file->f_pos;
}

/*
 * seek_from_position
 *   DESCRIPTION  : modify the current positon within the file by an offset
 *   INPUTS       : file   -- pointer to file we want to access
 *		    offset -- offset to increment/decrement the
 *		              current position in file by
 *   OUTPUTS      : an error message if one occurs
 *   RETURNS      : returns -1 if an error occured
 *                  returns new position in file on success
 *   SIDE EFFECTS : modifies current position in file by offset given
 *		    as parameter
 */
int32_t seek_from_position (fs_t* file, int32_t offset)
{
    /* check for null entries to prevent faults */
    if (file == NULL) {
	return FS_ERROR;
    }
    if (file->inode == NULL) {
	return FS_ERROR;
    }

    /* check for bounds to prevent seeking past end of file */
    if (file->f_pos + offset > file->inode->length) {
	return FS_ERROR;
    }

    /* check if file is in seekable state */
    if (file->flags == _ERR) {
	return FS_ERROR;
    }
    if (file->flags & _EOF) {
	return FS_ERROR;
    }

    file->f_pos += offset; /* increment current file position by offset */

    if (file->f_pos >= file->inode->length) {
	file->flags |= _EOF;      /* turn on EOF state since position exceeds length    */
    } else {
	file->flags &= ~_EOF; /* turn off EOF flag */
    }
    return file->f_pos;
}

/*
 * read_data
 *
 *   DESCRIPTION : copies a length sized chunk of data from file
 *		starting at position offset.
 *   INPUTS      : inode  -- inode number of file whose data is to be read
 *		   offset -- position after start of file to begin reading data
 *		   buf    -- buffer where file data is copied to
 *		   length -- size in bytes for the amount of data to
 *			  be read into the buffer.
 *   OUTPUTS     : none
 *   RETURNS     : returns the number of bytes read or -1 if
 *		   an error occured.
 *   SIDE EFFECTS: copies length size amount of data from data blocks
 *		 of an inode to the buffer passed as parameter.
 */
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length)
{
    inode_t* inode_ptr;
    uint32_t file_length;

    /* fill in inode_ptr structure with corresponding inode */
    inode_ptr = (inode_t*)((uint8_t*)fs_img_addr + ((inode + 1) * BLOCK_SIZE));
    file_length = inode_ptr->length;

    /* checks for null to prevent exceptions */
    if (buf == NULL)
	return FS_ERROR;
    if (inode_ptr == NULL)
	return FS_ERROR;

    /* if the memcpy will copy data that is out of bounds,
     * adjust the size of data to be transferred*/
    if (offset+length > file_length)
	length = file_length - offset;

    /* variables for book-keeping */
    uint8_t  block_idx;
    uint8_t* current_data_block;
    uint32_t temp_length = 0;
    uint32_t bytes_read = 0;

    /* if the data transfer involves more than one block */
    while (length > BLOCK_SIZE) {
	block_idx = (offset) / BLOCK_SIZE;              /* current data block index */

	/* adjust offset and size of transfer if position in current block plus length
	 * exceeds block size                                                       */
	if( (offset % BLOCK_SIZE + length) > BLOCK_SIZE)
	{
	    temp_length = BLOCK_SIZE - (offset % BLOCK_SIZE);
	    offset += temp_length;
	}
	/* point current data block to proper location                              */
	current_data_block = (uint8_t* )((1 + boot_block.num_inodes + (uint32_t)(inode_ptr->data_blocks[block_idx]))* BLOCK_SIZE + fs_img_addr);

	/* perform the memory transfer                                              */
	memcpy((uint8_t*)(buf + bytes_read), current_data_block, temp_length);
	length -= temp_length;     /* adjust size of data to read from data blocks */
	bytes_read += temp_length; /* increment number of bytes read               */
    }

    /* performs data transfer when size of transfer is less than block size        */
    block_idx = (offset) / BLOCK_SIZE;

    /* point curernt data block to proper location */
    current_data_block = (uint8_t* )((1 + boot_block.num_inodes + (uint32_t)(inode_ptr->data_blocks[block_idx])) * BLOCK_SIZE + fs_img_addr);

    memcpy((uint8_t*)(buf + bytes_read), current_data_block+ (offset % BLOCK_SIZE), length);
    bytes_read += length; /* adjust bytes read                                      */
    return bytes_read;    /* return bytes read to caller function                   */
}

/*
 * write_data
 *
 *  DESCRIPTION : copies a length sized chunk of data to the file
 *		 starting at position offset.
 *  INPUTS      : inode -- inode number of file whose data is to be
 *                          written to
 *		  offset -- position after start of file to begin
 *		            writing data
 *		  buf    -- buffer where file data is copied from
 *		  length -- size in bytes for the amount of data
 *		            to be written into the file.
 *  OUTPUTS     : none
 *  RETURNS     : returns the number of bytes written or -1 if
 *                an error occured.
 *  SIDE EFFECTS: copies length size amount of data from a buffer pased as
 *		  parameter to the data blocks of an inode
 */
int32_t write_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length)
{
    inode_t* inode_ptr;
    uint32_t file_length;
    /* fill in inode_ptr structure with corresponding inode */
    inode_ptr = (inode_t*)((uint8_t*)fs_img_addr + ((inode + 1) * BLOCK_SIZE));
    file_length = inode_ptr->length;

    /* checks for null to prevent exceptions */
    if (buf == NULL)
	return FS_ERROR;
    if (inode_ptr == NULL)
	return FS_ERROR;

    /* if the memcpy will copy data that is out of bounds,
     * adjust the size of data to be transferred        */
    if (offset+length > file_length)
	length = file_length - offset;

    /* variables for book-keeping */
    uint8_t  block_idx;
    uint8_t* current_data_block;
    uint32_t temp_length = 0;
    uint32_t bytes_written = 0;

    /* if the data transfer involves more than one block */
    while (length > BLOCK_SIZE) {
	block_idx = (offset) / BLOCK_SIZE;              /* current data block index */

	/* adjust offset and size of transfer if position in current block plus length
	 * exceeds block size                                                       */
	if( (offset % BLOCK_SIZE + length) > BLOCK_SIZE)
	{
	    temp_length = BLOCK_SIZE - (offset % BLOCK_SIZE);
	    offset += temp_length;
	}

	/* point current data block to correct location                             */
	current_data_block = (uint8_t* )((1 + boot_block.num_inodes + (uint32_t)(inode_ptr->data_blocks[block_idx]))* BLOCK_SIZE + fs_img_addr);

	/* perform the data transfer                                                */
	memcpy((uint8_t*)(current_data_block + bytes_written), buf, temp_length);

	/* adjust length of data to transferred and number of bytes written */
	length -= temp_length;
	bytes_written += temp_length;
    }

    /* if the transfer can be done using a single block                             */
    block_idx = (offset) / BLOCK_SIZE;

    /* point current data block to correct location                                 */
    // the plus one is for the boot block in the beginning
    current_data_block = (uint8_t* )((1 + boot_block.num_inodes + (uint32_t)(inode_ptr->data_blocks[block_idx])) * BLOCK_SIZE + fs_img_addr);

    /* perfrom the data transfer                                                    */
    memcpy((uint8_t*)(current_data_block + bytes_written), buf+(offset%BLOCK_SIZE), length);
    bytes_written += length; /* update number of bytes written                      */
    return bytes_written;    /* return number of bytes written to caller function   */
}

/*
 * fs_open
 *
 *  DESCRIPTION : opens a file for I/O operations.
 *  INPUTS      : fd     -- the file descriptor of the file to open
 *		 fname  -- pointer containing the name of the file to open
 *		 length -- ignored for now
 *  OUTPUTS     : displays an error message if one occurs
 *  RETURNS     : returns -1 if an error occurred or 0 if success.
 *  SIDE EFFECTS: sets the position in the current file to 0 and
 *		 sets flags to the open state
 */
int32_t fs_open(int32_t fd, uint8_t* fname, uint32_t length)
{
    //dentry_t _dentry;
    dentry_t* dentry = &dummy_dentry;
    fs_t* file = &curr_file_table->files[fd];            /* point to corresponding entry in array of files */

    /* dont allow access to a file if it is in an error state */
    if (file->flags & _ERR) {
	return FS_ERROR;
    }

    /* populate the directory entry structure with the data corresponding to the filename */
    //uint32_t len = 0;
    //uint8_t name[32];
    //for (; fname[len] != '\0'; len++)
    //memcpy(name, fname, len);
    if (read_dentry_by_name(fname, dentry) == FS_ERROR)
	return FS_ERROR;
    /* initialize the file and associated inode using the directory entry                 */

    read_fd_inode_by_dentry(file, dentry);
    file->flags  = _OPEN;		/* set the state of the file to open              */
    file->f_pos  = 0;			/* set the current position in the file to zero   */
    /*file->buffer = file_buffers[fd];     associate a buffer to the file                 */

    /* initialize file I/O functions that have been implemented                           */
    file->op_ptr = &f_ops_table;
    curr_file_table->bitmap |= (1 << fd);
    return FS_SUCCESS;
}

/*
 * fs_read
 *
 *  DESCRIPTION : reads block of data of size length from a file to
 *		  the buffer passed as a parameter
 *  INPUTS      : fd     -- the file descriptor of the file to read
 *		  buffer -- pointer containing the buffer to write the file data
 *		  length -- amount of data to read from file in bytes
 *  OUTPUTS     : an error message if one occurs
 *  RETURNS     : returns -1 if an error occurred or 0 if success.
 *  SIDE EFFECTS: sets the position in the current file to number of
 *                bytes read
 */
int32_t fs_read(int32_t fd, uint8_t* buffer, uint32_t length)
{
    /* checks for NULL to prevent faults */
    if (buffer == NULL)
	return FS_ERROR;
    if (&curr_file_table->files[fd] == NULL)
	return FS_ERROR;
    if (CHECK_MSB(length)) /* check for negative length */
	return FS_ERROR;

    dentry_t* dentry = &dummy_dentry;
    fs_t* file = &curr_file_table->files[fd];

    /* checks state of file before reading to prevent faults */
    if (file->flags & _CLOSE) {
	return FS_ERROR;
    }
    if (file->flags & _ERR) {
	return FS_ERROR;
    }

    if (file->flags & _EOF) {
	return FS_SUCCESS;
    }
    /* wait until any write operations complete before reading data */
    while (file->flags & _WRITE) ;

    /* populate directory entry using inode number associated with the file */
    if (read_dentry_by_index(file->inode_num, dentry) == FS_ERROR) {
	return FS_ERROR;
    }

    /* check that the associated inode is not null */
    if (file->inode == NULL)
	read_fd_inode_by_dentry(file, dentry);

    file->flags |= _READ;    /* set the file state to indicate it is being read             */
    int32_t pos;
    pos = read_data(file->inode_num, file->f_pos, buffer, length);
    file->f_pos += pos;       /* update current position in file to number of bytes read     */

    if (file->f_pos >= file->inode->length)
	file->flags |= _EOF; /* set the file state to indicate end of file has been reached */
    file->flags &= ~_READ;    /* clear the read flag from the current file state             */
    return pos;
}

/*
 * fs_write
 *
 *  DESCRIPTION : writes block of data of size length to a file from
 *		  the buffer passed as a parameter
 *  INPUTS      : fd     -- the file descriptor of the file to read
 *		  buffer -- pointer containing source buffer to write
 *		  the file data
 *		  length -- amount of data to write to file in bytes
 *  OUTPUTS     : an error message if one occurs
 *  RETURNS     : returns -1 if an error occurred or 0 if success.
 *  SIDE EFFECTS: sets the position in the current file to number
 *                of bytes written
 */
int32_t fs_write(int32_t fd, uint8_t* buffer, uint32_t length)
{
    // We are a read only file system but have capbilities of write also if needed
    /* check for null parameters to prevent faults */
    if (buffer == NULL)
	return FS_ERROR;
    if (&curr_file_table->files[fd] == NULL)
	return FS_ERROR;

    dentry_t* dentry = &dummy_dentry;
    fs_t* file = &curr_file_table->files[fd];
    /* check the state of file before trying to access or modify its data */

    if (file->flags & _CLOSE) {
	return FS_ERROR;
    }

    if (file->flags & _ERR) {
	return FS_ERROR;
    }

    /* wait until any other write operations are finished. */
    while (file->flags & _WRITE) ;

    /* populate a directory entry structure with the data corresponding to the file */
    if (read_dentry_by_index(file->inode_num, dentry) == FS_ERROR)
	return FS_ERROR;

    /* verify the contents of the inode associated with the file */
    if (file->inode == NULL)
	read_fd_inode_by_dentry(file, dentry);

    file->flags |= _WRITE;  /* modify the flags to convey a write operation is being performed */
    int32_t pos = write_data(file->inode_num, file->f_pos, buffer, length);
    file->f_pos = pos;      /* update position in file to number of bytes written              */

    if (file->f_pos >= file->inode->length) {
	file->flags |= _EOF; /* if position in file exceeds the size of the file,
				modify the flags to convey end of file was reached */
    }

    file->flags &= ~_WRITE;   /* clear write status from the current file state     */
    return FS_SUCCESS;
}

/*
 * fs_close
 *
 *  DESCRIPTION : closes a file and releases its associated resources.
 *  INPUTS      : fd     -- the file descriptor of the file to close
 *		  fname  -- pointer containing the name of the file to close
 *		  length -- ignored for now
 *  OUTPUTS     : an error message if one occurs
 *  RETURNS     : returns -1 if an error occurred or 0 if success.
 *  SIDE EFFECTS: sets the associated terminal and terminal i/o functions
 *		  to null, clears the flags of the file,
 *		  resets the current position, points the associated buffer
 *		  to null, points the associated inode to null, and resets
 *		  the inode number associated to the file.
 */
int32_t fs_close(int32_t fd, uint8_t* fname, uint32_t length)
{
    fs_t* file = &curr_file_table->files[fd];

    /* check that the file descriptor and associated file entry is valid */
    if (file == NULL || fd == -1)
	return FS_ERROR;
    /* check the file status and abort if the file is already closed. */
    if (file->flags & _CLOSE) {
	return FS_ERROR;
    }
    /* block until any write or read operations on the file are finished */
    while ( (file->flags & _WRITE) || (file->flags & _READ)) ;

    /* clear the associated terminal session resources and I/O functions if present.   */
    if (file->sess != NULL) {
	if (file->sess->op_ptr != NULL) {
	    file->sess->op_ptr = NULL;
	}

	if (file->sess->buffer != NULL) {
	    file->sess->buffer = NULL;
	}
    }
    file->flags      = _CLOSE; /* change file state to closed                  */
    file->f_pos      = 0;      /* change position in file to zero              */
    file->buffer     = NULL;   /* clear the pointer to the associated buffer   */
    file->inode      = NULL;   /* clear the pointer to the associated inode    */
    file->sess       = NULL;   /* clear the pointer to the associated terminal */
    file->inode_num  = 0;   /* change the associated inode# to 0       */
    /* clear I/O operation function pointers from the file                     */
    file->op_ptr     = NULL;
    curr_file_table->bitmap &= ~(1<<fd);
    return FS_SUCCESS;
}
/*
 * set_fs_start_addr
 *
 *  DESCRIPTION : sets the starting address of the filesystem dynamically
 *     		  when the kernel is initialized.
 *  INPUTS      : addr -- the starting address of the filesystem image.
 *  OUTPUTS     : none
 *  RETURNS     : none
 *  SIDE EFFECTS: sets the address of the filesystem image
 */
void set_fs_start_addr(unsigned int addr)
{
    /* set the starting address of the filesystem image to the address passed by input. */
    fs_img_addr = addr;
}

/* Function pointer array of I/O operations.
 *
 * This allows us to add/remove
 * I/O operations as they are implemented.
 *
 */
int32_t (*io[NUM_IO_OPS])(int32_t fd, uint8_t*buffer, uint32_t length) = {
    fs_open,
    fs_close,
    fs_read,
    fs_write
};
io_table_t f_ops_table = {&fs_open, &fs_close, &fs_read, &fs_write };
/*
 * read_fd_inode_by_dentry
 *
 *  DESCRIPTION : populates fields of a file and associated inode based
 *	 	  on a directory entry.
 *  INPUTS      : file -- fs_t struct representing file to be initalized
 *		  with information contained in the dentry.
 *		  dentry -- pointer to dentry_t struct used to set up
 *		  the file descriptor and its associated inode.
 *  OUTPUTS     : none
 *  RETURNS     : returns  1 if file was successfully populated.
 *		  returns -1 if file was not populated or dentry not found.
 *  SIDE EFFECTS: populates fields of the fs_t object and its associated inode
 */
int32_t read_fd_inode_by_dentry(fs_t* file, dentry_t* dentry)
{
    /* check for null values to prevent faults */
    if (dentry == NULL || file == NULL)
	return FS_ERROR;

    uint8_t* fs_addr = (uint8_t*)fs_img_addr;
    /* associate the file to an inode number by its index value */

    /* associate the file to an inode structure by its location in the filesystem */
    file->inode = (inode_t*)(fs_addr + (1 + dentry->inode_num) * BLOCK_SIZE);
    file->inode_num = dentry->inode_num;
    return 0;
}
io_table_t stdin_ops_table = { &terminal_open, &terminal_close, &terminal_read, &invalid_func };
io_table_t stdout_ops_table = { &terminal_open, &terminal_close, &invalid_func, &terminal_write };
/*
 * init_file_descriptors
 *
 *  DESCRIPTION : initializes the fields and resources associated
 *		  with each file descriptor currently supported by the OS.
 *  INPUTS      : fd_ptr -- pointer to array of fs_t structs representing
 *		  files to be initalized.
 *  OUTPUTS     : none
 *  RETURNS     : none
 *  SIDE EFFECTS: configures file descriptors zero and one to stdin,
 *		  and stdout respectively. Initializes entries of
 *		  fd_ptr array and associates each entry to a buffer
 *		  and set of I/O operations.
 */
void init_file_table(file_table_t* file_table)
{
    int index;
    dentry_t* dentry = &dummy_dentry;
    fs_t* files = file_table->files;
    file_table->bitmap = 0;
    /* iterate over each file entry that can be supported by the OS. */
    for (index = 0; index < MAX_NUM_FD; index++) {
	fs_t* fd = &files[index];	/* point to the corresponding entry in the files array */

	/* stdin and stdout occupy the first two entries of the files array */
	if (index == stdin) {
	    fd->sess = getCurrentSession();             /* point to an associated terminal */

	    /* use the existing terminal I/O operations for I/O on this file */
	    fd->sess->op_ptr = &stdin_ops_table;
	    fd->op_ptr = fd->sess->op_ptr;
	    read_dentry_by_index(index, dentry); /* populate a directory entry based on index */

	    /* populate the associated inode and inode num based on the directory entry */
	    read_fd_inode_by_dentry(fd, dentry);
	    fd->buffer = fd->sess->buffer;      /* associate the file with terminal session buffer */
	    fd->flags  = _OPEN;                 /* set the state to opened                         */
	    file_table->bitmap |= (1 << index);
	}
	else if (index == stdout) {
	    fd->sess = getCurrentSession(); /* point to an associated terminal */

	    /* use existing terminal I/O operations for file I/O */

	    fd->sess->op_ptr = &stdout_ops_table;
	    fd->op_ptr = fd->sess->op_ptr;
	    fd->buffer = fd->sess->buffer;           /* asscociate a buffer to the file               */
	    read_dentry_by_index(index, dentry);     /* populate a directory entry based on index     */
	    /* populate the inode and inode number fields associated to the file
	     * based on the direcotry entry */
	    read_fd_inode_by_dentry(fd, dentry);
	    fd->flags = _OPEN; /* set the state to opened  */
	    file_table->bitmap |= (1 << index);
	}
	else {
	    /* use the filesystem I/O operations for all other file descriptors */
	    fd->op_ptr = &f_ops_table;
	    //fd->buffer = file_buffers[index]; associate the file to a buffer
	    fd->inode              = NULL;    /* dissociate the file from any inode        */
	    fd->inode_num          = 0;       /* dissociate the file from any inode number */
	    fd->flags              = _CLOSE;  /* file is initially closed                  */
	    fd->f_pos              = 0;       /* position in file is initally zero         */
	}
    }

    file_table_bitmap |= (1 << current_proc->file_table_num);
    return;
}

/*
 * invalid_func
 *
 *  DESCRIPTION : function used for unaccessable function
 *  INPUTS      : fd     -- the file descriptor of the file to read
 *		  buffer -- pointer containing the buffer to write the file data
 *		  length -- amount of data to read from file in bytes
 *  OUTPUTS     :
 *  RETURNS     : returns -1
 *  SIDE EFFECTS:
 */
int32_t invalid_func(int32_t fd, uint8_t* buffer, uint32_t length)
{
    return -1;
}

/* get_curr_file_table
 *  DESCRIPTION : return pointer to current file table
 *  INPUTS      : none
 *  OUTPUTS     : none
 *  RETURN VALUE: returns NULL if error or pointer to file table
 *  SIDE EFFECTS: none
 */
file_table_t* get_curr_file_table()
{
    if (current_proc == NULL) {
	return NULL;
    }
    curr_file_table = &file_table[current_proc->file_table_num];
    if (curr_file_table == NULL)
	return NULL;
    return (file_table_t*)curr_file_table;
}


/* set_curr_file_table
 *  DESCRIPTION : set pointer to current file table based on number
 *  INPUTS      : file_table_num - index into file table array
 *  OUTPUTS     : none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: adjusts current file table pointer to point to array entry
 *                whose index corresponds to file table number passed as input.
 */
void set_curr_file_table(uint32_t file_table_num)
{
    if (&file_table[file_table_num] == NULL)
	return;
    if(file_table_num == 0)
	curr_file_table = &file_table[file_table_num];
    curr_file_table = (file_table_t*)&file_table[file_table_num];
}

/* next_free_file_table
 *  DESCRIPTION : returns index of next available index in file table bitmap
 *  INPUTS      : none
 *  OUTPUTS     : none
 *  RETURN VALUE: returns position of next available file table entry
 *  SIDE EFFECTS: adjusts current file table pointer to point to next
 *                available file table
 */
int16_t next_free_file_table()
{
    uint32_t pos = bitscan_reverse(file_table_bitmap);
    if (pos == -1)
	return -1;
    curr_file_table = &file_table[pos+1];
    return pos+1;
}
#endif
