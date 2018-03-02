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


#include <ctype.h>

#include "common.h"
#include "tomcrypt.h"
#include "ning.h"

void dump_bin(const char *func, const unsigned int line, const char *name,
              const void *addr, const unsigned int len)
{
    int i,j;
    const unsigned char *p = addr;
    if (!func || !name || !addr) {
        LOG_ERR("Invliad input\n");
        return;
    }
    
    printf("[%s][%d] name[%s] addr[%p] len[0x%x]:", func, line, name, addr, len);
    for (i = 0; i < len; i++) {
        if (0 == i % 4)
            printf("\n0x%08x: ", *(unsigned int *)p);
        
        for (j = 7; j >= 0; j--) {
            printf("%d", (*p >> j) & 0x1);
        }
        printf(" ");
        p++;
    }
    printf("\n");
}

void dump_hex(const char *func, const unsigned int line, const char *name,
              const void *addr, const unsigned int len)
{
    const unsigned char *p = addr;
    unsigned long x, y = 0, z;
    
    if (!func || !name || !addr) {
        LOG_ERR("Invliad input\n");
        return;
    }
    printf("[%s][%d] name[%s] addr[%p] len[0x%x]:\n", func, line, name, addr, len);

    for (x = 0; x < len; ) {
        printf("%02X ", p[x]);
        if (!(++x % 16) || x == len) {
            if((x % 16) != 0) {
                z = 16 - (x % 16);
                if(z >= 8)
                    printf(" ");
                for (; z != 0; --z) {
                    printf("   ");
                }
            }
            printf(" | ");
            for(; y < x; y++) {
                if((y % 8) == 0)
                    printf(" ");
                if(isgraph(p[y]))
                    printf("%c", p[y]);
                else
                    printf(".");
            }
            printf("\n");
        }
        else if((x % 8) == 0) {
            printf(" ");
        }
    }
}

int hex2str(char *buf, unsigned int size, char *out, unsigned int out_len)
{
    int i;
    unsigned char *p = (unsigned char *)buf;
    unsigned char tmp;
    unsigned char *o = (unsigned char *)out;
    
    if (out_len < (size * 2)) {
        LOG_ERR("Invalid input size[0x%x] out_len[0x%x]\n", size, out_len);
        return -1;
    }

    for(i = 0; i < size; i++) {
        tmp = ((*p) >> 4) & 0xf;
        tmp = (tmp > 9) ? (tmp + 'A' - 0xa) : (tmp + '0');
        *o = tmp;
        o++;
        
        tmp = (*p) & 0xf;
        tmp = (tmp > 9) ? (tmp + 'A' - 0xa) : (tmp + '0');
        *o = tmp;
        o++;
        
        p++;
    }

    return 0;
}

void gen_rand_buf(char *buf, const unsigned int size)
{
    unsigned int *p = (unsigned int *)buf;
    int i;
    
    if (!buf) {
        LOG_ERR("Invalid input\n");
        return;
    }
    
    for (i = 0; i < size; i +=4) {
        *p = rand();
        p++;
    }
    
    //DUMP_HEX("buf", buf, 0x10);
}

int gen_sha_name(const char *buf, const unsigned int size, char *name, unsigned int name_len)
{
    int ret = -1;
    char hash[SHA256_LEN_BYTES];
    
    memset(hash, 0, SHA256_LEN_BYTES);
    ret = do_sha256(buf, size, hash, sizeof(hash));
    if (ret) {
        LOG_ERR("do_sha256 failed, ret[0x%x]\n", ret);
        goto exit;
    }
    
    ret = hex2str(hash, sizeof(hash), name, name_len);
    if (ret) {
        LOG_ERR("do_sha256 failed, ret[0x%x]\n", ret);
        goto exit;
    }
exit:
    return ret;
    
}

/** generate the buffer name by SHA256
 * name = SHA256(buffer) + "-" + Fisrt_bits(buffer)
 * sample:
 * FF15BEB86E604DDAB61E592143E8FEE6AFCDD5CB2CA6EDDA9B0297AD7FA262E1-32E20C50581D1113C13613813192625C
 */
