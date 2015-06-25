#ifndef __CMDPROC_H__
#define __CMDPROC_H__
#include <stdlib.h>
#include <stdio.h>
#include <String.h>
#include "at_common.h"
#include "autoTest.h"
#include "at_network.h"
#include "keyMap.h"

#define MSG_FILE        "./msgQ"
#define C_SEMICOLON     ";"
#define C_SPACE         " "
#define C_COMMA         ","
#define C_POUND         '#'
#define NEWLINE         '\n'

#define MAX_CMD_NUM     18
#define MAX_FILE_NAME    60
#define MAX_SC_CONTENT     256
#define MAX_SC_LINE         500
#define MAX_SCRIPT_THREAD   10

#define CMD_LOOP         "loop"
#define LOOP_START       "start"
#define LOOP_END         "end"
#define CMD_DELAY        "delay"
#define CMD_CALL         "call"
#define CMD_IOEVT        "ioevt"
#define CMD_RECORD       "record"

#define MAX_TOKEN         15
#define AT_MAX_CONN       5
#define MAX_TEL_NUM       10
#define MAX_CMD_CODE      15
#define MAX_IOEVT         10
#define MAX_SHARED_LENTH  60
#define MAX_LEVEL_LENTH   10
#define MAX_TYPE_LENTH    10

typedef struct {
    char ioevt[MAX_KEY_CODE][MAX_KEY_LENTH];
    char action[MAX_ACT_LENTH];
}ioevt_t;
typedef union {
    char shared[MAX_SHARED_LENTH];
    ioevt_t ioevtObj;
}cmdParam_t;
typedef struct {
    char cmdCode[MAX_CMD_CODE];
    cmdParam_t cmdParamObj;
}cmdObj_t;

typedef struct {
    c8_t clientNum;
    cmdObj_t cmdObj;
}msgQ_t; 
typedef struct  {
    c8_t*   cmdName;
    c8_t*   nickName;
    c8_t*   desc;
    c8_t*	example;
    void (*func) (int32_t cmdNum);
}cmd_t;
typedef struct {
     c8_t scriptCmd[MAX_SC_LINE][MAX_CMD_LEN];
}scriptParam;
#define MSG_TYPE           1

#define RD_WR     2
#define RD        0
#define WR        1

#define PARAM_TEST            "test"
#define PARAM_REC             "rec"
#define PARAM_SC              "script"
#define PARAM_ALL             "all"
#define STR_CLIENT            "client"
#define STR_SYSLOG_PORT       "syslogPort"

int64_t getFileLength(FILE *filp);
int32_t cpScriptContentToArray(FILE *cmdFd, c8_t (*p_scriptCont)[MAX_CMD_LEN]);
int32_t getCmdContent(c8_t *p_scriptCont,const c8_t *scriptCmd[],int32_t *scriptLineNum);
void delEnterSymbol(c8_t *src);
void cmd_proc(c8_t *cmdLine);
int32_t getCmdCode(const cmd_t *cmdLib, const c8_t *str);
int32_t getTokens(const c8_t *inStr, const c8_t* delim,const c8_t  *pArgv[]);
 int32_t getRandValue(int32_t min_rand,int32_t max_rand);
void setCountOfSuccessCall(int type,int clientNum,int32_t value);
int32_t getCountOfSuccessCall(int type,int clientNum);
void setCountOfScriptRun(int32_t scriptNum,int32_t value);
int32_t getCountOfScriptRun(int32_t scriptNum);
void gScriptParamInit();

/*definition of cmdfunc property*/
void  showHelp(int32_t cmdNum);
void  autoConfig(int32_t cmdNum);
void  addClient(int32_t cmdNum);
void  delClient(int32_t cmdNum);
void  changeClientIpAddr(int32_t cmdNum);
void  showClient(int32_t cmdNum);
void  ioevt(int32_t cmdNum);
void  call(int32_t cmdNum);
void  setSysLogLevel(int32_t cmdNum);
void  start(int32_t cmdNum);
void  stop(int32_t cmdNum);
void  quit(int32_t cmdNum);
void  runScript(int32_t cmdNum);
void  delay(int32_t cmdNum);
void  randomTime(int32_t cmdNum);
void  randomKey(int32_t cmdNum);
void  look(int32_t cmdNum);
void  syslogConf(int32_t cmdNum);
void hideFeatureProc(int32_t cmdNum);
void  scriptCmdProcess();
#endif
