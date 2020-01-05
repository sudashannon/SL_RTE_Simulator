/**
 * @file RTE_Memory.h
 * @author Leon Shan (813475603@qq.com)
 * @brief
 * @version 0.1
 * @date 2019-10-26
 *
 * @copyright Copyright (c) 2019
 *
 */
#ifndef __RET_MEMORY_H
#define __RET_MEMORY_H

#include "RTE_Config.h"
#include "RTE_Log.h"

/* Some IMPORTANT MEM parameters */
/* Unlike the preview MEM versions, now they are statics */
#define MEMORY_BLOCK_ALIGN (sizeof(void *) * 2)
#define MEMORY_ALIGN_BYTES(buf) buf __attribute__ ((aligned (MEMORY_BLOCK_ALIGN)))

/**
 * @brief This enum defines how many bank the RTE_Memory will handle.
 *
 */
typedef enum __mem_bank_t {
    BANK_INVALID  = -1,
    BANK_0        = 0,
    BANK_1        = 1,
    BANK_CNT,
}mem_bank_t;
/**
 * @brief This struct defines the memory handle.
 *
 */
typedef void* mem_t;
typedef void* pool_t;

typedef struct __mem_handle_t_ {
    mem_t mem;
    pool_t pool;
    void *mutex;
    mutex_lock_f mutex_lock_func;
    mutex_unlock_f mutex_unlock_func;
}mem_handle_t;

/* For include header in CPP code */
#ifdef __cplusplus
extern "C" {
#endif
/**
 * @brief Initialize a memory bank as a memory pool.
 *
 * @param bank
 * @param mem_pool
 * @param mem_pool_size
 */
void memory_pool(mem_bank_t bank, void *mem_pool, size_t mem_pool_size);
/**
 * @brief Regist a mutex and its function for the memory module.
 *
 * @param mutex
 * @param lock_func
 * @param unlock_f
 */
void memory_regist_mutex(mem_bank_t bank, void *mutex,
						mutex_lock_f lock_func, mutex_unlock_f unlock_func);
/**
 * @brief Alloc a size of memory stack.
 *
 * @param bank
 * @param size
 * @return void*
 */
void* memory_alloc(mem_bank_t bank, size_t size);
/**
 * @brief Alloc a size of memory stack and set it to zero.
 *
 * @param bank
 * @param size
 * @return void*
 */
void *memory_calloc(mem_bank_t bank, size_t size);
/**
 * @brief Alloc a size of aligned memory stack.
 *
 * @param bank
 * @param align
 * @param size
 * @return void*
 */
void* memory_alloc_align(mem_bank_t bank, size_t align, size_t size);
/**
 * @brief Free a allocated memory stack.
 *
 * @param bank
 * @param ptr
 */
void memory_free(mem_bank_t bank,void* ptr);
/**
 * @brief Realloc a malloced buffer.
 *
 * @param bank
 * @param ptr
 * @param size
 * @return void*
 */
void* memory_realloc(mem_bank_t bank, void* ptr, size_t size);
/**
 * @brief Demon a bank of memory stack.
 *
 * @param ptr
 * @return size_t
 */
void memory_demon(mem_bank_t bank);
/**
 * @brief Get a malloced buffer's size.
 *
 * @param ptr
 * @return size_t
 */
size_t memory_sizeof_p(void *ptr);
/**
 * @brief Get a free size of a memory bank.
 *
 * @param bank
 * @return size_t
 */
size_t memory_sizeof_free(mem_bank_t bank);
/**
 * @brief Get a max size of a memory bank.
 *
 * @param bank
 * @return size_t
 */
size_t memory_sizeof_max(mem_bank_t bank);
/**
 * @brief Malloc a max free size of a memory bank.
 *
 * @param bank
 * @param size
 * @return void*
 */
void *memory_alloc_max(mem_bank_t bank,size_t *size);

/* For include header in CPP code */
#ifdef __cplusplus
}
#endif
#endif