/**
 * @file RTE_Vector.c
 * @author Leon Shan (813475603@qq.com)
 * @brief A vector implemention.
 * @version 0.1
 * @date 2019-11-03
 *
 * @copyright Copyright (c) 2019
 *
 */
#include "RTE_Vector.h"
#define THIS_MODULE "VECTOR"
#define VEC_LOGF(...) LOG_FATAL(THIS_MODULE, __VA_ARGS__)
#define VEC_LOGE(...) LOG_ERROR(THIS_MODULE, __VA_ARGS__)
#define VEC_LOGI(...) LOG_INFO(THIS_MODULE, __VA_ARGS__)
#define VEC_LOGW(...) LOG_WARN(THIS_MODULE, __VA_ARGS__)
#define VEC_LOGD(...) LOG_DEBUG(THIS_MODULE, __VA_ARGS__)
#define VEC_LOGV(...) LOG_VERBOSE(THIS_MODULE, __VA_ARGS__)
#define VEC_ASSERT(v) LOG_ASSERT(THIS_MODULE, v)
#define VEC_LOCK(v)   ((v->lock_func))(v->mutex)
#define VEC_UNLOCK(v) ((v->unlock_func))(v->mutex)
/**
 * @brief Init a vector from a specific memory bank.
 *
 * @param v
 * @param bank
 * @param mutex
 * @param mutex_lock_func
 * @param mutex_unlock_func
 */
void vector_init(vector_ref_t *v,mem_bank_t bank, void *mutex,
                    mutex_lock_f mutex_lock_func,
                    mutex_unlock_f mutex_unlock_func)
{
    VEC_ASSERT(bank < BANK_CNT);
    VEC_ASSERT(bank != BANK_INVALID);
    vector_t *vector = memory_calloc(bank, sizeof(vector_t));
    if(!vector) {
        VEC_LOGF("can't alloc memory for new vector!\r\n");
        goto end;
    }
    vector->bank = bank;
    vector->length = 0;
    vector->data = NULL;
    vector->mutex = mutex;
    vector->lock_func = mutex_lock_func ? mutex_lock_func : vector_default_mutex_lock;
    vector->unlock_func = mutex_unlock_func ? mutex_unlock_func : vector_default_mutex_unlock;
end:
    *v = vector;
}
/**
 * @brief Get the size of a vector.
 *
 * @param vector
 * @return uint32_t
 */
uint32_t vector_size(vector_ref_t vector)
{
    uint32_t retval = 0;
    VEC_LOCK(vector);
    retval = vector->length;
    VEC_UNLOCK(vector);
    return retval;
}
/**
 * @brief Clean a vector's data.
 *
 * @param vector
 */
void vector_clean(vector_ref_t vector)
{
    uint32_t v_size = 0;
    uint32_t i = 0;
    v_size = vector_size(vector);
    VEC_LOCK(vector);
    for (i = 0; i < v_size; i++) {
        memory_free(vector->bank, vector->data[i]);
        vector->data[i] = NULL;
    }
    memory_free(vector->bank, vector->data);
    vector->data = NULL;
    vector->length = 0;
    VEC_UNLOCK(vector);
}
/**
 * @brief Deinit a vector.
 *
 * @param v
 */
void vector_deInit(vector_ref_t *v)
{
    vector_t *vector = *v;
    vector_clean(vector);
    VEC_LOCK(vector);
    memory_free(vector->bank, vector);
    *v = NULL;
    VEC_UNLOCK(vector);
}
/**
 * @brief Get a element of a vector without lock operation.
 *
 * @param vector
 * @param idx
 * @return void*
 */
void *vector_get_element_lockfree(vector_ref_t vector, uint32_t idx)
{
    return vector->data[idx];
}
/**
 * @brief Get a element of a vector with lock operation.
 *
 * @param vector
 * @param idx
 * @return void*
 */
void *vector_get_element(vector_ref_t vector, uint32_t idx)
{
    void *retval = NULL;
    VEC_LOCK(vector);
    retval = vector->data[idx];
    VEC_UNLOCK(vector);
    return retval;
}
/**
 * @brief Push a element to a vector at the back.
 *
 * @param vector
 * @param element
 */
void vector_push_back(vector_ref_t vector, void *element)
{
    VEC_LOCK(vector);
    vector->data = memory_realloc(vector->bank, vector->data, (vector->length+1) * sizeof(void*));
    vector->data[vector->length] = element;
    vector->length++;
    VEC_UNLOCK(vector);
}
/**
 * @brief Pop the element at the back of a vector.
 *
 * @param vector
 * @return void*
 */
void *vector_pop_back(vector_ref_t vector)
{
    void *retval = NULL;
    VEC_LOCK(vector);
    if (vector->length) {
        vector->length--;
        retval = vector->data[vector->length];
        vector->data[vector->length] = NULL;
        vector->data = memory_realloc(vector->bank, vector->data, (vector->length) * sizeof(void*));
    }
    VEC_UNLOCK(vector);
    return retval;
}
/**
 * @brief Take an element out of a vector.
 *
 * @param vector
 * @param idx
 * @return void*
 */
