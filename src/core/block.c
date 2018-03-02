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

#include "block.h"
#include "common.h"
#include "ning.h"

/* block id = hash + first_bytes */
static int gen_block_id(BLOCK_ST *block)
{
    ASSERT(sizeof(block->id) >= sizeof(block->hash) + FILE_FIRST_BYTES);
    memcpy(block->id, block->hash, sizeof(block->hash));
    memcpy(block->id + sizeof(block->hash), block->data, FILE_FIRST_BYTES);
    return 0;
}

/**
 * id =  hash + FILE_FIRST_BYTES
 * name = DATA_PATH + hash + FILE_NAME_LINK_STR + FILE_FIRST_BYTES
 *
 *
 */
static int gen_block_name(BLOCK_ST *block)
{
    int ret = 0;
    char *src;
    char *dst;
    unsigned int cnt;
    char *cur;
    unsigned int len;

    memcpy(block->name, DATA_PATH, strlen(DATA_PATH));
    cur = block->name + strlen(DATA_PATH);
    len = sizeof(block->name) - strlen(DATA_PATH);

    ret = hex2str(block->id, sizeof(block->id), cur, len);
    if (ret) {
        LOG_ERR("hex2str fialed, ret[0x%x]\n", ret);
        goto exit;
    }

    src = cur + sizeof(block->hash) * 2;
    dst = src + LINK_STR_BYTES;

    memcpy(dst, src, FILE_FIRST_BYTES * 2);
    memcpy(src, FILE_NAME_LINK_STR, LINK_STR_BYTES);

    //DUMP_HEX("hash", block->hash, block->hash_len);
    //DUMP_HEX("first 128bit", block->data, FILE_FIRST_BYTES);
    LOG_INFO("block name[%s]\n", block->name);

exit:
    return ret;
}

/* prepare blcok id and name by data */
static int block_prepare(BLOCK_ST *block)
{
    int ret = -1;

    //DUMP_HEX("block data", block->data, sizeof(block->data));
    ret = do_sha256(block->data, sizeof(block->data), block->hash, sizeof(block->hash));
    if (ret) {
        LOG_ERR("do_sha256 failed, ret[0x%x]\n", ret);
        goto exit;
    }

    ret = gen_block_id(block);
    if (ret) {
        LOG_ERR("gen block id fialed, ret[0x%x]\n", ret);
        goto exit;
    }

    ret = gen_block_name(block);
    if (ret) {
        LOG_ERR("gen block name fialed, ret[0x%x]\n", ret);
        goto exit;
    }

exit:
    return ret;
}

static int find_block_by_id(BLOCK_ST *block)
{
    int ret = -1;

    memcpy(block->hash, block->id, sizeof(block->hash));
    ret = gen_block_name(block);
    if (ret) {
        LOG_ERR("gen block name fialed, ret[0x%x]\n", ret);
        goto exit;
    }

    ret = file_to_buf(block->data, sizeof(block->data), block->name);
    if (ret) {
        LOG_ERR("file to fialed, ret[0x%x]\n", ret);
        goto exit;
    }

exit:
    return ret;
}


static int is_block_exist(BLOCK_ST *block)
{
    unsigned int is_exist = 0;

    if (access(block->name, 0)) {
        return 0;
    }

    if (check_buf_and_file(block->data, sizeof(block->data), block->name)) {
        /* never */
        return 0;
    } else {
        /* exist and match */
        return 1;
    }

}

/** Write data from block to file.
 * input: block data 
 * output: block id, hash, name
 */
int write_block(BLOCK_ST *block)
{
    int ret = -1;

    ret = block_prepare(block);
    if (ret) {
        LOG_ERR("block prepare failed\n");
        goto exit;
    }

    if (is_block_exist(block)) {
        ret = 0;
        goto exit;
    }

    ret = buf_to_file(block->data, sizeof(block->data), block->name);
    if (ret) {
        LOG_ERR("buf to file failed\n");
        goto exit;
    }

exit:
    return ret;
}

/* Read data from file to block by id */
int read_block(BLOCK_ST *block)
{
    int ret = -1;

    ret = find_block_by_id(block);
    if (ret) {
        LOG_ERR("buf to file failed\n");
        goto exit;
    }

exit:
    return ret;
}
