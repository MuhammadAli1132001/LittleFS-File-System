/*
 * lfs_port.c
 *
 *  Created on: Apr 28, 2025
 *      Author: muhammadali
 */


#include "lfs.h"
#include "stm32g0xx_hal.h"
#include <string.h>

#define LFS_BASE_ADDR  0x0801C000UL
#define FLASH_PAGE_SIZE  2048U

extern lfs_t lfs;
extern struct lfs_config cfg;

int user_lfs_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size) {
    uint32_t address = LFS_BASE_ADDR + block * c->block_size + off;
    memcpy(buffer, (const void *)address, size);
    return 0;
}

int user_lfs_prog(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size) {
    HAL_FLASH_Unlock();
    uint32_t address = LFS_BASE_ADDR + block * c->block_size + off;
    for (uint32_t i = 0; i < size; i += 8) {
        uint64_t data64 = *(uint64_t *)((uint8_t *)buffer + i);
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, address + i, data64) != HAL_OK) {
            HAL_FLASH_Lock();
            return -1;
        }
    }
    HAL_FLASH_Lock();
    return 0;
}

int user_lfs_erase(const struct lfs_config *c, lfs_block_t block) {
    HAL_FLASH_Unlock();
    FLASH_EraseInitTypeDef erase;
    uint32_t page_error = 0;
    erase.TypeErase = FLASH_TYPEERASE_PAGES;
    erase.Page = (LFS_BASE_ADDR + block * c->block_size - FLASH_BASE) / FLASH_PAGE_SIZE;
    erase.NbPages = 1;
    if (HAL_FLASHEx_Erase(&erase, &page_error) != HAL_OK) {
        HAL_FLASH_Lock();
        return -1;
    }
    HAL_FLASH_Lock();
    return 0;
}

int user_lfs_sync(const struct lfs_config *c) {
    return 0;
}

struct lfs_config cfg = {
    .read = user_lfs_read,
    .prog = user_lfs_prog,
    .erase = user_lfs_erase,
    .sync = user_lfs_sync,
    .read_size = 16,
    .prog_size = 16,
    .block_size = FLASH_PAGE_SIZE,
    .block_count = (16*1024) / FLASH_PAGE_SIZE,
    .cache_size = 16,
    .lookahead_size = 16,
    .block_cycles = 500,
};
