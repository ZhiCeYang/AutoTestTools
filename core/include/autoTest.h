#ifndef __AUTOTEST_H__
#define __AUTOTEST_H__
#include "at_network.h"

#define INVITE                "INVITE"
#define ACK                   "ACK_OK"
#define CONTINUE_FLAG       1
#define BREAK_FLAG          0

#define MAX_CMD_LEN         256
#define MAX_BUFF_SIZE       300
#define MAX_LOG_SIZE        150
#define LOG_PATH            "./log"
#define TEST_NOTE_FILE      "./log/testNote.txt"

typedef struct s_clientParam{
    int32_t clientNum;
    int32_t socketFd;
    struct sockaddr_in clientAddr;
    int32_t addr_len;
}s_clientParam;

typedef enum logStr{
    LOG_SUSPEND = 0,
    LOG_TALK_BUILD,
    LOG_TALK_CONFERENCE,
    LOG_INVALID
}eLogStr;
#define MAX_LOG_INDEX LOG_INVALID
#define MAX_LOG_STR_LEN 20

void cmd_send(void);
void ret_receive(void);
LOCAL int32_t syslogServerInit();
int32_t getSysLogPortNum();
LOCAL void syslogProcess(char *log);
typedef void (*ptr_to_func)  (int32_t) ;
ptr_to_func signal (int32_t , ptr_to_func );
void sig_handler(int32_t sig);
/*void (*signal (int sig ,void (*func)(int))) (int) ;*/
pthread_t getSendThreadID(void);
void setSendThreadID(pthread_t id);
pthread_t getRecThreadID(void);
void setRecThreadID(pthread_t id);
pthread_t getScriptThreadID(int32_t scriptNum);
void setScriptThreadID(int32_t scriptNum,pthread_t id);
void getSysTime(c8_t *p_time);
int32_t getClientAge(int32_t clientNum);
#endif
