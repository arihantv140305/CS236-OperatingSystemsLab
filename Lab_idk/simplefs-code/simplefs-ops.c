#include "simplefs-ops.h"

extern struct filehandle_t file_handle_array[MAX_OPEN_FILES]; // Array for storing opened files

int simplefs_create(char *filename)
{
	/*
		Create file with name `filename` from disk
	*/
	struct inode_t *inode = (struct inode_t *)malloc(sizeof(struct inode_t));
	int flag = 0;
	for (int i = 0; i < NUM_INODES; i++)
	{
		simplefs_readInode(i, inode);
		if (strcmp(inode->name, filename) == 0)
		{
			flag = 1;
			break;
		}
	}
	if (flag == 1)
		return -1;
	int i = simplefs_allocInode();
	if (i == -1)
		return -1;
	simplefs_readInode(i, inode);
	strcpy(inode->name, filename);
	inode->status = INODE_IN_USE;
	inode->file_size = 0;
	simplefs_writeInode(i, inode);
	return i;
}

void simplefs_delete(char *filename)
{
	/*
		delete file with name `filename` from disk
	*/
	struct inode_t *inode = (struct inode_t *)malloc(sizeof(struct inode_t));
	for (int i = 0; i < NUM_INODES; i++)
	{
		simplefs_readInode(i, inode);
		if (strcmp(inode->name, filename) == 0)
		{
			for (int i = 0; i < MAX_FILE_SIZE; i++)
			{
				if (inode->direct_blocks[i] != -1)
					simplefs_freeDataBlock(inode->direct_blocks[i]);
			}
			simplefs_freeInode(i);
			break;
		}
	}
}

int simplefs_open(char *filename)
{
	/*
		open file with name `filename`
	*/
	struct inode_t *inode = (struct inode_t *)malloc(sizeof(struct inode_t));
	for (int i = 0; i < NUM_INODES; i++)
	{
		simplefs_readInode(i, inode);
		if (strcmp(inode->name, filename) == 0)
		{
			for (int j = 0; j < MAX_OPEN_FILES; j++)
			{
				if (file_handle_array[j].inode_number == -1)
				{
					file_handle_array[j].inode_number = i;
					file_handle_array[j].offset = 0;
					return j;
				}
			}
		}
	}
	return -1;
}

void simplefs_close(int file_handle)
{
	/*
		close file pointed by `file_handle`
	*/
	file_handle_array[file_handle].inode_number = -1;
	file_handle_array[file_handle].offset = 0;
}

int simplefs_read(int file_handle, char *buf, int nbytes)
{
	/*
		read `nbytes` of data into `buf` from file pointed by `file_handle` starting at current offset
	*/
	int inode_number = file_handle_array[file_handle].inode_number;
	int offset = file_handle_array[file_handle].offset;
	if (inode_number == -1)
		return -1;
	struct inode_t *inode = (struct inode_t *)malloc(sizeof(struct inode_t));
	simplefs_readInode(inode_number, inode);
	if (offset + nbytes > inode->file_size)
		return -1;
	int start_block = offset / BLOCKSIZE;
	int read_bytes = 0;
	int curr_block = start_block;
	int read_offset = offset % BLOCKSIZE;
	char *tempBuf = (char *)malloc(BLOCKSIZE);
	memset(tempBuf, 0, BLOCKSIZE);
	while (read_bytes < nbytes)
	{
		simplefs_readDataBlock(inode->direct_blocks[curr_block], tempBuf);
		int i = read_offset;
		do
		{
			buf[read_bytes++] = tempBuf[i++];
		} while ((offset + read_bytes) % BLOCKSIZE != 0 && read_bytes < nbytes);
		read_offset = 0;
		curr_block++;
	}
	assert(read_bytes == nbytes);
	return 0;
}

int simplefs_write(int file_handle, char *buf, int nbytes)
{
	/*
		write `nbytes` of data from `buf` to file pointed by `file_handle` starting at current offset
	*/
	int inode_number = file_handle_array[file_handle].inode_number;
	int offset = file_handle_array[file_handle].offset;
	if (inode_number == -1)
		return -1;
	struct inode_t *inode = (struct inode_t *)malloc(sizeof(struct inode_t));
	simplefs_readInode(inode_number, inode);
	int start_block = offset / BLOCKSIZE;
	int end_block = (offset + nbytes - 1) / BLOCKSIZE;
	if(end_block >= MAX_FILE_SIZE)
		return -1;
	int start_alloc = (offset % BLOCKSIZE == 0) ? start_block : start_block + 1;
	char *tempBuf = (char *)malloc(BLOCKSIZE);		
	memset(tempBuf, 0, BLOCKSIZE);
	for (int i = start_alloc; i <= end_block; i++)
	{
		int k = simplefs_allocDataBlock();
		if (k == -1)
		{
			for (int j = start_alloc; j < i; j++)
			{
				simplefs_freeDataBlock(inode->direct_blocks[j]);
				inode->direct_blocks[j] = -1;
			}
			return -1;
		}
		inode->direct_blocks[i] = k;
		simplefs_writeDataBlock(inode->direct_blocks[i], tempBuf);
	}
	int curr_block = start_block;
	int write_offset = offset % BLOCKSIZE;
	int write_bytes = 0;
	while (write_bytes < nbytes)
	{
		simplefs_readDataBlock(inode->direct_blocks[curr_block], tempBuf);
		int i = write_offset;
		do
		{
			tempBuf[i++] = buf[write_bytes++];
		} while ((offset + write_bytes) % BLOCKSIZE != 0 && write_bytes < nbytes);
		simplefs_writeDataBlock(inode->direct_blocks[curr_block], tempBuf);
		write_offset = 0;
		curr_block++;
	}
	inode->file_size += nbytes;
	simplefs_writeInode(inode_number, inode);
	assert(write_bytes == nbytes);

	return 0;
}

int simplefs_seek(int file_handle, int nseek)
{
	/*
	   increase `file_handle` offset by `nseek`
	*/
		int inode_number = file_handle_array[file_handle].inode_number;
		int offset = file_handle_array[file_handle].offset;
		if (inode_number == -1)
			return -1;
		struct inode_t *inode = (struct inode_t *)malloc(sizeof(struct inode_t));
		simplefs_readInode(inode_number, inode);
		if (offset + nseek > inode->file_size || offset + nseek < 0)
			return -1;
		file_handle_array[file_handle].offset = offset + nseek;
	return 0;
}