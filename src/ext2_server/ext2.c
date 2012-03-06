#include <stdlib.h>
#include "../includes/ext2.h"

t_ext2 *ext2_create(char *device){
	t_ext2 *fs = malloc( sizeof(t_ext2) );

	fs->device = (uint8_t*)device;

	//Main Superblock
	fs->superblock = (t_ext2_superblock*)(device + 1024);

	if(	fs->superblock->magic != EXT2_SUPER_MAGIC ){
		perror("Not ext2 partition");
		exit(EXIT_FAILURE);
	}

	return fs;
}
