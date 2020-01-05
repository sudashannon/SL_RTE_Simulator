/*****************************************************************************
*** Author: Shannon
*** Version: 2.4 2018.10.8
*** History: 1.0 创建，修改自tivaware
             2.0 为RTE的升级做适配，更改模块名称
             2.1 动静态结合方式管理
             2.2 引入RTE_Vec进行统一管理
             2.3 多线程环境下引入ALONE机制，确保某些关键timer可以在独立得线程中运行
             2.4 引入多线程多Timer机制，不同线程的Timer按不同线程进行管理，简化已有的查询timer机制
*****************************************************************************/
#include "../include/RTE_RoundRobin.h"
#include "../include/RTE_Memory.h"
#include "../include/RTE_Log.h"
#define THIS_MODULE "RR"
#define RR_LOGF(...) LOG_FATAL(THIS_MODULE, __VA_ARGS__)
#define RR_LOGE(...) LOG_ERROR(THIS_MODULE, __VA_ARGS__)
#define RR_LOGI(...) LOG_INFO(THIS_MODULE, __VA_ARGS__)
#define RR_LOGW(...) LOG_WARN(THIS_MODULE, __VA_ARGS__)
#define RR_LOGD(...) LOG_DEBUG(THIS_MODULE, __VA_ARGS__)
#define RR_LOGV(...) LOG_VERBOSE(THIS_MODULE, __VA_ARGS__)
#define RR_ASSERT(v) LOG_ASSERT(THIS_MODULE, v)
/*************************************************
*** 管理RoundRobin的结构体变量，动态管理
*************************************************/
static rr_t RoundRobinHandle = {0};
/*************************************************
*** Args:   NULL
*** Function: RoundRobin初始化
*************************************************/
void RoundRobin_Init(void)
{
	RoundRobinHandle.TimerGroup = (rr_timergroup_t *)
                                    memory_calloc(BANK_0, sizeof(rr_timergroup_t) * ROUNDROBIN_MAX_GROUP_NUM);
#if ROUNDROBIN_USE_OS == 0
	RoundRobinHandle.RoundRobinRunTick = 0;
#endif
	RoundRobinHandle.TimerGroupCnt = 0;
}/*************************************************
*** Args:   NULL
*** Function: RoundRobin TimerGroup初始化
*************************************************/
rr_group_id_t RoundRobin_CreateGroup(void *mutex,
                                    mutex_lock_f mutex_lock_func,
                                    mutex_unlock_f mutex_unlock_func)
{
    rr_group_id_t retval = 0;
	if(RoundRobinHandle.TimerGroupCnt>=ROUNDROBIN_MAX_GROUP_NUM)
		return RR_NOSPACEFORNEW;
    retval = RoundRobinHandle.TimerGroupCnt;
	RoundRobinHandle.TimerGroup[RoundRobinHandle.TimerGroupCnt].TimerGroupID = retval;
	vector_init(&RoundRobinHandle.TimerGroup[RoundRobinHandle.TimerGroupCnt].SoftTimerTable, BANK_0,
                mutex, mutex_lock_func, mutex_unlock_func);
	RoundRobinHandle.TimerGroupCnt++;
	return retval;
}
/*************************************************
*** Args:
          GroupID 定时器所属Group的ID
					*TimerName 待添加定时器名称
					ReloadValue 重装载值
          ReloadEnable 重装载使能
          ReloadEnable 定时器运行使能
          *TimerCallback 定时器回调函数
          *UserParameters 回调函数输入参数
*** Function: 为当前RoundRobin环境的某一Group添加一个软定时器
*************************************************/
rr_timer_id_t RoundRobin_CreateTimer(
	uint8_t GroupID,
	uint32_t ReloadValue,
	uint8_t ReloadEnable,
	uint8_t RunEnable,
	void (*TimerCallback)(void *),
	void* UserParameters)
{
    uint8_t timer_cnt = vector_size(RoundRobinHandle.TimerGroup[GroupID].SoftTimerTable);
	if(timer_cnt >= ROUNDROBIN_MAX_NUM)
		return RR_NOSPACEFORNEW;
	rr_softtimer_t *v = memory_calloc(BANK_0, sizeof(rr_softtimer_t));
	v->TimerID = timer_cnt;
	v->Flags.F.AREN = ReloadEnable;
	v->Flags.F.CNTEN = RunEnable;
	v->ARR = ReloadValue;
	v->CNT = ReloadValue;
	v->Callback = TimerCallback;
	v->UserParameters = UserParameters;
	vector_push_back(RoundRobinHandle.TimerGroup[GroupID].SoftTimerTable, v);
	return v->TimerID;
}
/*************************************************
*** Args:
					*Name 待删除定时器名称
*** Function: 为当前RoundRobin环境删除一个软定时器
*************************************************/
rr_error_t RoundRobin_RemoveTimer(rr_group_id_t GroupID, rr_timer_id_t TimerID)
{
    uint8_t timer_cnt = vector_size(RoundRobinHandle.TimerGroup[GroupID].SoftTimerTable);
	if(TimerID > timer_cnt) {
        RR_LOGE("No such timer");
        return RR_NOSUCHTIMER;
    }
	memory_free(BANK_0, vector_take_element(RoundRobinHandle.TimerGroup[GroupID].SoftTimerTable, TimerID));
	return RR_NOERR;
}

