/**
 * @file RTE_Vector.h
 * @author Leon Shan (813475603@qq.com)
 * @brief
 * @version 0.1
 * @date 2019-11-03
 *
 * @copyright Copyright (c) 2019
 *
 */
#ifndef __RTE_VECTOR_H
#define __RTE_VECTOR_H
#include "RTE_Config.h"
#include "../include/RTE_Memory.h"
typedef int8_t (*vector_comp_t)(const void*, const void*);
/**
 * @brief Struct define for vector.
 *
 */
typedef struct __vector_t_ {
    uint32_t length;
    mem_bank_t bank;
    mutex_lock_f lock_func;
    mutex_unlock_f unlock_func;
    void *mutex;
    void **data;
} vector_t, *vector_ref_t;
/* For include header in CPP code */
#ifdef __cplusplus
extern "C" {
#endif
/**
 * @brief Default lock function for a null mutex.
 *
 * @param mutex
 * @return int8_t
 */
static inline int8_t vector_default_mutex_lock(void *mutex)
{
    UNUSED(mutex);
    return 0;
}
/**
 * @brief Default unlock function for a null mutex.
 *
 * @param mutex
 * @return int8_t
 */
static inline int8_t vector_default_mutex_unlock(void *mutex)
{
    UNUSED(mutex);
    return 0;
}
/**
 * @brief Init a vector from a specific memory bank.
 *
 * @param v
 * @param bank
 * @param mutex
 * @param mutex_lock_func
 * @param mutex_unlock_func
 */
extern void vector_init(vector_ref_t *v,mem_bank_t bank, void *mutex,
                    mutex_lock_f mutex_lock_func,
                    mutex_unlock_f mutex_unlock_func);
/**
 * @brief Get the size of a vector.
 *
 * @param vector
 * @return uint32_t
 */
extern uint32_t vector_size(vector_ref_t vector);
/**
 * @brief Clean a vector's data.
 *
 * @param vector
 */
extern void vector_clean(vector_ref_t vector);
/**
 * @brief Deinit a vector.
 *
 * @param v
 */
extern void vector_deInit(vector_ref_t *v);
/**
 * @brief Get a element of a vector without lock operation.
 *
 * @param vector
 * @param idx
 * @return void*
 */
extern void *vector_get_element_lockfree(vector_ref_t vector, uint32_t idx);
/**
 * @brief Get a element of a vector with lock operation.
 *
 * @param vector
 * @param idx
 * @return void*
 */
extern void *vector_get_element(vector_ref_t vector, uint32_t idx);
/**
 * @brief Push a element to a vector at the back.
 *
 * @param vector
 * @param element
 */
extern void vector_push_back(vector_ref_t vector, void *element);
/**
 * @brief Pop the element at the back of a vector.
 *
 * @param vector
 * @return void*
 */
extern void *vector_pop_back(vector_ref_t vector);
/**
 * @brief Take an element out of a vector.
 *
 * @param vector
 * @param idx
 * @return void*
 */
extern void *vector_take_element(vector_ref_t vector, uint32_t idx);
/**
 * @brief Sort user's vector in a certain method.
 *
 * @param vector
 * @param sort_bund
 * @param comp
 */
extern void vector_qsort(vector_ref_t vector, uint32_t sort_bund, vector_comp_t comp);
/**
 * @brief Search user's target in a vector.
 *
 * @param vector
 * @param target
 * @param search_bund
 * @param comp
 * @return int32_t
 */
extern int32_t vector_bsearch(vector_ref_t vector, void *target, uint32_t search_bund, vector_comp_t comp);
/* For include header in CPP code */
#ifdef __cplusplus
}
#endif
#endif
