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

#include "pack.h"
#include <math.h>

LIST_HEAD(pack_list_head);


static int pack_init(PACK_ST *pack)
{
    pack->level = 0;
    return 0;
}

/* find pack by level */
static PACK_ST *find_pack(struct list_head *head, unsigned int level)
{
	PACK_ST *p = NULL;

	list_for_each_entry(p, head, list) {
		if (level == p->level) {
			return p;
		}
	}

	LOG_INFO("not found level[%d]\n", level);
	return NULL;
}

static void add_pack(struct list_head *head, PACK_ST *pack)
{
	LOG_DBG("add pack level[%d]\n", pack->level);
	list_add_tail(&pack->list, head);
}

/* get the special level pack */
static PACK_ST *get_pack(unsigned int level)
{
    PACK_ST *pack = NULL;

    pack = find_pack(&pack_list_head, level);

    if (pack)
        return pack;

    pack = (PACK_ST *)malloc(sizeof(PACK_ST));
    if (!pack) {
        LOG_ERR("malloc block failed\n");
        goto exit;
    }

    memset(pack, 0, sizeof(PACK_ST));
    pack->pos = pack->ids;
    pack->level = level;
    pack->num = 0;

    add_pack(&pack_list_head, pack);

exit:
    return pack;
}

void pack_dump(struct list_head *head)
{
	PACK_ST *p = NULL;
    unsigned int bk_per_level = BLOCK_SIZE/DATA_ID_LEN_BYTES;
    unsigned int block_total = 0;

	list_for_each_entry(p, head, list) {
        block_total += p->num * pow(bk_per_level, p->level);
		LOG_DBG("pack level[%d] num[%d] ids[%p] pos[%p]\n", p->level, p->num, p->ids, p->pos);
	}

    LOG_DBG("block per level[%d] = (block_size[%d]) / (ID_len[%d]), total block[%d]\n", 
        bk_per_level, BLOCK_SIZE, DATA_ID_LEN_BYTES, block_total);
}

/**
 * Add block to pack. When pack is full, add to next level.
 * level1 
 * level2
 * ...
 * leveln
 */
int pack_add_block(unsigned int level, BLOCK_ST *block)
{
    PACK_ST *pack;
    BLOCK_ST *blk = NULL;
    int ret = 0;

    pack = get_pack(level);

    ASSERT((pack->ids + sizeof(pack->ids)) >= (pack->pos + sizeof(block->id)));

    memcpy(pack->pos, block->id, sizeof(block->id));
    pack->pos += sizeof(block->id);
    pack->num++;

    if ((pack->ids + sizeof(pack->ids)) > (pack->pos + sizeof(block->id))) {
        return 0;
    }

    LOG_INFO("num[%d] level[%d] pack is full, write to next level[%d]\n", pack->num, level, level + 1);

    blk = (BLOCK_ST *)malloc(sizeof(BLOCK_ST));
    if (!blk) {
        LOG_ERR("malloc block failed\n");
        goto exit;
    }

    COMPILE_TIME_ASSERT(sizeof(block->data) == sizeof(pack->ids));

    memset(blk, 0, sizeof(BLOCK_ST));
    memcpy(blk->data, pack->ids, sizeof(pack->ids));

    ret = write_block(blk);
    if (ret) {
        LOG_ERR("write block failed\n");
        goto exit;
    }

    /* add to next level */
    ret = pack_add_block(level + 1, blk);
    if (ret) { 
        LOG_ERR("level[%d] pack add failed\n", level + 1);
        goto exit;
    }

    /* reset current level pack */
    memset(pack->ids, 0, sizeof(pack->ids));
    pack->pos = pack->ids;
    pack->num = 0;

exit:
    if (blk)
        free(blk);

    return ret;
}

/* write pack to file */
int pack_to_file(FILE *fp)
{
	PACK_ST *p = NULL;
	int ret = 0;
	unsigned int cnt;
	struct list_head *head = &pack_list_head;

	pack_dump(head);

	list_for_each_entry(p, head, list) {
		cnt = fwrite(p, 1, sizeof(PACK_ST), fp);
	    if (cnt != sizeof(PACK_ST)) {
	        LOG_ERR("write file failed, cnt[0x%x] size[0x%lx]\n", cnt, sizeof(PACK_ST));
	        ret = -1;
	        goto exit;
	    }

	}

exit:
	return ret;	
}

/* Read file to generate pack list */
int file_to_pack(FILE *fp)
{
    struct list_head *head = &pack_list_head;
    PACK_ST *pack;
    PACK_ST *tmp;
    unsigned int cnt;

    while(!feof(fp)) {
        pack = NULL;
        pack = (PACK_ST *)malloc(sizeof(PACK_ST));
        if (!pack) {
            LOG_ERR("malloc block failed\n");
            goto exit;
        }

        cnt = fread(pack, 1, sizeof(PACK_ST), fp);

        if (!cnt) {
            if (feof(fp)) {
                break;
            }
            LOG_ERR("read file failed\n");
            goto exit;
        }
        pack->pos = pack->ids + pack->num * DATA_ID_LEN_BYTES;
        pack->list.next = NULL;
        pack->list.prev = NULL;
        add_pack(head, pack);

    }

    pack_dump(head);

    return 0;
exit:
    list_for_each_entry_safe(pack, tmp, head, list) {
        list_del(&pack->list);
        free(pack);
    }
    return -1;
}

