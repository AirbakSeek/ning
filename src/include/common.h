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

#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "ning.h"

#if LOG_LEVEL == 1
#define LOG_ERR(fmt, ...)  printf("[ERR][%s][%d]" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define LOG_DBG(fmt, ...)
#define LOG_INFO(fmt, ...)
#elif LOG_LEVEL == 2
#define LOG_ERR(fmt, ...)  printf("[ERR][%s][%d]" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define LOG_DBG(fmt, ...)  printf("[DBG][%s][%d]" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...)
#elif LOG_LEVEL == 3
#define LOG_ERR(fmt, ...)  printf("[ERR][%s][%d]" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define LOG_DBG(fmt, ...)  printf("[DBG][%s][%d]" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...)  printf("[INFO][%s][%d]" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif

#define DUMP_BIN(name, addr, len) dump_bin(__FUNCTION__, __LINE__, name, addr, len)
#define DUMP_HEX(name, addr, len) dump_hex(__FUNCTION__, __LINE__, name, addr, len)

#define PANIC(fmt, ...) \
    do { \
        printf("[PANIC][%s][%d]" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
        exit(0); \
    } while (0)

#define ASSERT(expr) \
    do { \
        if (!(expr)) { \
            printf("assertion '%s' failed at %s:%d (func '%s')\n", \
                #expr, __FILE__, __LINE__, __func__); \
            exit(0); \
        } \
    } while (0)

#define COMPILE_TIME_ASSERT(x) \
    do { \
        switch (0) { case 0: case ((x) ? 1: 0): default : break; } \
    } while (0)



void dump_bin(const char *func, const unsigned int line, const char *name,
              const void *addr, const unsigned int len);
void dump_hex(const char *func, const unsigned int line, const char *name,
              const void *addr, const unsigned int len);
int hex2str(char *buf, unsigned int size, char *out, unsigned int out_len);
int do_sha256(const char *buf, const unsigned int size, char *hash, const unsigned hash_len);
int gen_buf_name(char *buf, const unsigned int size, char *name, unsigned int name_len);
void gen_rand_buf(char *buf, const unsigned int size);
int check_buf_and_file(char *buf, unsigned int size, char *file_path);
int buf_to_file(char *buf, unsigned int size, char *data_name);
int file_to_buf(char *buf, unsigned int size, char *file_path);

#endif
