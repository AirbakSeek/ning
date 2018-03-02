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

#include "extract.h"
#include "block.h"
#include "pack.h"
#include "archive.h"

int block_to_pack(PACK_ST *pack, BLOCK_ST *block)
{
    int ret = 0;
    memcpy(pack, block->data, sizeof(PACK_ST));
    return ret;
}

int gen_output_file_name(CMD_CFG_ST *cfg, char *file_name, unsigned int len)
{
    ASSERT(strlen(cfg->file_path) > strlen(TARGET_FILE_SUBFIX));

    memset(file_name, 0, len);

    memcpy(file_name, cfg->file_path, strlen(cfg->file_path) - strlen(TARGET_FILE_SUBFIX));

    /* TODO: file name chould be set by user */
    if (!access(file_name, 0)) {
        memcpy(file_name, cfg->file_path, strlen(cfg->file_path) - 1);
    }

    if (!access(file_name, 0)) {
        LOG_ERR("output file[%s] is exist\n", file_name);
        return -1;
    }
    return 0;
}

int do_extract(CMD_CFG_ST *cfg)
{
    int ret = 0;
    unsigned int cnt;
    FILE *fp_in = NULL;
    FILE *fp_out = NULL;
    ARCHIVE_ST *archive = NULL;
    char *output_file_name = NULL;

    if (access(cfg->file_path, 0)) {
        LOG_ERR("file[%s] is not exist\n", cfg->file_path);
        return -1;
    }

    fp_in = fopen(cfg->file_path, FILE_OPEN_FLAG_RO);
    if (!fp_in) {
        LOG_ERR("open file[%s] failed\n", cfg->file_path);
        goto exit;
    }

    output_file_name = (char *)malloc(FILE_PATH_MAX);
    if (!output_file_name) {
        LOG_ERR("malloc failed\n");
        goto exit;
    }

    ret = gen_output_file_name(cfg, output_file_name, FILE_PATH_MAX);
    if (ret) {
        LOG_ERR("gen target file failed\n");
        goto exit;
    }

    fp_out = fopen(output_file_name, FILE_OPEN_FLAG_W);
    if (!fp_out) {
        LOG_ERR("open file failed\n");
        goto exit;
    }

    archive = (ARCHIVE_ST *)malloc(sizeof(ARCHIVE_ST));
    if (!archive) {
        LOG_ERR("malloc archive failed\n");
        goto exit;
    }

    cnt = fread(archive, 1, sizeof(ARCHIVE_ST), fp_in);
        LOG_DBG("read file cnt[0x%x]\n", cnt);

    if (!cnt) {
            LOG_ERR("read file failed\n");
            goto exit;
    }

    /* read file data to generate pack list */
    ret = file_to_pack(fp_in);
    if (ret) {
        LOG_ERR("file to pack failed\n");
        goto exit;
    }

    /* extract pack to block */
    ret = pack_extract(fp_out, archive->size);
    if (ret) {
        LOG_ERR("extract pack to block failed\n");
        goto exit;
    }

    LOG_DBG("Extract to [%s] suc\n", output_file_name);

exit:

    if (fp_in)
        fclose(fp_in);

    if (fp_out)
        fclose(fp_out);

    if (output_file_name)
        free(output_file_name);

    if (archive)
        free(archive);


    return ret;
}