/* loop pack to block data, and write it to file */
static int pack_to_block(PACK_ST *pack, FILE *fp)
{
    int ret = 0;
    int i;
    BLOCK_ST *bk = NULL;
    unsigned int cnt;
    long offset = ftell(fp);

    bk = (BLOCK_ST *)malloc(sizeof(BLOCK_ST));
    if (!bk) {
        LOG_ERR("malloc block failed\n");
        goto exit;
    }

    for (i = 0; i < pack->num; i++) {
        pack->pos -= sizeof(bk->id);
        memcpy(bk->id, pack->pos, sizeof(bk->id));
        //DUMP_HEX("block id", bk->id, sizeof(bk->id));
        ret = read_block(bk);
        if (ret) {
            LOG_ERR("read block failed, ret[0x%x]\n", ret);
            break;
        }
        
        LOG_INFO("offset[0x%lx]\n", offset);

        /* Write from back to front */
        cnt = fwrite(bk->data, 1, sizeof(bk->data), fp);
        if (cnt != sizeof(bk->data)) {
            LOG_ERR("write file failed, cnt[0x%x] size[0x%lx]\n", cnt, sizeof(bk->data));
            ret = -1;
            break;
        }

        offset -= BLOCK_SIZE;
        if (offset < 0) {
            break;
        }
        fseek(fp, offset, SEEK_SET);
    }

    /* reset pack ids number */
    pack->num = 0;

 exit:


    if (bk)
        free(bk);

    return ret;
}


int pack_pop_block_id(PACK_ST *pack, BLOCK_ST *block)
{
    if (pack->pos == pack->ids) {
        LOG_INFO("pack level[%d] ids is empty\n", pack->level);
        return -1;
    }
    LOG_INFO("level[%d] pos[%p] ids[%p] size[0x%zx]\n", pack->level, pack->pos, pack->ids, sizeof(block->id));
    ASSERT(pack->pos >= pack->ids + sizeof(block->id));

    pack->pos -= sizeof(block->id);
    memcpy(block->id, pack->pos, sizeof(block->id));
    return 0;
}

/* when current level is empty, get ids from next level */
int pack_list_update(struct list_head *head, PACK_ST *pack)
{
    PACK_ST *pack_next = NULL;
    int ret;
    BLOCK_ST *bk = NULL;

    /* last node */
    if (head == pack->list.next) {
        list_del(&pack->list);
        LOG_INFO("level[%d] last node, list is empty\n", pack->level);
        return 0;
    }

    pack_next = list_entry(pack->list.next, PACK_ST, list);

    bk = (BLOCK_ST *)malloc(sizeof(BLOCK_ST));
    if (!bk) {
        LOG_ERR("malloc block failed\n");
        return -1;
    }

    ret = pack_pop_block_id(pack_next, bk);
    if (ret) {
        ret = pack_list_update(head, pack_next);
        if (ret) {
            LOG_ERR("pack list update failed, ret[0x%x]\n", ret);
            return -1;
        }
        ret = pack_pop_block_id(pack_next, bk);
        if (ret) {
            LOG_INFO("no block id to pop\n");
            return 0;
        }
    }

    ret = read_block(bk);
    if (ret) {
        LOG_ERR("read block failed, ret[0x%x]\n", ret);
        return -1;
    }

    /* update pack ids */
    memcpy(pack->ids, bk->data, sizeof(pack->ids));
    if (bk) {
        free(bk);
        bk = NULL;
    }

    pack->num  = sizeof(pack->ids) / sizeof(bk->id);
    pack->pos = pack->ids + pack->num * sizeof(bk->id);
    return 0;
}

int pack_extract(FILE *fp, size_t file_size)
{
    struct list_head *head = &pack_list_head;
    PACK_ST *pack = NULL;
    long offset = file_size - file_size % BLOCK_SIZE;
    int ret = 0;

    fseek(fp, offset, SEEK_SET);
    while(!list_empty(head)) {
        pack = list_first_entry(head, PACK_ST, list);

        /* extract pack level 0 */
        ret = pack_to_block(pack, fp);
        if (ret) {
            LOG_ERR("pack to block failed, ret[0x%x]\n", ret);
            break;
        }

        /* fill pack level 0 */
        ret = pack_list_update(head, pack);
        if (ret) {
            LOG_ERR("pack list update failed, ret[0x%x]\n", ret);
            break;
        }

        if (0 == pack->num) {
            /* */
            LOG_DBG("all pack extract\n");
            break;
        }
    }

    offset = ftell(fp);
    ASSERT(BLOCK_SIZE == offset);
    rewind(fp);
    fflush(fp);
    ret = ftruncate(fileno(fp), (off_t)file_size);
    if (ret) {
        LOG_ERR("ftruncate file size[%zu] fail\n", file_size);
        return -1;
    }
    
    return 0;   
}