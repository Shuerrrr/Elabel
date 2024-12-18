#ifndef GLOBAL_MESSAGE_H
#define GLOBAL_MESSAGE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "Firmware_define.h"
#include "ui.h"

#define FIRMWARE_VERSION "2.0.0"

//--------------------------------------事件标志组(自定义中断函数)-----------------------------------------//
#define ENCODER_TASK_BIT ( 1UL << 0UL )
#define BUTTON_TASK_BIT ( 1UL << 1UL )

#ifdef __cplusplus
extern "C" {
#endif
EventGroupHandle_t get_eventgroupe(void);
#ifdef __cplusplus
}
#endif
//--------------------------------------事件标志组(自定义中断函数)-----------------------------------------//




//--------------------------------------SNTP时间同步函数-----------------------------------------//
#ifdef __cplusplus
extern "C" {
#endif
// void SNTP_syset_time(void);

void HTTP_syset_time(void);

long long get_unix_time(void);
#ifdef __cplusplus
}
#endif
//--------------------------------------SNTP时间同步函数-----------------------------------------//






//--------------------------------------Focus 对应的结构体--------------------------------------//
typedef struct {
    int is_focus; 
    int focus_task_id;
} Focus_state;

//--------------------------------------Task_list对应的结构体--------------------------------------//
typedef enum {
    newest,
    updating_from_server,
    firmware_need_update,
} task_list_state;

//2表示否 1表示是 0表示没东西
typedef struct {
    char *createBy;
    long long createTime;
    char *updateBy;
    long long updateTime;
    char *remark;
    int id;
    char *title;
    int isPressing;
    char *todoType;
    int isComplete;
    long long startTime;
    int fallTiming;
    int isFocus;
    int isImportant;
} TodoItem;

typedef struct {
    TodoItem *items;  // 动态数组存储 TodoItem
    int size;         // 当前数组中的元素数量
} TodoList;

typedef struct TaskNode {
    char *task;               // 任务字符串
    struct TaskNode *next;    // 下一个任务节点
} TaskNode;

#ifdef __cplusplus
extern "C" {
#endif

// 创建新任务
TaskNode* create_task(const char* task_content);

void modify_task(TaskNode *head, int position, const char *task_content);

// 添加任务到链表末尾
void add_task(TaskNode **head, const char *task_content);

// 删除指定位置的任务
void delete_task(TaskNode **head, int position);

// 打印任务列表
void print_tasks(TaskNode *head);

// 释放所有任务的内存
void free_tasks(TaskNode *head);

char* find_task_by_position(TaskNode *head, int position);

int get_task_position(TaskNode *head, const char *task_content);

task_list_state get_task_list_state(void);

void set_task_list_state(task_list_state _task_list_state);

//TodoItem初始化
void cleantodoItem(TodoItem* _todoitem);

// 根据 ID 查找 TodoItem
TodoItem* find_todo_by_id(TodoList *list, int id);

// 根据 Title 查找 TodoItem
TodoItem* find_todo_by_title(TodoList *list, const char *title);

// 向 TodoList 添加一个 TodoItem
void add_or_update_todo_item(TodoList *list, TodoItem item);

//清除 TodoList
void clean_todo_list(TodoList *list);
#ifdef __cplusplus
}
#endif

//-------------------------------------- Global_data--------------------------------------//
typedef struct 
{
    Focus_state* m_focus_state;
    TodoList* m_todo_list;

    char* usertoken;

    uint8_t mac_uint[6];
    char mac_str[18];
    //为了确保字符串能够正确存储格式化后的 MAC 地址，并以 '\0' 结尾，字符串的大小应该至少为 18 字节。具体计算如下：
    // 每个字节以两位十六进制表示：02（2 字符）
    // 6 个字节的 MAC 地址：6 * 2 = 12 字符
    // 5 个冒号分隔符：5 字符
    // 1 个结束字符 '\0'：1 字符
    
    char* newest_firmware_url;
    char *version;
    char *deviceModel;
    char *createTime;

} Global_data;

typedef enum {
    TASK_NO_CHANGE,
    ADD_TASK,
    DELETE_TASK,
    UPDATE_TASK,
} APP_task_state;

extern uint32_t elabelUpdateTick;
extern TaskNode *task_list;
extern uint8_t tasklen;
extern uint8_t last_tasklen;

#ifdef __cplusplus
extern "C" {
#endif
// 获取单例实例的函数
Global_data* get_global_data();
#ifdef __cplusplus
}
#endif

void print_uint8_array(uint8_t *array, size_t length);
#endif