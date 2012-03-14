
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <google/protobuf-c/protobuf-c-rpc.h>

#include <rpc_layer.pb-c.h>

#include "ext2_server.h"

#define FS_FULL_READ_PRIV	S_IREAD | S_IRGRP | S_IROTH
#define FS_FULL_WRITE_PRIV	 S_IWRITE | S_IWGRP | S_IWOTH

static t_ext2_server *ext2_server;

// -----------  RPC Functions  -----------------
static void ext2_service__read_dir(RpcLayer__RemoteExt2_Service *, const RpcLayer__ReadDirRequest *, RpcLayer__ReadDirResponse_Closure, void *);
static void ext2_service__get_attr(RpcLayer__RemoteExt2_Service *, const RpcLayer__GetAttrRequest *, RpcLayer__GetAttrResponse_Closure, void *);
static void ext2_service__read(RpcLayer__RemoteExt2_Service *, const RpcLayer__ReadRequest *, RpcLayer__ReadResponse_Closure, void *);
static RpcLayer__RemoteExt2_Service ext2_service = RPC_LAYER__REMOTE_EXT2__INIT(ext2_service__);


t_ext2_server *ext2_server_create(char* socket_path, char* device_path){
	t_ext2_server *ext2_server = malloc(sizeof(t_ext2_server));
	struct stat sb;

	ext2_server->socket_file = socket_path;

	ext2_server->log = log_create(NULL, "Ext2 Server", true, LOG_LEVEL_DEBUG);
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

	log_info(ext2_server->log, "[+] Initialize Ext2 FS ... ");
	ext2_server->fs = ext2_create(ext2_server->device);

	return ext2_server;
}

void ext2_server_run(t_ext2_server *self){

	log_info(ext2_server->log, "[+] Initialize RPC Server ... ");

	protobuf_c_rpc_server_new(PROTOBUF_C_RPC_ADDRESS_LOCAL, self->socket_file, (ProtobufCService *) &ext2_service, NULL);

	log_info(ext2_server->log, "[+] Listening ... ");

	for (;;)
		protobuf_c_dispatch_run(protobuf_c_dispatch_default());

}

static void ext2_service__read_dir(RpcLayer__RemoteExt2_Service *service,
									const RpcLayer__ReadDirRequest *request,
									RpcLayer__ReadDirResponse_Closure closure,
									void *closure_data) {

	RpcLayer__ReadDirResponse response = RPC_LAYER__READ_DIR_RESPONSE__INIT;

	t_list *elements = ext2_list_dir(ext2_server->fs, request->path);

	if (elements != NULL && list_size(elements) > 0) {

		response.n_elements = list_size(elements);
		response.elements = calloc(list_size(elements), sizeof(char*));

		int index;
		for(index = 0; index < list_size(elements); index ++){
			t_ext2_inode_entry *entry = list_get(elements, index);
			response.elements[index] = entry->name;
		}

	} else {
		response.n_elements = 0;
	}

	closure(&response, closure_data);

	if (elements != NULL){
		list_destroy_and_destroy_elements(elements, (void*)ext2_inode_entry_free);
	}
}

static void ext2_service__get_attr(RpcLayer__RemoteExt2_Service *service,
									const RpcLayer__GetAttrRequest *request,
									RpcLayer__GetAttrResponse_Closure closure,
									void *closure_data){

	RpcLayer__GetAttrResponse response = RPC_LAYER__GET_ATTR_RESPONSE__INIT;

	t_ext2_inode *element = ext2_get_element_inode(ext2_server->fs, request->path);

	if( element != NULL ){
		response.fileexist = true;

		if( EXT2_INODE_HAS_MODE_FLAG(element, EXT2_IFDIR) ){
			response.mode = S_IFDIR | FS_FULL_READ_PRIV;
		} else {
			response.mode = S_IFREG | FS_FULL_READ_PRIV | FS_FULL_WRITE_PRIV;
		}

		response.nlinks = element->links_count;
		response.size = element->size;
	} else {
		response.fileexist = false;
	}

	closure(&response, closure_data);
}

static void ext2_service__read(RpcLayer__RemoteExt2_Service *service,
									const RpcLayer__ReadRequest *request,
									RpcLayer__ReadResponse_Closure closure,
									void *closure_data){

	RpcLayer__ReadResponse response = RPC_LAYER__READ_RESPONSE__INIT;

	t_ext2_inode *element = ext2_get_element_inode(ext2_server->fs, request->path);

	if( element == NULL ){
		response.error = true;
		closure(&response, closure_data);
		return;
	}

	if( request->offset > element->size ){
		response.data.len = 0;
		closure(&response, closure_data);
		return;
	}

	size_t size_to_read = request->length;
	if( request->offset + request->length > element->size ){
		size_to_read = element->size - request->offset;
	}

	uint8_t buff[size_to_read];

	response.data.data = buff;
	response.data.len = size_to_read;

	ext2_read_inode_data(ext2_server->fs, element, request->offset, size_to_read, buff);

	closure(&response, closure_data);
}

int main(int argc, char **argv) {

	ext2_server = ext2_server_create("../socket", "dev/ext2.disk");
	ext2_server_run(ext2_server);

	return EXIT_FAILURE;
}

