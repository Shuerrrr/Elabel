#include "global_message.h"
#include "esp_log.h"
#include "esp_sntp.h"
#include "esp_http_client.h"
#include <time.h>
#include <sys/time.h>
#include "cJSON.h"
//--------------------------------------事件标志组(自定义中断函数)-----------------------------------------//
EventGroupHandle_t get_eventgroupe(void) {
    static EventGroupHandle_t m_eventGroup = NULL; // 静态变量，确保只初始化一次
    if (m_eventGroup == NULL) {
        m_eventGroup = xEventGroupCreate(); // 创建事件组
    }
    return m_eventGroup; // 返回事件组句柄
}
//--------------------------------------事件标志组(自定义中断函数)-----------------------------------------//




bool is_syset_time = false;
time_t now = 0;
struct tm timeinfo = {0};
uint32_t elabelUpdateTick = 0;
TaskNode *task_list = NULL;
uint8_t tasklen = 0;
uint8_t last_tasklen = 0;
//--------------------------------------SNTP时间同步函数-----------------------------------------//
// void initialize_sntp(void)
// {
//     ESP_LOGI("UNIX TIME", "Initializing SNTP");
//     esp_sntp_setoperatingmode(ESP_SNTP_OPMODE_POLL);
//     sntp_setservername(0, "cn.pool.ntp.org");
//     sntp_setservername(1, "210.72.145.44");		// 国家授时中心服务器 IP 地址
//     sntp_setservername(2, "ntp.aliyun.com");   

//     #ifdef CONFIG_SNTP_TIME_SYNC_METHOD_SMOOTH
//         sntp_set_sync_mode(SNTP_SYNC_MODE_SMOOTH);
//     #endif
//     esp_sntp_init();
// }

// void inner_syset()
// {
//     initialize_sntp();

//     int retry = 0;

//     // 等待时间同步完成
//     while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < 200) {
//         vTaskDelay(5000 / portTICK_PERIOD_MS);
//         ESP_LOGE("SNTP_SYTIME", "Waiting for system time to be set... (%d/%d)", retry, 200);
//     }

//     ESP_LOGI("UNIX TIME", "System time synchronized successfully.");
//     is_syset_time = true;
//     vTaskDelete(NULL);
//     // wait for time to be set
// }

// void obtain_time(void *pvParameter)
// {
//     inner_syset();
// }

// void SNTP_syset_time(void)
// {
//     xTaskCreate(&obtain_time, "obtain_time", 4096, NULL, 10, NULL);
// }
//--------------------------------------SNTP时间同步函数------------------------------------------//




//--------------------------------------http时间同步函数-----------------------------------------//
// 定义一个静态缓冲区来存储接收的数据
char *response_buffer = NULL;
int response_buffer_len = 0;
bool send_error = false;