int gen_buf_name(char *buf, const unsigned int size, char *name, unsigned int name_len)
{
    int ret = -1;
    char hash_name[SHA256_LEN_BYTES * 2 + 1];
    char first_bits_name[FILE_FIRST_BYTES * 2 + 1];

    if (name_len < DATA_FILE_LEN_BYTES * 2) {
        LOG_ERR("name len[%d] is not enought, should be [%d]\n", name_len, DATA_FILE_LEN_BYTES * 2);
    }
    
    memset(hash_name, 0, sizeof(hash_name));
    ret = gen_sha_name(buf, size, hash_name, sizeof(hash_name));
    if (ret) {
        LOG_ERR("gen_sha_name failed, ret[0x%x]\n", ret);
        goto exit;
    }

    memset(first_bits_name, 0, sizeof(first_bits_name));

    ret = hex2str(buf, FILE_FIRST_BYTES, first_bits_name , sizeof(first_bits_name));
    if (ret) {
        LOG_ERR("first bit to str failed, ret[0x%x]\n", ret);
        goto exit;
    }
    snprintf(name, name_len, "%s%s%s", hash_name, FILE_NAME_LINK_STR, first_bits_name);
    
exit:
    return ret;

}

int do_sha256(const char *buf, const unsigned int size, char *hash, const unsigned hash_len)
{
    int ret = -1;
    hash_state md;
    
    if (!buf || !hash || !hash_len) {
        LOG_ERR("Invalid input\n");
        goto exit;
    }
    
    if (hash_len < SHA256_LEN_BYTES) {
        LOG_ERR("out len[0x%x] is too small\n", hash_len);
        goto exit;
    }
    
    memset(&md, 0, sizeof(hash_state));
    
    ret = sha256_init(&md);
    if (ret) {
        LOG_ERR("sha256_init failed, ret[0x%x]", ret);
        goto exit;
    }
    
    ret = sha256_process(&md, (unsigned char *)buf, size);
    if (ret) {
        LOG_ERR("sha256_process failed, ret[0x%x]", ret);
        goto exit;
    }
    
    ret = sha256_done(&md, (unsigned char *)hash);
    if (ret) {
        LOG_ERR("sha256_done failed, ret[0x%x]", ret);
        goto exit;
    }
    
exit:
    return ret;
}

/* check the buf whether match the block file that already exist */
int check_buf_and_file(char *buf, unsigned int size, char *file_path)
{
    FILE *fp = NULL;
    char *file_buf = NULL;
    unsigned int cnt;
    int ret = -1;
    
    file_buf = (char *)malloc(BLOCK_SIZE);
    if (!file_buf) {
        LOG_ERR("malloc failed\n");
        goto exit;
    }
    
    fp = fopen(file_path, FILE_OPEN_FLAG_RO);
    if (!fp) {
        LOG_ERR("open file[%s] failed\n", file_path);
        goto exit;
    }
    
    cnt = fread(file_buf, 1, BLOCK_SIZE, fp);
    if (cnt != size) {
        LOG_ERR("write file failed, cnt[0x%x] size[0x%x]\n", cnt, BLOCK_SIZE);
        goto exit;
    }
    
    if (0 == memcmp(buf, file_buf, size)) {
        LOG_INFO("file and buf match\n");
        ret = 0;
        goto exit;
    } else {
        LOG_ERR("Congratulations on winning the lottery，\n\
                two buffer have the same sha256 result and same first 128bit\n");
        DUMP_HEX("buf", buf, size);
        DUMP_HEX("file_buf", file_buf, BLOCK_SIZE);
        free(file_buf);
        file_buf = NULL;
        fclose(fp);
        fp = NULL;
        PANIC("It is a bug, please save the log and send email to airbak@126.com, thanks\n");
    }
    
exit:
    if (file_buf)
        free(file_buf);
    if (fp)
        fclose(fp);
    
    return ret;
}

/* write buffer to file */
int buf_to_file(char *buf, unsigned int size, char *file_path)
{
    int ret = -1;
    FILE *fp = NULL;
    unsigned int cnt;
     
    fp = fopen(file_path, FILE_OPEN_FLAG_W);
    if (!fp) {
        LOG_ERR("open file[%s] failed\n", file_path);
        goto exit;
    }
    
    cnt = fwrite(buf, 1, size, fp);
    if (cnt != size) {
        LOG_ERR("write file failed, cnt[0x%x] size[0x%x]\n", cnt, size);
        goto exit;
    }
    
    ret = 0;
exit:
    if (fp)
        fclose(fp);
    
    return ret;
}

/* read buffer to file */
int file_to_buf(char *buf, unsigned int size, char *file_path)
{
    int ret = -1;
    FILE *fp = NULL;
    unsigned int cnt;
    
    fp = fopen(file_path, FILE_OPEN_FLAG_RO);
    if (!fp) {
        LOG_ERR("open file[%s] failed\n", file_path);
        goto exit;
    }
    
    cnt = fread(buf, 1, size, fp);
    if (cnt != size) {
        LOG_ERR("read file failed, cnt[0x%x] size[0x%x]\n", cnt, size);
        goto exit;
    }
    
    ret = 0;
exit:
    if (fp)
        fclose(fp);
    
    return ret;
}


