#ifndef __RTE_ROUNDROBIN_H
#define __RTE_ROUNDROBIN_H
#include "RTE_Config.h"
#if ROUNDROBIN_USE_OS == 1
#include "../utils/cmsis_os/include/cmsis_os2.h"
#endif
#include "RTE_Vector.h"
#define SYSTEM_GROUP                  0
typedef uint8_t rr_error_t;
typedef uint8_t rr_timer_id_t;
typedef uint8_t rr_group_id_t;
enum
{
	RR_NOERR = 0,
	RR_HANDLEUNINIT = 1,
	RR_NOSPACEFORNEW = 2,
	RR_ALREADYEXIST = 3,
	RR_NOSUCHTIMER = 4,
	RR_NOSUCHGROUP = 5,
};
typedef struct
{
	union {
		struct {
			uint8_t AREN:1;     /*!< Auto-reload enabled */
			uint8_t CNTEN:1;    /*!< Count enabled */
		} F;
		uint8_t FlagsVal;
	} Flags;
	rr_timer_id_t TimerID;      /*!< Timer ID */
	volatile uint32_t ARR;      /*!< Auto reload value */
	volatile uint32_t CNT;      /*!< Counter value, counter counts down */
	void (*Callback)(void *);	/*!< Callback which will be called when timer reaches zero */
	void* UserParameters;       /*!< Pointer to user parameters used for callback function */
} rr_softtimer_t;
typedef struct
{
	rr_group_id_t TimerGroupID;
	vector_t *SoftTimerTable;
#if ROUNDROBIN_USE_OS == 1
    osMutexId_t TimerGroupMutex;
#endif
} rr_timergroup_t;
typedef struct
{
	uint8_t TimerGroupCnt;
#if ROUNDROBIN_USE_OS == 0
	volatile uint32_t RoundRobinRunTick;
#endif
	rr_timergroup_t *TimerGroup;
    void *mutex;
    mutex_lock_f mutex_lock_func;
    mutex_unlock_f mutex_unlock_func;
} rr_t;
#ifdef __cplusplus
extern "C" {
#endif

extern void RoundRobin_Init(void);

extern rr_group_id_t RoundRobin_CreateGroup(void *mutex,
                                            mutex_lock_f mutex_lock_func,
                                            mutex_unlock_f mutex_unlock_func);

extern rr_timer_id_t RoundRobin_CreateTimer(
	uint8_t GroupID,
	uint32_t ReloadValue,
	uint8_t ReloadEnable,
	uint8_t RunEnable,
	void (*TimerCallback)(void *),
	void* UserParameters);

extern rr_error_t RoundRobin_RemoveTimer(rr_group_id_t GroupID, rr_timer_id_t TimerID);

extern void RoundRobin_TickHandler(void);

extern void RoundRobin_Run(uint8_t GroupID);

extern rr_error_t RoundRobin_ReadyTimer(rr_group_id_t GroupID, rr_timer_id_t TimerID);

extern rr_error_t RoundRobin_ResetTimer(rr_group_id_t GroupID, rr_timer_id_t TimerID);

extern rr_error_t RoundRobin_PauseTimer(rr_group_id_t GroupID, rr_timer_id_t TimerID);

extern rr_error_t RoundRobin_ResumeTimer(rr_group_id_t GroupID, rr_timer_id_t TimerID);

extern bool RoundRobin_IfRunTimer(rr_group_id_t GroupID, rr_timer_id_t TimerID);

extern void RoundRobin_Demon(void);

extern uint32_t RoundRobin_GetTick(void);

extern uint32_t RoundRobin_TickElaps(uint32_t prev_tick);

extern void RoundRobin_DelayMS(uint32_t Delay);

#ifdef __cplusplus
}
#endif
#endif