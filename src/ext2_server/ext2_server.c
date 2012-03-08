
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <google/protobuf-c/protobuf-c-rpc.h>

#include "../commons/rpc_layer.pb-c.h"

#include "ext2_server.h"


static t_ext2_server *ext2_server;

// -----------  RPC Functions  -----------------
static void ext2_service__read_dir(RpcLayer__Ext2Services_Service *, const RpcLayer__ReadDirRequest *, RpcLayer__ReadDirResponse_Closure, void *);
static RpcLayer__Ext2Services_Service ext2_service = RPC_LAYER__EXT2_SERVICES__INIT(ext2_service__);


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

	const char name[] = "socket";

	protobuf_c_rpc_server_new(PROTOBUF_C_RPC_ADDRESS_LOCAL, name, (ProtobufCService *) &ext2_service, NULL);

	for (;;)
		protobuf_c_dispatch_run(protobuf_c_dispatch_default());

}

static void ext2_service__read_dir(RpcLayer__Ext2Services_Service *service,
									const RpcLayer__ReadDirRequest *request,
									RpcLayer__ReadDirResponse_Closure closure,
									void *closure_data) {

	RpcLayer__ReadDirResponse response = RPC_LAYER__READ_DIR_RESPONSE__INIT;

	t_list *elements = ext2_list_dir(ext2_server->fs, "/");

	if (elements != NULL && list_size(elements) > 0) {

		response.n_elements = list_size(elements);
		response.elements = calloc(list_size(elements), sizeof(char*));

		int index;
		for(index = 0; index < list_size(elements); index ++){
			response.elements[index] = list_get(elements, index);
		}

	} else {
		response.n_elements = 0;
	}

	closure(&response, closure_data);

	list_destroy_and_destroy_elements(elements, free);
}

int main(int argc, char **argv) {

	ext2_server = ext2_server_create("dev/ext2.disk");
	ext2_server_run(ext2_server);

	return EXIT_FAILURE;
}

