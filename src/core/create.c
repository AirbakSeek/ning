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
#include "create.h"
#include "block.h"
#include "pack.h"
#include "archive.h"

int compress_block(BLOCK_ST *block)
{
    int ret = 0;

    ret = pack_add_block(0, block);
    if (ret) {
        LOG_ERR("add block to pack failed(0x%x)\n", ret);
    }

    return ret;
}

static int create_data_dir(void)
{
    int ret;
    if (!(access(DATA_PATH, 0))) {
        LOG_DBG("DATA_PATH[%s] is exist\n", DATA_PATH);
        return 0;
    }

    LOG_DBG("mkdir DATA_PATH[%s]\n", DATA_PATH);
    ret = mkdir(DATA_PATH, 0755);
    if (ret) {
        LOG_ERR("mkdir [%s] failed\n, ret[0x%x]", DATA_PATH, ret);
        return ret;
    }

    return 0;

}

/* TODO: file name could be input */
int gen_tar_file_name(CMD_CFG_ST *cfg, char *tar_name, unsigned int len)
{
    ASSERT((strlen(cfg->file_path) + strlen(TARGET_FILE_SUBFIX)) < len);
    snprintf(tar_name, FILE_PATH_MAX, "%s%s", cfg->file_path, TARGET_FILE_SUBFIX);

    if (!access(tar_name, 0)) {
        LOG_ERR("target file[%s] is exist\n", tar_name);
        return -1;
    }
    return 0;
}

int output_file(ARCHIVE_ST *archive, char *tar_name)
{
    int ret = 0;
    FILE *fp = NULL;
    unsigned int cnt;

    fp = fopen(tar_name, FILE_OPEN_FLAG_W);
    if (!fp) {
        LOG_ERR("open file failed\n");
        goto exit;
    }
    cnt = fwrite(archive, 1, sizeof(ARCHIVE_ST), fp);
    if (cnt != sizeof(ARCHIVE_ST)) {
            LOG_ERR("write file failed, cnt[0x%x] size[0x%lx]\n", cnt, sizeof(ARCHIVE_ST));
            ret = -1;
            goto exit;
    }

    ret = pack_to_file(fp);

exit:
    if (fp)
        fclose(fp);

    return ret;
}

int do_create(CMD_CFG_ST *cfg)
{
    int ret = -1;
    FILE *fp_in = NULL;
    BLOCK_ST *block = NULL;
    PACK_ST *pack = NULL;
    ARCHIVE_ST *archive = NULL;
    unsigned int cnt;
    char *tar_name = NULL;

    if (access(cfg->file_path, 0)) {
        LOG_ERR("file[%s] is not exist\n", cfg->file_path);
        return -1;
    }

    ret = create_data_dir();
    if (ret) {
        LOG_ERR("create data dir failed\n");
        return -1;
    }

    tar_name = (char *)malloc(FILE_PATH_MAX);
    if (!tar_name) {
        LOG_ERR("malloc failed\n");
        goto exit;
    }

    ret = gen_tar_file_name(cfg, tar_name, FILE_PATH_MAX);
    if (ret) {
        LOG_ERR("gen target file failed\n");
        goto exit;
    }

    block = (BLOCK_ST *)malloc(sizeof(BLOCK_ST));
    if (!block) {
        LOG_ERR("malloc block failed\n");
        goto exit;
    }

    pack = (PACK_ST *)malloc(sizeof(PACK_ST));
    if (!pack) {
        LOG_ERR("malloc pack failed\n");
        goto exit;
    }

    archive = (ARCHIVE_ST *)malloc(sizeof(ARCHIVE_ST));
    if (!archive) {
        LOG_ERR("malloc archive failed\n");
        goto exit;
    }

    ret = archive_init(archive, cfg->file_path);
    if (ret) {
        LOG_ERR("archive init failed\n");
        goto exit;
    }

    fp_in = fopen(cfg->file_path, FILE_OPEN_FLAG_RO);
    if (!fp_in) {
        LOG_ERR("open file[%s] failed\n", cfg->file_path);
        goto exit;
    }

    while(!feof(fp_in)) {
        cnt = fread(block->data, 1, sizeof(block->data), fp_in);
        LOG_INFO("read file cnt[0x%x]\n", cnt);
        ret = write_block(block);
        if (ret) {
            LOG_ERR("write block failed, ret[0x%x]\n", ret);
            break;
        }


        ret = compress_block(block);

        memset(block, 0, sizeof(BLOCK_ST));

    }

    ret = output_file(archive, tar_name);
    if (ret) {
        LOG_ERR("create file failed, ret[0x%x]\n", ret);
        goto exit;
    }

    LOG_DBG("Create [%s] to [%s] suc\n", cfg->file_path, tar_name);

exit:
    if (tar_name)
        free(tar_name);

    if (fp_in)
        fclose(fp_in);

    if (block)
        free(block);

    if (pack)
        free(pack);

    if (archive)
        free(archive);

    return ret;
}
