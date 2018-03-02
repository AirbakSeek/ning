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

#include <time.h>
#include <sys/stat.h> /* mkdir */
#include <unistd.h> /* access */

#include "common.h"
#include "tomcrypt.h"
#include "test.h"
#include "ning.h"

static int test_sha(void)
{
    int ret = 0;
    char *buf = NULL;
    hash_state md;
    char hash[32];
    char *test1 = "abc";
    /* result: ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad */
    int result1[] = {0xbf1678ba, 0xeacf018f, 0xde404141, 0x2322ae5d, 0xa36103b0, 0x9c7a1796, 0x61ff10b4, 0xad1500f2};
    
    char *test2 = "xyz";
    /* result: 3608bca1e44ea6c4d268eb6db02260269892c0b42b86bbf1e77a6fa16c3c9282 */
    int result2[] = {0xa1bc0836, 0xc4a64ee4, 0x6deb68d2, 0x266022b0, 0xb4c09298, 0xf1bb862b, 0xa16f7ae7, 0x82923c6c};
    
    LOG_INFO("\n");
    buf = (char *)malloc(BLOCK_SIZE);
    if (!buf) {
        LOG_ERR("malloc failed\n");
        return -1;
    }

    memset((void *)buf, (int)rand(), BLOCK_SIZE);
    DUMP_BIN("buf", buf, 0x10);
    
    memset(&md, 0, sizeof(md));
    memset(hash, 0, sizeof(hash));
    
    sha256_init(&md);
    sha256_process(&md, (unsigned char *)test1, strlen(test1));
    sha256_done(&md, (unsigned char *)hash);
    
    if(memcmp(result1, hash, sizeof(hash))) {
        LOG_ERR("sha256 compare failed\n");
        DUMP_HEX("result1", result1, sizeof(hash));
        DUMP_HEX("sha256", hash, sizeof(hash));
        ret = -1;
    }
    
    memset(&md, 0, sizeof(md));
    memset(hash, 0, sizeof(hash));
    sha256_init(&md);
    sha256_process(&md, (unsigned char *)test2, strlen(test2));
    sha256_done(&md, (unsigned char *)hash);
    
    if(memcmp(result2, hash, sizeof(hash))) {
        LOG_ERR("sha256 compare failed\n");
        DUMP_HEX("result2", result2, sizeof(hash));
        DUMP_HEX("sha256", hash, sizeof(hash));
        DUMP_BIN("sha256", hash, sizeof(hash));
        ret = -1;
    }
    
    memset(&md, 0, sizeof(md));
    memset(hash, 0, sizeof(hash));
    sha256_init(&md);
    sha256_process(&md, (unsigned char *)buf, BLOCK_SIZE);
    sha256_done(&md, (unsigned char *)hash);
    
    DUMP_BIN("sha256", hash, sizeof(hash));
    DUMP_HEX("sha256", hash, sizeof(hash));
    return ret;
}

static int test_dump(void)
{
    unsigned int tmp[] = {
        0,
        0xffffffff,
        0xa5a5a5a5,
        0x5a5a5a5a,
        0x12345678,
        0xa5ff005a};

    DUMP_BIN("tmp", &tmp, sizeof(tmp));
    return 0;
}

static int test_hex2str(void)
{
    //unsigned int test = 0x12Ab0F;
    char test[] ={0x12, 0xab, 0x0f};
    char out[8 + 1];
    
    memset(out, 0, sizeof(out));
    hex2str((char *)&test, sizeof(test), out, sizeof(out));
    DUMP_HEX("test", &test, sizeof(test));
    DUMP_HEX("out", out, sizeof(out));
    out[8] = '\0';
    LOG_DBG("out[%s]\n", out);
    return 0;
}

