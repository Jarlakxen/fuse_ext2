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
		uint16_t i_mode;
		uint16_t i_uid;
		uint32_t i_size;
		uint32_t i_atime;
		uint32_t i_ctime;
		uint32_t i_mtime;
		uint32_t i_dtime;
		uint16_t i_gid;
		uint16_t i_links_count;
		uint32_t i_blocks;
		uint32_t i_flags;
		uint32_t i_osd1;
		uint32_t i_block[15];
		uint32_t i_generation;
		uint32_t i_file_acl;
		uint32_t i_dir_acl;
		uint32_t i_faddr;
		uint8_t i_osd2[12];
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
	uint8_t 			*ext2_get_block(t_ext2 *, uint32_t block_number);
	t_ext2_block_group	*ext2_get_block_group(t_ext2*, uint32_t group_number);
	uint32_t 			 ext2_get_number_of_block_group(t_ext2 *);
	bool 	 			 ext2_has_superblock(uint32_t group_number);
	uint32_t 	 		 ext2_get_block_size(t_ext2 *);

#endif /* EXT2_H_ */
