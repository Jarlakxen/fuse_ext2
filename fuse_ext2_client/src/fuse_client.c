#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <fuse.h>
#include <sys/stat.h>
#include <google/protobuf-c/protobuf-c-rpc.h>

#include <rpc_layer.pb-c.h>
#include <log.h>

ProtobufCService *service = NULL;
t_log *log = NULL;

static int fuse_ext2_getattr(const char *path, struct stat *stbuf) {
	int res = 0;

	RpcLayer__GetAttrRequest query = RPC_LAYER__GET_ATTR_REQUEST__INIT;
	protobuf_c_boolean is_done = 0;

	query.path = strdup(path);

	void _handle_response(const RpcLayer__GetAttrResponse *result, void *closure_data) {

		*(protobuf_c_boolean *) closure_data = 1;

		if( !result->fileexist ) {
			res = -ENOENT;
			return;
		}

		stbuf->st_mode = result->mode;
		stbuf->st_size = result->size;
		stbuf->st_nlink = result->nlinks;
		stbuf->st_blocks = result->blocks;
	}

	rpc_layer__remote_ext2__get_attr(service, &query, _handle_response, &is_done);

	while (!is_done)
		protobuf_c_dispatch_run(protobuf_c_dispatch_default());

	return res;
}

static int fuse_ext2_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
	RpcLayer__ReadDirRequest query = RPC_LAYER__READ_DIR_REQUEST__INIT;
	protobuf_c_boolean is_done = 0;

	query.path = strdup(path);

	void _handle_response(const RpcLayer__ReadDirResponse *result, void *closure_data) {

		int index;
		for (index = 0; index < result->n_elements; index++) {
			filler(buf, result->elements[index], NULL, 0);
		}

		*(protobuf_c_boolean *) closure_data = 1;
	}

	rpc_layer__remote_ext2__read_dir(service, &query, _handle_response, &is_done);

	while (!is_done)
		protobuf_c_dispatch_run(protobuf_c_dispatch_default());

	free(query.path);

	return 0;
}

static int fuse_ext2_open(const char *path, struct fuse_file_info *fi) {
	return 0;
}

static int fuse_ext2_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
	RpcLayer__ReadRequest query = RPC_LAYER__READ_REQUEST__INIT;
	protobuf_c_boolean is_done = 0;
	int res = 0;

	query.path = strdup(path);
	query.offset = offset;
	query.length = size;

	void _handle_response(const RpcLayer__ReadResponse *result, void *closure_data) {

		if( result->error ) {
			res = -ENOENT;
			return;
		}

		memcpy(buf, result->data.data, result->data.len);

		res = result->data.len;

		*(protobuf_c_boolean *) closure_data = 1;
	}

	rpc_layer__remote_ext2__read(service, &query, _handle_response, &is_done);

	while (!is_done)
		protobuf_c_dispatch_run(protobuf_c_dispatch_default());

	free(query.path);

	return res;
}

static struct fuse_operations ext2_operations = {
        .getattr = fuse_ext2_getattr,
        .readdir = fuse_ext2_readdir,
        .open = fuse_ext2_open,
        .read = fuse_ext2_read
};

int main(int argc, char **argv) {
	ProtobufC_RPC_Client *client;
	const char *name = "../socket";

	log = log_create(NULL, "Ext2 Client", true, LOG_LEVEL_DEBUG);

	log_info(log, "[+] Initialize RPC Client ... ");

	service = protobuf_c_rpc_client_new(PROTOBUF_C_RPC_ADDRESS_LOCAL, name, &rpc_layer__remote_ext2__descriptor, NULL);

	client = (ProtobufC_RPC_Client *) service;

	log_info(log, "[+] Connecting to Server ... ");
	while (!protobuf_c_rpc_client_is_connected(client)){
		protobuf_c_dispatch_run(protobuf_c_dispatch_default());
	}

	log_info(log, "[+] Running FUSE ... ");

	int ret;
	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

	ret = fuse_main(args.argc, args.argv, &ext2_operations);

	fuse_opt_free_args(&args);

	return ret;
}