#define URL "http://mshopact.vivo.com.cn/tool/config"
esp_err_t _http_event_handler(esp_http_client_event_t *evt) {
    switch (evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGI("HTTP_SYTIME", "Get_Sys_time_HTTP_EVENT_ERROR");
            send_error = true;
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGI("HTTP_SYTIME", "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGI("HTTP_SYTIME", "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            response_buffer = realloc(response_buffer, response_buffer_len + evt->data_len + 1);
            if (response_buffer == NULL) {
                ESP_LOGI("HTTP_SYTIME", "Failed to allocate memory for response buffer");
                return ESP_ERR_NO_MEM;
            }
            memcpy(response_buffer + response_buffer_len, evt->data, evt->data_len);
            response_buffer_len += evt->data_len;
            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGI("HTTP_SYTIME", "HTTP_EVENT_ON_FINISH");
            if (response_buffer == NULL) break;
            ESP_LOGI("HTTP_SYTIME", "Full response: %s", response_buffer);
            cJSON *json = cJSON_Parse(response_buffer);
            if (json) 
            {
                cJSON *data = cJSON_GetObjectItem(json, "data");
                if (data) 
                {
                    //校准时间
                    long long nowTime = (long long)cJSON_GetObjectItem(data, "nowTime")->valuedouble;
                    struct timeval tv;
                    
                    // 将毫秒时间戳转换为秒和微秒
                    tv.tv_sec = nowTime / 1000;          // 秒
                    tv.tv_usec = (nowTime % 1000) * 1000; // 微秒

                    // 设置系统时间
                    if (settimeofday(&tv, NULL) < 0) {
                        perror("settimeofday");
                    }
                    // Set timezone to China Standard Time
                    setenv("TZ", "CST-8", 1);
                    tzset();
                    time(&now);
	                localtime_r(&now, &timeinfo);
                    is_syset_time = true;
                }
            }
            // 这里可以对完整的响应数据进行处理
            free(response_buffer); // 释放内存
            response_buffer = NULL;
            response_buffer_len = 0;
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI("HTTP_SYTIME", "HTTP_EVENT_ON_DISCONNECTED");
            break;
        default:
            break;
    }
    return ESP_OK;
}

void HTTP_syset_time(void)
{
    send_error = false;

    esp_http_client_config_t sys_time_config = {
        .url = URL,
        .event_handler = _http_event_handler,
    };


    esp_http_client_handle_t sys_time_client = esp_http_client_init(&sys_time_config);

    // Send GET request
    esp_err_t err = esp_http_client_perform(sys_time_client);

    if (err == ESP_OK) {
        ESP_LOGI("HTTP_SYTIME", "HTTPS Status = %d, content_length = %d",
                 esp_http_client_get_status_code(sys_time_client),
                 esp_http_client_get_content_length(sys_time_client));
    } else {
        ESP_LOGE("HTTP_SYTIME", "HTTP GET request failed: %s", esp_err_to_name(err));
    }

    // Cleanup
    esp_http_client_cleanup(sys_time_client); 

    while(!is_syset_time)
    {
        vTaskDelay(100 / portTICK_PERIOD_MS);
        if(send_error) HTTP_syset_time();
    }
}
//--------------------------------------http时间同步函数-----------------------------------------//



//-----------------------------------------时间戳获取-------------------------------------------//
long long get_unix_time(void)
{
    while(!is_syset_time) 
    {
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        ESP_LOGE("UNIX TIME","Waiting Systime set\n");
    }
    // 获取当前 Unix 时间戳
    struct timeval now;

    gettimeofday(&now, NULL);

    // 将秒和微秒转换为毫秒
    long long timestamp_in_ms = (long long)now.tv_sec * 1000 + now.tv_usec / 1000;
    
    ESP_LOGI("UNIX TIME","Return Unix timestamp in milliseconds: %lld\n", timestamp_in_ms);

	// 打印现在时间
	char strftime_buf[64];
	strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
	ESP_LOGI("UNIX TIME", "Current time: %s", strftime_buf);

    return timestamp_in_ms;
}
//-----------------------------------------时间戳获取-------------------------------------------//



//--------------------------------------TODOLIST 对应的结构体--------------------------------------//
SemaphoreHandle_t Global_message_mutex;

task_list_state m_task_list_state = newest;

task_list_state get_task_list_state()
{    
    return m_task_list_state;
};

void set_task_list_state(task_list_state _task_list_state)
{
    m_task_list_state = _task_list_state;
}

void cleantodoItem(TodoItem* _todoitem)
{
    _todoitem->createBy = NULL;
    _todoitem->createTime = 0;
    _todoitem->updateBy = NULL;
    _todoitem->remark = NULL;
    _todoitem->id = 0;
    _todoitem->title = NULL;
    _todoitem->isPressing = 0;
    _todoitem->todoType = NULL;
    _todoitem->isComplete = 0;
    _todoitem->startTime = 0;
    _todoitem->fallTiming = 0;
    _todoitem->isFocus = 0;
    _todoitem->isImportant = 0;    
}

// 根据 ID 查找 TodoItem
TodoItem* find_todo_by_id(TodoList *list, int id) 
{
    for (int i = 0; i < list->size; i++) {
        if (list->items[i].id == id) {
            return &list->items[i];
        }
    }
    ESP_LOGE("Task_list","Can not find todo_by_id/n.");
    return NULL; // 未找到返回 NULL
}

// 根据 Title 查找 TodoItem
TodoItem* find_todo_by_title(TodoList *list, const char *title) 
{
    if(title == NULL)
    {
        ESP_LOGE("Task_list","Can not find todo_by_id/n.");
        return NULL;
    } 
    for (int i = 0; i < list->size; i++) {
        if (list->items[i].title != NULL && strcmp(list->items[i].title, title) == 0) {
            return &list->items[i];
        }
    }
    ESP_LOGE("Task_list","Can not find todo_by_id/n.");
    return NULL; // 未找到返回 NULL
}

//为新的item申请内存
void copy_write_todo_item(TodoItem* src, TodoItem* dst)
{
    if(src->createBy!=NULL) dst->createBy = strdup(src->createBy);
    else dst->createBy = strdup("NULL");

    if(src->updateBy!=NULL) dst->updateBy = strdup(src->updateBy);
    else dst->updateBy = strdup("NULL");

    if(src->remark!=NULL) dst->remark = strdup(src->remark);
    else dst->remark = strdup("NULL");

    if(src->title!=NULL) dst->title = strdup(src->title);
    else dst->title = strdup("NULL");

    if(src->todoType!=NULL) dst->todoType = strdup(src->todoType);
    else dst->todoType = strdup("NULL");

    dst->createTime = src->createTime;
    dst->updateTime = src->updateTime;
    dst->id = src->id;
    dst->isPressing = src->isPressing;
    dst->isComplete = src->isComplete;
    dst->startTime = src->startTime;
    dst->fallTiming = src->fallTiming;
    dst->isFocus = src->isFocus;
    dst->isImportant = src->isImportant;
}

void clean_todo_list(TodoList *list)
{
    for(int i = 0; i<list->size; i++)
    {
        if(list->items[i].createBy!=NULL) free(list->items[i].createBy);
        if(list->items[i].title!=NULL) free(list->items[i].title);
        if(list->items[i].updateBy!=NULL) free(list->items[i].updateBy);
        if(list->items[i].remark!=NULL) free(list->items[i].remark);
        if(list->items[i].todoType!=NULL) free(list->items[i].todoType);
    }
    list->size = 0;
    free(list->items);
    list->items = NULL;
}

// 向 TodoList 添加一个 TodoItem
void add_or_update_todo_item(TodoList *list, TodoItem item) 
{
    if(item.isFocus == 1)
    {   
        if(get_global_data()->m_focus_state->is_focus != 1) 
        {
            get_global_data()->m_focus_state->is_focus = 1;
            get_global_data()->m_focus_state->focus_task_id = item.id;
            ESP_LOGI("Task_list", "A New focus Task show up, its id is %d.\n", item.id);
        }
    }

    list->items = (TodoItem *)realloc(list->items, (list->size + 1) * sizeof(TodoItem));
    //申请内存和指针NULL是两个东西。不写这一句，直接调用list->items[list->size].remark会报错
    list->items[list->size].remark = NULL;
    list->items[list->size].createBy = NULL;
    list->items[list->size].updateBy = NULL;
    list->items[list->size].todoType = NULL;
    list->items[list->size].title = NULL;
    if (list->items == NULL) 
    {
        ESP_LOGE("Task_list", "Memory allocation error!\n");
        return;
    }
    copy_write_todo_item(&item,&(list->items[list->size]));
    list->size++;
    ESP_LOGI("Task_list", "Add new item id is %d, title is %s ,total size of todolist is %d, create time is %lld, falling time is %d, focus state is %d.\n", list->items[list->size-1].id, list->items[list->size-1].title, list->size, item.startTime, item.fallTiming, item.isFocus);
}

TaskNode* create_task(const char* task_content) {
    TaskNode *new_task = (TaskNode *)malloc(sizeof(TaskNode));
    if (new_task == NULL) {
        printf("内存分配失败\n");
        return NULL;
    }
    printf("create_task before dup: %s\n",task_content);
    new_task->task = strdup(task_content);  // 复制任务内容
    new_task->next = NULL;
    return new_task;
}

//修改任务内容
void modify_task(TaskNode *head, int position, const char *task_content) {
    TaskNode *temp = head;
    int index = 0;

    // 遍历链表直到找到指定位置的任务
    while (temp != NULL) {
        if (index == position) {
            free(temp->task);  // 释放原任务字符串的内存
            temp->task = strdup(task_content);  // 复制新的任务内容
            printf("任务 \"%s\" 修改成功！\n", task_content);
            lv_obj_t *ui_tmpButton = lv_obj_get_child(ui_Container3, position+1);
            lv_obj_t *ui_tmpLabel = lv_obj_get_child(ui_tmpButton, 0);
            lv_label_set_text(ui_tmpLabel, task_content);
            return;
        }
        temp = temp->next;
        index++;
    }
    // 如果超出了链表长度，打印错误信息
    printf("指定位置的任务不存在！\n");
}

// 添加任务到链表末尾
void add_task(TaskNode **head, const char *task_content) {

    if(get_task_position(*head,task_content) != -1)
    {
        printf("任务已存在！\n");
        return;
    }

    TaskNode *new_task = create_task(task_content);
    if (*head == NULL) {
        *head = new_task;  // 如果链表为空，设置为头结点
    } else {
        TaskNode *temp = *head;
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = new_task;
    }
    lv_obj_t *ui_tmpButton = lv_btn_create(ui_Container3);
    lv_obj_set_width(ui_tmpButton, 130);
    lv_obj_set_height(ui_tmpButton, 24);
    lv_obj_set_align(ui_tmpButton, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_tmpButton, LV_OBJ_FLAG_SCROLL_ON_FOCUS);     /// Flags
    lv_obj_clear_flag(ui_tmpButton, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    lv_obj_set_style_radius(ui_tmpButton, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_tmpButton, lv_color_hex(0xF6F6F6), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_tmpButton, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui_tmpButton, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui_tmpButton, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_tmpButton, 2, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *ui_tmpLabel = lv_label_create(ui_tmpButton);
    lv_obj_set_width(ui_tmpLabel, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_tmpLabel, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_align(ui_tmpLabel, LV_ALIGN_CENTER);
    lv_label_set_text(ui_tmpLabel, "Task3");
    lv_obj_set_style_text_color(ui_tmpLabel, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_tmpLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    printf("任务 \"%s\" 添加成功！\n", task_content);
    tasklen++;
}

// 删除指定位置的任务
void delete_task(TaskNode **head, int position) {
    if (*head == NULL) {
        printf("任务列表为空，无法删除任务！\n");
        return;
    }

    if(position >= tasklen || position < 0)
    {
        printf("指定位置的任务不存在！\n");
        return;
    }

    TaskNode *temp = *head;

    // 删除头结点
    if (position == 0) {
        *head = temp->next;
        printf("任务 \"%s\" 已删除\n", temp->task);
        free(temp->task);  // 释放任务字符串的内存
        free(temp);        // 释放节点的内存
        tasklen--;
        return;
    }

    // 找到要删除任务的前一个节点
    for (int i = 0; temp != NULL && i < position - 1; i++) {
        temp = temp->next;
    }

    // 如果找不到任务
    if (temp == NULL || temp->next == NULL) {
        printf("指定位置的任务不存在！\n");
        return;
    }

    TaskNode *next = temp->next->next;
    printf("任务 \"%s\" 已删除\n", temp->next->task);
    tasklen--;
    free(temp->next->task);  // 释放任务字符串的内存
    free(temp->next);        // 释放节点的内存

    lv_obj_t *ui_tmpButton = lv_obj_get_child(ui_Container3, position+1);
    lv_obj_del(ui_tmpButton);

    temp->next = next;
}

// 打印任务列表
void print_tasks(TaskNode *head) {
    TaskNode *temp = head;
    int index = 0;
    printf("任务列表:\n");
    while (temp != NULL) {
        printf("任务[%d]: %s\n", index, temp->task);
        temp = temp->next;
        index++;
    }
}

// 释放所有任务的内存
void free_tasks(TaskNode *head) {
    TaskNode *temp;
    while (head != NULL) {
        temp = head;
        head = head->next;
        free(temp->task);  // 释放任务字符串的内存
        free(temp);        // 释放节点的内存
    }
    task_list = NULL;
    tasklen = 0;
}

char* find_task_by_position(TaskNode *head, int position) {
    TaskNode *temp = head;
    int index = 0;

    // 遍历链表直到找到指定位置的任务
    while (temp != NULL) {
        if (index == position) {
            return temp->task;  // 找到任务，返回节点指针
        }
        temp = temp->next;
        index++;
    }

    // 如果超出了链表长度，返回 NULL
    return NULL;
}

int get_task_position(TaskNode *head, const char *task_content)
{
    TaskNode *temp = head;
    int index = 0;

    // 遍历链表直到找到指定位置的任务
    while (temp != NULL) {
        if (strcmp(temp->task,task_content) == 0) {
            return index;  // 找到任务，返回节点指针
        }
        temp = temp->next;
        index++;
    }

    // 如果超出了链表长度，返回 NULL
    return -1;
}


//--------------------------------------TODOLIST 对应的结构体--------------------------------------//


//-------------------------------------- Global_data--------------------------------------//
// 静态变量，存储单例实例
static Global_data *instance = NULL;
// 获取单例实例的函数
Global_data* get_global_data() {
    // 如果实例还未创建，则创建它
    if (instance == NULL) 
    {
        instance = (Global_data*)malloc(sizeof(Global_data));
        if (instance != NULL) 
        {
            Global_message_mutex = xSemaphoreCreateMutex();
            instance->m_focus_state = (Focus_state*)malloc(sizeof(Focus_state));
            instance->m_focus_state->is_focus = 2;
            instance->m_focus_state->focus_task_id = 0;

            instance->m_todo_list = (TodoList*)malloc(sizeof(TodoList));
            instance->m_todo_list->items = NULL;
            instance->m_todo_list->size = 0;

            instance->usertoken = NULL;

            memset(instance->mac_str, 0, sizeof(instance->mac_str));

            instance->newest_firmware_url = NULL;
            instance->version = NULL;
            instance->deviceModel = NULL;
            instance->createTime = NULL;
        }
    }
    return instance;
}
//-------------------------------------- Global_data--------------------------------------//

void print_uint8_array(uint8_t *array, size_t length) {
    // 创建一个字符串缓冲区以存储打印内容
    char buffer[256]; // 假设最大长度为 256 字节
    size_t offset = 0;

    for (size_t i = 0; i < length; i++) {
        // 将每个元素格式化为十六进制并追加到缓冲区
        offset += snprintf(buffer + offset, sizeof(buffer) - offset, "%02X ", array[i]);
    }

    // 使用 ESP_LOGI 打印整个数组
    ESP_LOGI("data", "Array: %s\n", buffer);
}