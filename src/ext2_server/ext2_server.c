
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "ext2_server.h"


t_ext2_server *ext2_server_create(char* device_path){
	t_ext2_server *ext2_server = malloc(sizeof(t_ext2_server));
	struct stat sb;

	ext2_server->device_path = strdup(device_path);
	ext2_server->device_desc = open(device_path, O_RDWR);

	if (ext2_server->device_desc == -1) {
		perror("Invalid device path");
		exit(EXIT_SUCCESS);
	}

	if (fstat(ext2_server->device_desc, &sb) == -1) {
		perror("Invalid device descriptor");
		exit(EXIT_SUCCESS);
	}

	if (!S_ISREG(sb.st_mode)) {
		fprintf(stderr, "%s is not a file\n", device_path);
		exit(EXIT_SUCCESS);
	}

	ext2_server->device = mmap(0, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, ext2_server->device_desc, 0);

	if (ext2_server->device == MAP_FAILED) {
		perror("mmap");
		exit(EXIT_SUCCESS);
	}

	ext2_server->fs = ext2_create(ext2_server->device);

	return ext2_server;
}

void ext2_server_run(t_ext2_server *self){


	t_list *elements = ext2_list_dir(self->fs, "/");

	if( elements != NULL ){

		void _print(char *entry){
			printf("%s\n", entry);
		}

		list_iterate(elements, _print);
	}

	list_destroy_and_destroy_elements(elements, free);

}
