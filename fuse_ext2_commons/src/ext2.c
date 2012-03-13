#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "ext2.h"

#include "string_utils.h"


static t_list *ext2_get_block_directory_entrys(t_ext2*, uint8_t *block, t_list *list_to_fill);


//  ---  iNodes Functions  ---
static t_list *ext2_list_inode(t_ext2 *, t_ext2_inode *root);
static t_ext2_inode *ext2_find_inode(t_ext2 *, t_ext2_inode *root,  char **path);
static uint32_t ext2_get_inode_block_entry(t_ext2 *, t_ext2_inode *node, uint32_t entry_index);
static uint32_t ext2_find_block_entry(t_ext2 *, uint32_t base_block, uint8_t base_block_level, uint32_t entry_index);


//  ---  Block Group Functions  ---
static t_ext2_block_group *ext2_block_group_create(uint16_t group_number, uint32_t first_block, bool has_superblock, uint8_t *block_bitmap, uint8_t *inode_bitmap, uint32_t block_size);
static void ext2_block_group_free(t_ext2_block_group *block_group);
static t_ext2_block_group *ext2_get_block_group(t_ext2*, uint16_t group_number);
static uint32_t ext2_get_number_of_block_group(t_ext2 *);


//  ---  iNodes Entries Functions  ---
t_ext2_inode_entry *ext2_inode_entry_create(t_ext2 *fs, t_ext2_directory_entry *dir_entry);



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
	fs->entries_per_block = fs->block_size / sizeof(uint32_t);
	fs->inode_blocks_amount = ext2_get_inode_blocks_amount(fs);
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

	t_ext2_block_group *block_group = ext2_block_group_create(group_number, first_block, has_superblock, block_bitmap, inode_bitmap, self->block_size);

	block_group->superblock = superblock;
	block_group->block_group_descriptor = block_group_descriptor;
	block_group->inodes_table = inodes_table;

	return block_group;
}

inline t_ext2_inode	*ext2_get_root_inode(t_ext2 *self){
	t_ext2_block_group *block_group = ext2_get_block_group(self, 0);
	t_ext2_inode *inode = &block_group->inodes_table[EXT2_ROOT_INODE_INDEX];
	ext2_block_group_free(block_group);
	return inode;
}

t_ext2_inode *ext2_get_inode(t_ext2 *self, uint32_t inode_index){
	uint16_t block_group_index = (inode_index - 1) / self->superblock->inodeper_group;
	uint32_t local_index = (inode_index - 1) % self->superblock->inodeper_group;

	t_ext2_block_group *block_group = ext2_get_block_group(self, block_group_index);
	t_ext2_inode *inode = &block_group->inodes_table[local_index];
	ext2_block_group_free(block_group);

	return inode;
}

t_list *ext2_list_dir(t_ext2 *self, char *dir_path){
	t_ext2_inode *inode = ext2_get_element_inode(self, dir_path);

	if( inode == NULL ){
		return NULL;
	}

	return ext2_list_inode(self, inode);
}

t_ext2_inode *ext2_get_element_inode(t_ext2 *self, char *path){
	if( strcmp("/", path) == 0 ){
		return ext2_get_root_inode(self);
	}

	char **path_stack = string_utils_split(path+1,"/");

	t_ext2_inode *inode = ext2_find_inode(self, ext2_get_root_inode(self), path_stack);

	int index = 0;
	for(; path_stack[index] != NULL ;index++){
		free(path_stack[index]);
	}
	free(path_stack);

	return inode;
}

inline static t_ext2_inode *ext2_find_inode(t_ext2 *self, t_ext2_inode *root,  char **path) {

	if( path[0] == NULL ){
		return root;
	}

	t_list *subelements = ext2_list_inode(self, root);

	bool _find_inode(t_ext2_inode_entry *entry){
		return strcmp(path[0], entry->name) == 0;
	}

	t_ext2_inode *inode = list_find(subelements, (void*)_find_inode);

	list_destroy_and_destroy_elements(subelements, (void*)ext2_inode_entry_free);

	if( inode == NULL ){
		return NULL;
	}

	return ext2_find_inode(self, inode, &path[1]);

}

