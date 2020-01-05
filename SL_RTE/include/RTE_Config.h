/**
 * @file RTE_Config.h
 * @author Leon Shan (813475603@qq.com)
 * @brief
 * @version 0.1
 * @date 2019-12-22
 *
 * @copyright Copyright (c) 2019
 *
 */
#ifndef __RTE_CONFIG_H
#define __RTE_CONFIG_H
/**
 * @brief General include.
 *
 */
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
/**
 * @brief Log Configuration.
 *
 */
#define LOG_UST_TEST            0           // define for if enable test main
#define LOG_MAX_HEAD_LENGTH     128         //
#define LOG_MAX_FILTER_CNT      8           //
/**
 * @brief Memory Configuration.
 *
 */
#define MEMORY_UST_TEST             0           /* define for if enable test main */
#define MEMORY_USE_64BIT            1
/**
 * @brief Vector Configuration.
 *
 */
#define VECTOR_UST_TEST             0           /* define for if enable test main */
/**
 * @brief RoundRobin Configuration.
 *
 */
#define ROUNDROBIN_USE_OS             0
#define ROUNDROBIN_MAX_NUM    		  16
#define ROUNDROBIN_MAX_GROUP_NUM      4
/**
 * @brief Some general typedef.
 *
 */
/* Typedef for mutex */
typedef int8_t (*mutex_lock_f)(void *mutex);
typedef int8_t (*mutex_unlock_f)(void *mutex);
/* Typedef for unused */
#ifndef UNUSED
#define UNUSED(x)                   (void)x
#endif
#endif