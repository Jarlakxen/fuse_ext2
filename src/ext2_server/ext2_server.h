/*
 * ext2_server.h
 *
 *  Created on: 05/03/2012
 *      Author: fviale
 */

#ifndef EXT2_SERVER_H_
#define EXT2_SERVER_H_

	#include "../includes/ext2.h"

	typedef struct{
		char* device_path;
		int device_desc;
		char* device;

		t_ext2 *fs;

	}t_ext2_server;

	t_ext2_server *ext2_server_create(char* device_path);
	void 		   ext2_server_run(t_ext2_server *);

#endif /* EXT2_SERVER_H_ */
