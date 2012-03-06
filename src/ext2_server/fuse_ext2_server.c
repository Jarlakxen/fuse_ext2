#include <stdio.h>
#include <stdlib.h>
#include "ext2_server.h"

int main(void) {
	t_ext2_server * server = ext2_server_create("dev/ext2.disk");

	ext2_server_run(server);

	return EXIT_SUCCESS;
}

