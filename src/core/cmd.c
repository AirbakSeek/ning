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

#include "cmd.h"

static int option_parse(CMD_CFG_ST *cfg, char *option)
{
    unsigned char *p = (unsigned char *)option;
    unsigned int loop = strlen(option) - 1;
    
    p++;
    while(loop--) {
        switch (*p) {
            case 'c':
                cfg->is_create = 1;
                break;
            case 'x':
                cfg->is_extract = 1;
                break;
            case 'f':
                cfg->has_file = 1;
                break;
            case 't':
                cfg->is_test = 1;
                break;
            default:
                LOG_ERR("not  nsupport option[-%c]\n", *p);
                return -1;
        }
        p++;
    }
    
    return 0;
}

int check_cmd(CMD_CFG_ST *cfg)
{
    if (cfg->is_create || cfg->is_extract) {
        if (!cfg->has_file) {
            LOG_ERR("please set option[-f]\n");
            return -1;
        }
        if (!cfg->file_path) {
            LOG_ERR("please set file path\n");
            return -1;
        }
    }
    
    return 0;
}

int cmd_parse(CMD_CFG_ST *cfg, int argc, char *argv[])
{
    int i;
    int ret = 0;
    for (i = 1; i < argc; i++) {
        LOG_DBG("argv[%d] = %s\n", i, argv[i]);
        if ('-' == argv[i][0]) {
            ret = option_parse(cfg, argv[i]);
            continue;
        }
        
        if (cfg->has_file) {
            cfg->file_path = argv[i];
        }
        
        if (ret) {
            LOG_ERR("not support [%s]\t", argv[i]);
            return -1;
        }
    }
    
    return ret;
}