/*************************************************
*** Args:   Timer 待处理定时器
*** Function: SoftTimer按时处理
*************************************************/
inline static void RoundRobin_CheckTimer(uint8_t GroupID,uint8_t TimerID)
{
    rr_softtimer_t*Timer = ((rr_softtimer_t*)vector_get_element_lockfree(RoundRobinHandle.TimerGroup[GroupID].SoftTimerTable, TimerID));
	/* Check if count is zero */
	if(Timer->CNT == 0) {
		/* Call user callback function */
		Timer->Callback(Timer->UserParameters);
		/* Set new counter value */
		Timer->CNT = Timer->ARR;
		/* Disable timer if auto reload feature is not used */
		if (!Timer->Flags.F.AREN) {
			/* Disable counter */
			RoundRobin_RemoveTimer(GroupID,TimerID);
		}
	}
}
/*************************************************
*** Args:   Null
*** Function: RoundRobin时基函数
*************************************************/
void RoundRobin_TickHandler(void)
{
#if ROUNDROBIN_USE_OS == 0
	RoundRobinHandle.RoundRobinRunTick++;
#endif
	// Loop through each group in the group table.
	for(uint8_t i = 0; i < RoundRobinHandle.TimerGroupCnt; i++) {
        uint8_t timer_cnt = vector_size(RoundRobinHandle.TimerGroup[i].SoftTimerTable);
		// Loop through each task in the task table.
		for(uint8_t j = 0; j < timer_cnt; j++) {
		    rr_softtimer_t*Timer = ((rr_softtimer_t*)vector_get_element_lockfree(RoundRobinHandle.TimerGroup[i].SoftTimerTable,j));
			/* Check if timer is enabled */
			if (Timer->Flags.F.CNTEN) {
				/* Decrease counter if needed */
				if (Timer->CNT)
					Timer->CNT--;
			}
		}
	}
#if ROUNDROBIN_USE_OS == 1
	RoundRobin_Run(SYSTEM_GROUP);
#endif
}
/*************************************************
*** Args:   Null
*** Function: RoundRobin运行函数 在非操作系统环境下调用
*************************************************/
void RoundRobin_Run(uint8_t GroupID)
{
    uint8_t timer_cnt = vector_size(RoundRobinHandle.TimerGroup[GroupID].SoftTimerTable);
	// Loop through each task in the task table.
	for(uint8_t i = 0; i < timer_cnt; i++) {
		RoundRobin_CheckTimer(GroupID,i);
	}
}
/*************************************************
*** Args:
					*Name 待就绪定时器名称
*** Function: 复位当前RoundRobin环境中的一个软定时器
*************************************************/
rr_error_t RoundRobin_ReadyTimer(rr_group_id_t GroupID, rr_timer_id_t TimerID)
{
    rr_softtimer_t*Timer = ((rr_softtimer_t*)vector_get_element(RoundRobinHandle.TimerGroup[GroupID].SoftTimerTable, TimerID));
	Timer->CNT = 0;
	Timer->Flags.F.CNTEN = 1;
	return RR_NOERR;
}
/*************************************************
*** Args:
					*Name 待复位定时器名称
*** Function: 复位当前RoundRobin环境中的一个软定时器
*************************************************/
rr_error_t RoundRobin_ResetTimer(rr_group_id_t GroupID, rr_timer_id_t TimerID)
{
    rr_softtimer_t*Timer = ((rr_softtimer_t*)vector_get_element(RoundRobinHandle.TimerGroup[GroupID].SoftTimerTable, TimerID));
	Timer->CNT = Timer->ARR;
	Timer->Flags.F.CNTEN = 1;
	return RR_NOERR;
}
/*************************************************
*** Args:
					*Name 待暂停定时器名称
*** Function: 暂停当前RoundRobin环境中的一个软定时器
*************************************************/
rr_error_t RoundRobin_PauseTimer(rr_group_id_t GroupID, rr_timer_id_t TimerID)
{
	((rr_softtimer_t*)vector_get_element(RoundRobinHandle.TimerGroup[GroupID].SoftTimerTable,TimerID))->Flags.F.CNTEN = 0;
	return RR_NOERR;
}
/*************************************************
*** Args:
					*Name 待暂停定时器名称
*** Function: 恢复当前RoundRobin环境中的一个软定时器
*************************************************/
rr_error_t RoundRobin_ResumeTimer(rr_group_id_t GroupID, rr_timer_id_t TimerID)
{
	((rr_softtimer_t*)vector_get_element(RoundRobinHandle.TimerGroup[GroupID].SoftTimerTable,TimerID))->Flags.F.CNTEN = 1;
	return RR_NOERR;
}
/*************************************************
*** Args:
					*Name 待暂停定时器名称
*** Function: 恢复当前RoundRobin环境中的一个软定时器
*************************************************/
bool RoundRobin_IfRunTimer(uint8_t GroupID,uint8_t TimerID)
{
	return ((rr_softtimer_t*)vector_get_element(RoundRobinHandle.TimerGroup[GroupID].SoftTimerTable,TimerID))->Flags.F.CNTEN;
}
/*************************************************
*** Args:
					Null
*** Function: 获取当前RoundRobin环境信息
*************************************************/
void RoundRobin_Demon(void)
{
	RR_LOGI("--------------------------------------------------");
	for(uint8_t i=0;i<RoundRobinHandle.TimerGroupCnt;i++)
	{
		RR_LOGI("-------------------------");
		RR_LOGI("Group:%02d Using Timer Count:%d Max Num:%d",
		RoundRobinHandle.TimerGroup[i].TimerGroupID,
		vector_size(RoundRobinHandle.TimerGroup[i].SoftTimerTable),
		ROUNDROBIN_MAX_NUM);
		for(uint8_t j=0;j < vector_size(RoundRobinHandle.TimerGroup[i].SoftTimerTable);j++)
		{
		    rr_softtimer_t*Timer = ((rr_softtimer_t*)vector_get_element_lockfree(RoundRobinHandle.TimerGroup[i].SoftTimerTable,j));
			RR_LOGI("%Timer:02d----AutoReload Enable:%x AutoReload Val:%6d Now Val:%6d Run Enable:%x",
			Timer->TimerID,
			Timer->Flags.F.AREN,
			Timer->ARR,
			Timer->CNT,
			Timer->Flags.F.CNTEN);
		}
	}
}
/*************************************************
*** Args:
					Null
*** Function: 获取当前RoundRobin环境运行时间
*************************************************/
uint32_t RoundRobin_GetTick(void)
{
	/* Return current time in milliseconds */
#if ROUNDROBIN_USE_OS == 1
    return osKernelGetTickCount();
#else
	return RoundRobinHandle.RoundRobinRunTick;
#endif
}
/*************************************************
*** Args:
					prev_tick a previous time stamp (return value of systick_get() )
*** Function: 获取两次tick之间时间差
*************************************************/
uint32_t RoundRobin_TickElaps(uint32_t prev_tick)
{
	uint32_t act_time = RoundRobin_GetTick();
	/*If there is no overflow in sys_time simple subtract*/
	if(act_time >= prev_tick) {
		prev_tick = act_time - prev_tick;
	} else {
		prev_tick = UINT32_MAX - prev_tick + 1;
		prev_tick += act_time;
	}
	return prev_tick;
}
/*************************************************
*** Args:   Delay
					Null
*** Function: 延时一段毫秒
*************************************************/
void RoundRobin_DelayMS(uint32_t Delay) {
	/* Delay for amount of milliseconds */
#if ROUNDROBIN_USE_OS == 0
    uint32_t tickstart = RoundRobin_GetTick();
    while ((RoundRobin_GetTick() - tickstart) < Delay) {
        RoundRobin_Run(SYSTEM_GROUP);
    }
#else
    osDelay(Delay);
#endif
}