static int test_file(void)
{
    int ret = -1;
    FILE *fp = NULL;
    char *buf = NULL;
    char *out = NULL;
    unsigned int cnt;
    int is_exist = 0;
    
    buf = (char *)malloc(BLOCK_SIZE);
    if (!buf) {
        LOG_ERR("malloc failed\n");
        goto exit;
    }
    srand((unsigned int)time(NULL));
    memset((void *)buf, (int)rand(), BLOCK_SIZE);
    
    out = (char *)malloc(BLOCK_SIZE);
    if (!out) {
        LOG_ERR("malloc failed\n");
        goto exit;
    }
    memset(out, 0, BLOCK_SIZE);
    
    is_exist = !(access(TEST_FILE_NAME, 0));
    
    fp = fopen(TEST_FILE_NAME, FILE_OPEN_FLAG_R);
    if (!fp) {
        LOG_ERR("open file failed\n");
        goto exit;
    }
    
    if (is_exist) {
        cnt = fread(out, 1, BLOCK_SIZE, fp);
        LOG_DBG("read file cnt[0x%x]\n", cnt);
        DUMP_HEX("out", out, 0x10);
        rewind(fp);
        memset(out, 0, BLOCK_SIZE);
    }
    
    cnt = fwrite(buf, 1, BLOCK_SIZE, fp);
    
    if (cnt != BLOCK_SIZE) {
        LOG_ERR("write file failed, cnt[0x%x] size[0x%x]\n", cnt, BLOCK_SIZE);
        goto exit;
    }
    
    rewind(fp);
    cnt = fread(out, 1, BLOCK_SIZE, fp);
    LOG_DBG("read file cnt[0x%x]\n", cnt);
    
    DUMP_HEX("buf", buf, 0x10);
    DUMP_HEX("out", out, 0x10);
    
    ret = 0;
exit:
    if (buf)
        free(buf);
    if (out)
        free(out);
    if (fp)
        fclose(fp);
    return ret;
}

static int test_create_data(void)
{
    int ret = -1;
    FILE *fp = NULL;
    char *buf = NULL;
    char name[(DATA_FILE_LEN_BYTES * 2 ) + 1];
    char *file_path = NULL;
    unsigned int cnt;

    int is_exist = 0;
    int i;
    int loop = 4;
    
    buf = (char *)malloc(BLOCK_SIZE);
    if (!buf) {
        LOG_ERR("malloc failed\n");
        goto exit;
    }
    
    file_path = (char *)malloc(FILE_PATH_MAX);
    if (!file_path) {
        LOG_ERR("malloc failed\n");
        goto exit;
    }
    
    LOG_DBG("loop[%d]\n", loop);

    while(loop--) {
        memset(buf, 0, BLOCK_SIZE);
        memset(file_path, 0, FILE_PATH_MAX);
        memset(name, 0, sizeof(name));

        gen_rand_buf(buf, BLOCK_SIZE);
        ret = gen_buf_name(buf, BLOCK_SIZE, name, sizeof(name));
        if (ret) {
            LOG_ERR("Invalid input\n");
            break;
        }
        snprintf(file_path, FILE_PATH_MAX, "%s%s", DATA_PATH, name);
        //LOG_DBG("file_path[%s]\n", file_path);

        is_exist = !(access(file_path, 0));
        
        if (is_exist) {
            LOG_ERR("file_path[%s] is exist\n", file_path);
            break;
        }

        fp = fopen(file_path, FILE_OPEN_FLAG_W);
        if (!fp) {
            LOG_ERR("open file[%s] failed\n", file_path);
            break;
        }
        
        cnt = fwrite(buf, 1, BLOCK_SIZE, fp);
        fclose(fp);

        if (cnt != BLOCK_SIZE) {
            LOG_ERR("write file failed, cnt[0x%x] size[0x%x]\n", cnt, BLOCK_SIZE);
            break;
        }

    }

exit:
    if (buf)
        free(buf);
    if (file_path)
        free(file_path);
    
    return ret;
}

static int test_mkdir(void)
{
    int ret = 0;
    int is_exist = 0;

    is_exist = !(access(DATA_PATH, 0));
    if (is_exist) {
        LOG_DBG("DATA_PATH[%s] is exist\n", DATA_PATH);
        return 0;
    }
    
    LOG_DBG("is_exist[0x%x] DATA_PATH[%s]\n", is_exist, DATA_PATH);
    ret = mkdir(DATA_PATH, 0755);
    if (ret) {
        LOG_ERR("mkdir failed\n");
        goto exit;
    }

exit:
    return ret;

}

int do_test(const unsigned int cmd)
{
    int ret = -1;
    
    srand((unsigned int)time(NULL));
    switch (cmd) {
        case TEST_CMD_SHA:
            ret = test_sha();
            break;
        case TEST_CMD_HEX2STR:
            ret = test_hex2str();
            break;
        case TEST_CMD_DUMP:
            ret = test_dump();
            break;
        case TEST_CMD_FILE:
            ret = test_file();
            break;
        case TEST_CMD_CREATE_DATA:
            ret = test_mkdir();
            ret = test_create_data();
            break;
        case TEST_CMD_MKDIR:
            ret = test_mkdir();
            break;
        default:
            LOG_ERR("not support cmd[0x%x]", cmd);
            break;
    }
    return ret;
}
