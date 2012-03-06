/*
 * ext2.h
 *
 *  Created on: 05/03/2012
 *      Author: fviale
 */

#ifndef EXT2_H_
#define EXT2_H_

	#include <stdint.h>
	#include <stdbool.h>

	#include "bitarray.h"

	#define EXT2_SUPER_MAGIC	0xEF53

	// t_ext2_superblock -> state
	#define EXT2_VALID_FS	1	//Unmounted cleanly
	#define EXT2_ERROR_FS	2	//Errors detected

	// t_ext2_superblock -> errors
	#define EXT2_ERRORS_CONTINUE	1	//continue as if nothing happened
	#define EXT2_ERRORS_RO			2	//remount read-only
	#define EXT2_ERRORS_PANIC		3	//cause a kernel panic

	// t_ext2_superblock -> creator_os
	#define EXT2_OS_LINUX	0	//Linux
	#define EXT2_OS_HURD	1	//GNU HURD
	#define EXT2_OS_MASIX	2	//MASIX
	#define EXT2_OS_FREEBSD	3	//FreeBSD
	#define EXT2_OS_LITES	4	//Lites

	// t_ext2_superblock -> rev_level
	#define EXT2_GOOD_OLD_REV	0	//Revision 0
	#define EXT2_DYNAMIC_REV	1	//Revision 1 with variable inode sizes, extended attributes, etc.

	// t_ext2_inode -> rev_level
	#define EXT2_IFSOCK	0xC000	//socket
	#define EXT2_IFLNK	0xA000	//symbolic link
	#define EXT2_IFREG	0x8000	//regular file
	#define EXT2_IFBLK	0x6000	//block device
	#define EXT2_IFDIR	0x4000	//directory
	#define EXT2_IFCHR	0x2000	//character device
	#define EXT2_IFIFO	0x1000	//fifo

	typedef struct {
		uint32_t inodecount;
		uint32_t blockcount;
		uint32_t r_blockcount;
		uint32_t free_blockcount;
		uint32_t free_inodecount;
		uint32_t first_data_block;
		uint32_t log_block_size;
		uint32_t log_frag_size;
		uint32_t blockper_group;
		uint32_t fragper_group;
		uint32_t inodeper_group;
		uint32_t mtime;
		uint32_t wtime;
		uint16_t mnt_count;
		uint16_t max_mnt_count;
		uint16_t magic;
		uint16_t state;
		uint16_t errors;
		uint16_t minor_rev_level;
		uint32_t lastcheck;
		uint32_t checkinterval;
		uint32_t creator_os;
		uint32_t rev_level;
		uint16_t def_resuid;
		uint16_t def_resgid;

		//-- EXT2_DYNAMIC_REV Specific --
		uint32_t first_ino;
		uint16_t inode_size;
		uint16_t block_group_nr;
		uint32_t feature_compat;
		uint32_t feature_incompat;
		uint32_t feature_ro_compat;
		uint8_t uuid[16];
		uint8_t volume_name[16];
		uint8_t last_mounted[64];
		uint32_t algo_bitmap;

	}__attribute__ ((packed)) t_ext2_superblock;

	typedef struct {
		uint32_t block_bitmap;
		uint32_t inode_bitmap;
		uint32_t inode_table;
		uint16_t free_blocks_count;
		uint16_t free_inodes_count;
		uint16_t used_dirs_count;
		uint16_t pad;
		uint8_t reserved[12];
	}__attribute__ ((packed)) t_ext2_block_group_descriptor;

	typedef struct {
		uint16_t mode;
		uint16_t uid;
		uint32_t size;
		uint32_t atime;
		uint32_t ctime;
		uint32_t mtime;
		uint32_t dtime;
		uint16_t gid;
		uint16_t links_count;
		uint32_t blocks;
		uint32_t flags;
		uint32_t osd1;
		uint32_t block[15];
		uint32_t generation;
		uint32_t file_acl;
		uint32_t dir_acl;
		uint32_t faddr;
		uint8_t osd2[12];
	}__attribute__ ((packed)) t_ext2_inode;

	typedef struct {
		uint32_t number;
		uint32_t first_block;
		bool has_superblock;

		t_ext2_superblock *superblock;
		t_ext2_block_group_descriptor *block_group_descriptor;

		t_bitarray *block_bitmap;
		t_bitarray *inode_bitmap;

		t_ext2_inode *inodes_table;

	} t_ext2_block_group;

	typedef struct {
		uint8_t *device;
		t_ext2_superblock *superblock;
	} t_ext2;

	t_ext2				*ext2_create(char *device);
	uint8_t 			*ext2_get_block(t_ext2 *, uint16_t block_number);
	t_ext2_block_group	*ext2_get_block_group(t_ext2*, uint16_t group_number);
	uint32_t 			 ext2_get_number_of_block_group(t_ext2 *);
	bool 	 			 ext2_has_superblock(uint16_t group_number);
	uint32_t 	 		 ext2_get_block_size(t_ext2 *);

#endif /* EXT2_H_ */