void *vector_take_element(vector_ref_t vector, uint32_t idx)
{
    void *retval = NULL;
    uint32_t v_size = vector_size(vector);
    if ((v_size > 1) && (v_size >= (idx + 1))) {
        retval = vector_get_element(vector, idx);
        VEC_LOCK(vector);
        /* Since dst is always < src we can just use memcpy */
        memcpy(vector->data + idx, vector->data + idx + 1, (vector->length-idx-1) * sizeof(void*));
        vector->length--;
        vector->data[vector->length] = NULL;
        vector->data = memory_realloc(vector->bank, vector->data, (vector->length)*sizeof(void*));
        VEC_UNLOCK(vector);
    }
    return retval;
}
/**
 * @brief Internal quicksort.
 *
 * @param head
 * @param tail
 * @param comp
 */
static void quicksort(void **head, void **tail, vector_comp_t comp)
{
    while (head < tail) {
        void **h = head - 1;
        void **t = tail;
        void *v = tail[0];
        for (;;) {
            do ++h; while(h < t && comp(h[0], v) < 0);
            do --t; while(h < t && comp(v, t[0]) < 0);
            if (h >= t) break;
            void *x = h[0];
            h[0] = t[0];
            t[0] = x;
        }
        void *x = h[0];
        h[0] = tail[0];
        tail[0] = x;
        // do the smaller recursive call first, to keep stack within O(log(N))
        if (t - head < tail - h - 1) {
            quicksort(head, t, comp);
            head = h + 1;
        } else {
            quicksort(h + 1, tail, comp);
            tail = t;
        }
    }
}
/**
 * @brief Sort user's vector in a certain method.
 *
 * @param vector
 * @param sort_bund
 * @param comp
 */
void vector_qsort(vector_ref_t vector, uint32_t sort_bund, vector_comp_t comp)
{
    VEC_LOCK(vector);
    if(!comp || sort_bund > vector->length)
        return;
    if(sort_bund > 1) {
        quicksort(vector->data, vector->data + sort_bund - 1, comp);
    }
    VEC_UNLOCK(vector);
}
/**
 * @brief Search user's target in a vector.
 *
 * @param vector
 * @param target
 * @param search_bund
 * @param comp
 * @return int32_t
 */
int32_t vector_bsearch(vector_ref_t vector, void *target, uint32_t search_bund, vector_comp_t comp)
{
    int32_t retval = -1;
    void *find = (void *)NULL;
    if(!comp || !target || search_bund > vector->length)
        return retval;
    VEC_LOCK(vector);
    if(search_bund == 0)
        goto end;
    else if(search_bund == 1) {
        if(!comp(target, vector->data[0]))
            retval = 0;
        goto end;
    } else {
        int8_t cmp_ret = 0;
        uint32_t first = 0, last = search_bund, medium = 0;
        while(first <= last) {
            medium = first + (last - first)/2;
            find = vector->data[medium];
            cmp_ret = comp(target, find);
            if(cmp_ret < 0)
                last = medium - 1;
            else if(cmp_ret > 0)
                first = medium;
            else {
                retval = medium;
                goto end;
            }
        }
    }
end:
    VEC_UNLOCK(vector);
    return retval;
}


#if VECTOR_UST_TEST == 1
static uint64_t test_get_tick(void)
{
    return 123456789;
}
static size_t test_data_out (uint8_t *data,size_t length)
{
    printf("%.*s",length,(char *)data);
    return length;
}
/**
 * @brief A general main function for test.
 *
 * @param argc
 * @param argv
 * @return int
 */
int main(int argc, char *argv[]){
    vector_t *test_vector = NULL;
    log_init(NULL, test_data_out, NULL, NULL, test_get_tick);
    VEC_ASSERT(test_vector);
    static uint8_t bank_0[100*1024] = {0};
    memory_pool(BANK_0, bank_0, 102400);
    memory_demon(BANK_0);
    struct {
        uint8_t *data;
        size_t length;
    } test_ele = {
        .data = NULL,
        .length = 0,
    };
    test_ele.data = memory_alloc(BANK_0, 1024);
    test_ele.length = 1024;
    memory_demon(BANK_0);
    VEC_LOGI("p_test's size %d\r\n", memory_sizeof_p(test_ele.data));
    VEC_LOGI("Init a vector.\r\n");
    vector_init(&test_vector, BANK_0, NULL, NULL, NULL);
    VEC_LOGI("Push a element into the vector.\r\n");
    vector_push_back(test_vector, &test_ele);
    VEC_LOGI("The vector has %d element.\r\n", vector_size(test_vector));
    memory_demon(BANK_0);
    return 0;
}
#endif
