#ifndef __main_task_h__
#define __main_task_h__

#ifdef __cplusplus
extern "C" {
#endif

#define MAIN_TASK_NAME     "main-task"
#define MAIN_TASK_STACK    512
#define MAIN_TASK_PRIORITY 1

void main_task(void *);

#ifdef __cplusplus
}
#endif

#endif /* __main_task_h__ */