inline static t_list *ext2_list_inode(t_ext2 *self, t_ext2_inode *root) {

	if (!EXT2_INODE_HAS_MODE_FLAG(root, EXT2_IFDIR)) {
		return NULL;
	}

	t_list *list = list_create();

	int block_amount = root->blocks / (2 << self->superblock->log_block_size);

	int index;
	for (index = 0; index < block_amount; index++) {

		uint32_t block_entry = ext2_get_inode_block_entry(self, root, index);

		if (block_entry != 0) {
			ext2_get_block_directory_entrys(self, ext2_get_block(self, block_entry), list);
		}
	}

	return list;
}

static uint32_t ext2_get_inode_block_entry(t_ext2 *self, t_ext2_inode *node, uint32_t entry_index) {

	int base_block_index = 0, blocks_offset = 0;
	for (; blocks_offset < entry_index && base_block_index < (sizeof(EXT2_INODES_INDIRECTION_LEVEL) / sizeof(uint8_t)); base_block_index++) {

		blocks_offset += powl(self->entries_per_block, EXT2_INODES_INDIRECTION_LEVEL[base_block_index]);

	}

	uint8_t level = EXT2_INODES_INDIRECTION_LEVEL[base_block_index];

	if (level == 0) {
		return node->block[base_block_index];
	}

	return ext2_find_block_entry(self, node->block[base_block_index], level, entry_index);
}

static uint32_t ext2_find_block_entry(t_ext2 *self, uint32_t base_block, uint8_t base_block_level, uint32_t entry_index) {
	uint32_t *current_block = (uint32_t*)ext2_get_block(self, entry_index);

	if (base_block_level == 0) {
		return current_block[entry_index];
	}

	long elements_in_sublevels = powl(self->entries_per_block, base_block_level - 1);

	int index = entry_index / elements_in_sublevels;

	return ext2_find_block_entry(self, current_block[index], base_block_level - 1, entry_index - index * elements_in_sublevels);
}


static t_list *ext2_get_block_directory_entrys(t_ext2 *self, uint8_t *block, t_list *list_to_fill){

	int offset = 0;

	while( offset < self->block_size ){
		t_ext2_directory_entry *entry = (t_ext2_directory_entry*)(block + offset);

		if( entry->inode != 0 ){
			list_add(list_to_fill, ext2_inode_entry_create(self, entry));
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

inline uint32_t ext2_get_inode_blocks_amount(t_ext2 *self){
	uint32_t blocks = 0;

	int index;
	for (index = 0; index < (sizeof(EXT2_INODES_INDIRECTION_LEVEL) / sizeof(uint8_t)); index++) {

		long blocks_in_level = powl(self->entries_per_block, EXT2_INODES_INDIRECTION_LEVEL[index]);

		blocks = blocks + blocks_in_level;
	}

	return blocks;
}

// ---------------------------------------------------------------------------------
// ------------------------------ INTERNAL FUNCTIONS -------------------------------
// ---------------------------------------------------------------------------------

inline static t_ext2_block_group *ext2_block_group_create(uint16_t group_number, uint32_t first_block, bool has_superblock, uint8_t *block_bitmap, uint8_t *inode_bitmap, uint32_t block_size){
	t_ext2_block_group *block_group = malloc( sizeof(t_ext2_block_group) );

	memset(block_group, 0, sizeof(t_ext2_block_group));

	block_group->number = group_number;
	block_group->first_block = first_block;
	block_group->has_superblock = has_superblock;

	block_group->block_bitmap = bitarray_create(block_bitmap, block_size);
	block_group->inode_bitmap = bitarray_create(inode_bitmap, block_size);

	return block_group;
}

inline static void ext2_block_group_free(t_ext2_block_group *block_group){
	bitarray_destroy(block_group->block_bitmap);
	bitarray_destroy(block_group->inode_bitmap);
	free(block_group);
}

t_ext2_inode_entry *ext2_inode_entry_create(t_ext2 *fs, t_ext2_directory_entry *dir_entry){
	t_ext2_inode_entry *inode_entry = malloc( sizeof(t_ext2_inode_entry) );

	inode_entry->inode_index = dir_entry->inode;

	inode_entry->name = malloc( dir_entry->name_len + 1 );
	memcpy(inode_entry->name, dir_entry->name, dir_entry->name_len);
	inode_entry->name[dir_entry->name_len] = '\0';

	inode_entry->inode = ext2_get_inode(fs, dir_entry->inode);

	return inode_entry;
}

void ext2_inode_entry_free(t_ext2_inode_entry *self){
	free(self->name);
	free(self);
}
