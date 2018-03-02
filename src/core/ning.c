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

#include <execinfo.h> /* backtrace */
#include <signal.h> /* sigaction */

#include "test.h"
#include "common.h"
#include "ning.h"
#include "block.h"
#include "cmd.h"
#include "create.h"
#include "extract.h"

CMD_CFG_ST g_cmd_cfg;

static void dump_calltrace(int sig, siginfo_t * info, void *secret)
{
    void *array[16];
    size_t size;

    char **strings;
    size_t i;
    size = backtrace(array, 16);
    strings = backtrace_symbols(array, size);
    printf("Obtained %zd stack frames.\n", size);
    for (i = 0; i < size; i++)
        printf("%s\n", strings[i]);
    free(strings);

    exit(0);
}

static void show_usage(void)
{
    printf("ning version %s\n", NING_VERSION);
    printf("usage:\n");
    printf("First option must be a mode specifier:\n");
    printf("\t-c Create -x Extract\n");
    printf("Common Options:\n");
    printf("\t-f <filename>  Location of archive\n");
    printf("Create: ning -c\n");
    printf("Extract: ning -x\n");
    printf("Test: ning -t\n");
    printf("\n");

}

static int cmd_test(void)
{
    int ret = 0;
    unsigned int cmd;
    
    for (cmd = TEST_CMD_MIN; cmd <= TEST_CMD_MAX; cmd++) {
        ret += do_test(cmd);
    }
    return ret;
}



int main(int argc, char *argv[])
{
    int ret = 0;
    struct sigaction sa;
    CMD_CFG_ST *cfg = &g_cmd_cfg;
    

    LOG_INFO("argc[%d] argv[0] = %s\n", argc, argv[0]);
    LOG_INFO("DATA_PATH[%s]\n", DATA_PATH);

    sa.sa_sigaction = dump_calltrace;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_SIGINFO;
    sigaction(SIGSEGV, &sa, NULL);
    sigaction(SIGABRT, &sa, NULL);

    memset(cfg, 0, sizeof(CMD_CFG_ST));
    ret = cmd_parse(cfg, argc, argv);
    if (ret) {
        return ret;
    }
    
    ret = check_cmd(cfg);
    if (ret) {
        return ret;
    }
    
    if (cfg->is_create) {
        LOG_DBG("create\n");
        ret = do_create(cfg);
    } else if (cfg->is_extract) {
        LOG_DBG("extract\n");
        ret = do_extract(cfg);
    } else if (cfg->is_test) {
        LOG_DBG("test\n");
        ret = cmd_test();     
        ret += do_test(TEST_CMD_MKDIR);
        ret += do_test(TEST_CMD_CREATE_DATA);
    } else {
        show_usage();
    }

    return ret;
}
