#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "ext2.h"


static t_list *ext2_get_block_directory_entrys(t_ext2*, uint8_t *block, t_list *list_to_fill);

static t_list *ext2_list_inode(t_ext2 *, t_ext2_inode *root);

static t_ext2_block_group *ext2_get_block_group(t_ext2*, uint16_t group_number);
static uint32_t ext2_get_number_of_block_group(t_ext2 *);

//  ---  Block Group Functions  ---
static t_ext2_block_group *ext2_create_block_group(uint16_t group_number, uint32_t first_block, bool has_superblock, uint8_t *block_bitmap, uint8_t *inode_bitmap, uint32_t block_size);
static void ext2_free_block_group(t_ext2_block_group *block_group);


t_ext2 *ext2_create(char *device) {
	t_ext2 *fs = malloc(sizeof(t_ext2));

	fs->device = (uint8_t*) device;

	//Main Superblock
	fs->superblock = (t_ext2_superblock*) (device + 1024);

	if (fs->superblock->magic != EXT2_SUPER_MAGIC) {
		perror("Not ext2 partition");
		exit(EXIT_FAILURE);
	}

	fs->block_size = ext2_get_block_size(fs);
	fs->number_of_block_groups = ext2_get_number_of_block_group(fs);

	return fs;
}

inline uint8_t *ext2_get_block(t_ext2 *self, uint16_t block_number){
	if( self->superblock->blockcount <= block_number ){
		return NULL;
	}

	return self->device + (block_number * self->block_size);
}

t_ext2_block_group *ext2_get_block_group(t_ext2 *self, uint16_t group_number){

	if( self->number_of_block_groups <= group_number ){
		return NULL;
	}

	uint32_t first_block = (group_number * self->superblock->blockper_group) + self->superblock->first_data_block;

	bool has_superblock = ext2_has_superblock(group_number);

	void *superblock = NULL;
	void *block_group_descriptor = NULL;
	void *block_bitmap = NULL;
	void *inode_bitmap = NULL;
	void *inodes_table = NULL;

	if( has_superblock ){
		superblock = ext2_get_block(self, first_block);
		first_block++;
		block_group_descriptor = ext2_get_block(self, first_block);
		first_block++;
		block_bitmap = ext2_get_block(self, ((t_ext2_block_group_descriptor*)block_group_descriptor)->block_bitmap);
		inode_bitmap = ext2_get_block(self, ((t_ext2_block_group_descriptor*)block_group_descriptor)->inode_bitmap);
		inodes_table = ext2_get_block(self, ((t_ext2_block_group_descriptor*)block_group_descriptor)->inode_table);
	} else {
		block_bitmap = ext2_get_block(self, first_block);
		first_block++;
		inode_bitmap = ext2_get_block(self, first_block);
		first_block++;
		inodes_table = ext2_get_block(self, first_block);
		first_block++;
	}

	t_ext2_block_group *block_group = ext2_create_block_group(group_number, first_block, has_superblock, block_bitmap, inode_bitmap, self->block_size);

	block_group->superblock = superblock;
	block_group->block_group_descriptor = block_group_descriptor;
	block_group->inodes_table = inodes_table;

	return block_group;
}

inline t_ext2_inode	*ext2_get_root_inode(t_ext2 *self){
	t_ext2_block_group *block_group = ext2_get_block_group(self, 0);
	t_ext2_inode *inode = &block_group->inodes_table[EXT2_ROOT_INODE_INDEX];
	ext2_free_block_group(block_group);
	return inode;
}

t_list *ext2_list_dir(t_ext2 *self, char *dir_path){

	if( strcmp("/", dir_path) == 0 ){
		return ext2_list_inode(self, ext2_get_root_inode(self));
	}

	return NULL;
}

inline static t_list *ext2_list_inode(t_ext2 *self, t_ext2_inode *root) {

	if (!EXT2_INODE_HAS_MODE_FLAG(root, EXT2_IFDIR)) {
		return NULL;
	}

	t_list *list = list_create();

	int block_amount = root->blocks / (2 << self->superblock->log_block_size);

	int cont;

	for (cont = 0; cont < block_amount; cont++) {
		if (root->block[cont] != 0) {
			ext2_get_block_directory_entrys(self, ext2_get_block(self, root->block[cont]), list);
		}
	}

	return list;
}

static t_list *ext2_get_block_directory_entrys(t_ext2 *self, uint8_t *block, t_list *list_to_fill){

	int offset = 0;

	while( offset < self->block_size ){
		t_ext2_directory_entry *entry = (t_ext2_directory_entry*)(block + offset);

		if( entry->inode != 0 ){
			char *name = calloc(1, entry->name_len);

			memcpy(name, entry->name, entry->name_len);

			list_add(list_to_fill, name);
		}

		offset = offset + entry->rec_len;
	}

	return list_to_fill;
}

inline uint32_t ext2_get_number_of_block_group(t_ext2 *self){
	return floor((double)(self->superblock->blockcount - self->superblock->first_data_block) / (double)(self->superblock->blockper_group));
}

bool ext2_has_superblock(uint16_t group_number) {

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

// ---------------------------------------------------------------------------------
// ------------------------------ INTERNAL FUNCTIONS -------------------------------
// ---------------------------------------------------------------------------------

inline static t_ext2_block_group *ext2_create_block_group(uint16_t group_number, uint32_t first_block, bool has_superblock, uint8_t *block_bitmap, uint8_t *inode_bitmap, uint32_t block_size){
	t_ext2_block_group *block_group = malloc( sizeof(t_ext2_block_group) );

	memset(block_group, 0, sizeof(t_ext2_block_group));

	block_group->number = group_number;
	block_group->first_block = first_block;
	block_group->has_superblock = has_superblock;

	block_group->block_bitmap = bitarray_create(block_bitmap, block_size);
	block_group->inode_bitmap = bitarray_create(inode_bitmap, block_size);

	return block_group;
}

inline static void ext2_free_block_group(t_ext2_block_group *block_group){
	bitarray_destroy(block_group->block_bitmap);
	bitarray_destroy(block_group->inode_bitmap);
	free(block_group);
}
