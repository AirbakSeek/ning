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

#ifndef NING_H
#define NING_H

#define NING_VERSION  ("1.0.0")

#define BLOCK_SIZE  (1024)

#define FILE_OPEN_FLAG_RO "rb"
#define FILE_OPEN_FLAG_R "rb+"
#define FILE_OPEN_FLAG_W "wb+"

#define DATA_PATH  (APP_PATH"data/")

#define SHA256_LEN_BITS      (256)
#define SHA256_LEN_BYTES      (SHA256_LEN_BITS/8)
#define FILE_FIRST_BITS      (128)
#define FILE_FIRST_BYTES      (FILE_FIRST_BITS/8)

#define LEVEL1_SIZE_BYTES    (SHA256_LEN_BYTES)
#define LEVEL2_SIZE_BYTES    (FILE_FIRST_BYTES)

#define FILE_NAME_LINK_STR  "-"
#define LINK_STR_BYTES      (1)

#define DATA_ID_LEN_BYTES  (LEVEL1_SIZE_BYTES  + LEVEL2_SIZE_BYTES)
#define DATA_FILE_LEN_BYTES  (DATA_ID_LEN_BYTES + LINK_STR_BYTES)

#define FILE_PATH_MAX  1024
#define FILE_NAME_MAX  256

#define TARGET_FILE_SUBFIX    ".ning"

typedef struct {
    unsigned int is_create;
    unsigned int is_extract;
    unsigned int has_file;
    unsigned int is_test;
    char *file_path;
} CMD_CFG_ST;

#endif
