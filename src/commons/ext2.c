#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "ext2.h"

t_ext2 *ext2_create(char *device) {
	t_ext2 *fs = malloc(sizeof(t_ext2));

	fs->device = (uint8_t*) device;

	//Main Superblock
	fs->superblock = (t_ext2_superblock*) (device + 1024);

	if (fs->superblock->magic != EXT2_SUPER_MAGIC) {
		perror("Not ext2 partition");
		exit(EXIT_FAILURE);
	}

	return fs;
}

uint8_t *ext2_get_block(t_ext2 *self, uint32_t block_number){
	if( self->superblock->blockcount <= block_number ){
		return NULL;
	}

	return self->device + (block_number * ext2_get_block_size(self));
}

t_ext2_block_group *ext2_get_block_group(t_ext2 *self, uint32_t group_number){

	if( ext2_get_number_of_block_group(self) <= group_number ){
		return NULL;
	}

	t_ext2_block_group *block_group = malloc( sizeof(t_ext2_block_group) );

	memset(block_group, 0, sizeof(t_ext2_block_group));

	block_group->number = group_number;

	uint32_t first_block = (group_number * self->superblock->blockper_group) + self->superblock->first_data_block;

	block_group->first_block = first_block;

	block_group->has_superblock = ext2_has_superblock(group_number);

	uint8_t *block_bitmap;
	uint8_t *inode_bitmap;

	if( block_group->has_superblock ){
		block_group->superblock = (void*)(self->device + first_block * ext2_get_block_size(self));
		first_block++;
		block_group->block_group_descriptor = (void*)(self->device + first_block * ext2_get_block_size(self));
		first_block++;
		block_bitmap = self->device + block_group->block_group_descriptor->block_bitmap * ext2_get_block_size(self);
		inode_bitmap = self->device + block_group->block_group_descriptor->inode_bitmap * ext2_get_block_size(self);
		block_group->inodes_table = (void*)(self->device + block_group->block_group_descriptor->inode_table * ext2_get_block_size(self));
	} else {
		block_bitmap = self->device + first_block * ext2_get_block_size(self);
		first_block++;
		inode_bitmap = self->device + first_block * ext2_get_block_size(self);
		first_block++;
		block_group->inodes_table = (void*)(self->device + first_block * ext2_get_block_size(self));
		first_block++;
	}

	block_group->block_bitmap = bitarray_create(block_bitmap, ext2_get_block_size(self));
	block_group->inode_bitmap = bitarray_create(inode_bitmap, ext2_get_block_size(self));

	return NULL;
}

inline uint32_t ext2_get_number_of_block_group(t_ext2 *self){
	return floor((double)(self->superblock->blockcount) / (double)(self->superblock->blockper_group));
}

bool ext2_has_superblock(uint32_t group_number) {

	if (group_number == 0 || group_number == 1) {
		return true;
	}

	if (group_number % 3 == 0 || group_number % 5 == 0 || group_number % 7 == 0) {
		return true;
	}

	return false;
}

inline uint32_t ext2_get_block_size(t_ext2 *self){
	return 1024 << self->superblock->log_block_size;
}
