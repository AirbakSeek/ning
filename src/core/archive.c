/*
 * Copyright (c) 2017, Jianhui Li, airbak@126.com
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/stat.h>
#include "archive.h"

int get_file_name(char *file_path, char *file_name)
{
	int ret = 0;

	/* TODO: remove floder in file path */
	memcpy(file_name, file_path, strlen(file_path));
	return ret;
}

int archive_init(ARCHIVE_ST *archive, char *file_path)
{
	int ret;
	struct stat *st;

	st = (struct stat *)malloc(sizeof(struct stat));
	if (!st) {
		LOG_ERR("malloc stat failed\n");
		goto exit;
	}

	/* TODO: handle link file */
	ret = stat(file_path, st);
	if (ret) {
		LOG_ERR("stat file[%s] failed, ret[0x%x]\n", file_path, ret);
		goto exit;
	}

	LOG_DBG("file[%s]\n"
		"size[0x%llx] mode[0x%x] blksize[0x%x] blocks[0x%llx] ctime[0x%lx]\n", 
		file_path,
		st->st_size, st->st_mode, st->st_blksize, st->st_blocks, st->st_ctime);


	ret = get_file_name(file_path, archive->name);
	if (ret) {
		LOG_ERR("get file name failed, ret[0x%x]\n", ret);
		goto exit;
	}

	archive->size = st->st_size;
	archive->mode = st->st_mode;



exit:
	if (st)
		free(st);
	return ret;
}