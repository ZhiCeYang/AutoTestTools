#include <string.h>
#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <fcntl.h>  

#include "at_cmdProc.h"
#include "at_network.h"
#include "autoTest.h"
#include "keyMap.h"

IMPORT pthread_mutex_t gMutex;
IMPORT at_key_t gKeyLib[MAX_KEY_NUM];
const c8_t *gCmdParam[MAX_TOKEN]; 
LOCAL c8_t gScriptName[MAX_SCRIPT_THREAD][MAX_FILE_NAME];
LOCAL c8_t gConfFileName[MAX_FILE_NAME];
LOCAL scriptParam *gScriptParam[MAX_SCRIPT_THREAD];
LOCAL int32_t gParamNum = 0;
int32_t gMsgid;
LOCAL int32_t gSuccessCall[MAX_CALL_TYPE][MAX_CLIENT];
LOCAL int32_t gScriptCount[MAX_SCRIPT_THREAD];

cmd_t gCmdLib[MAX_CMD_NUM] = {
    {
        .cmdName = "help",
        .nickName = "h",
        .example = "help: h/help",
        .func  =  showHelp,
    },
    {
        .cmdName = "autoConfig",
        .nickName = "ac",
        .example = "autoConfig: autoConfig config.txt",
        .func  =  autoConfig,
    },
    {
        .cmdName = "add",
        .nickName = "ad",
        .example = "add: add 1(client number) 192.168.10.1(ip address)",
        .func = addClient,
    },
    {
        .cmdName = "call",
        .nickName = "call",
        .example = "call: call 1(client number) 1000(tel number)",
        .func = call,
    },
    {
        .cmdName = "changeIp",
        .nickName = "ci",
        .example = "changeIp: changeIp 1(client number) 192.168.10.1(ip address)",
        .func  = changeClientIpAddr,
    },    
    {
        .cmdName = "del",
        .nickName = "dl",
        .example = "del: del 1(client number)",
        .func = delClient,
    },
    {
        .cmdName = "delay",
        .nickName = "dy",
        .example = "delay: delay 5(secends)",
        .func = delay,
    },
    {
        .cmdName = "ioevt",
        .nickName = "ie",
        .example = "ioevt: ioevt 1(client number) 1,2(key number) p/l(action)",
        .func = ioevt,
    },
    {
        .cmdName = "look",
        .nickName = "l",
        .example = "look: look",
        .func = look,
    },    
    {
        .cmdName = "quit",
        .nickName = "q",
        .example = "quit: quit",
        .func = quit,
    }, 
    {
        .cmdName = "runScript",
        .nickName = "rs",
        .example = "runScript: runScript 1(thread Num) script.txt(script file name)",
        .func = runScript,
    },

    {
        .cmdName = "randomTime",
        .nickName = "rt",
        .example = "randomTime: randomTime 1(min) 10(max)",
        .func = randomTime,
    },
    {
        .cmdName = "randomKey",
        .nickName = "rk",
        .example = "randomKey: randomKey 1(client Num)",
        .func = randomKey,
    },
    {
        .cmdName = "show",
        .nickName = "s",
        .example = "show: show",
        .func = showClient,
    },
    {
        .cmdName = "start",
        .nickName = "st",
        .example = "start: ____test: start test\n\
         |_rec:  start rec\n\
         |_all:  start all",
        .func = start,
    },
    {
        .cmdName = "stop",
        .nickName = "sp",
        .example = "stop:  ____test: stop test\n\
         |_rec:  stop rec\n\
         |_rec:  stop 1(thread num) script\n\
         |_all:  stop all",
        .func = stop,
    },
    {
        .cmdName = "syslog",
        .nickName = "sl",
        .example = "syslog: syslog 1(client) enable/disable/record",
        .func = syslogConf,
    },
    {
        .cmdName = "hideFeature",
        .nickName = "hf",
        .example = "hideFeature: hideFeature 1(client) enable/disable",
        .func = hideFeatureProc,
    }

};
char *gCommands[] = {
    "help",         "autoConfig",
    "add",          "call",         
    "changeIp",     "del",  
    "test",         "rec",
    "delay",        "all",
    "ioevt",        "ie",           
    "look",         "quit",
    "runScript",    "rs",
    "randomTime",   "rt",
    "randomKey",    "rk",
    "show",         "start",  
    "stop",         "script",
    "syslog",       "sl",
    "hideFeature",  "hf",              
    "enable",       "disable",
    "show",         "s",
    "192.168.10."
};
int64_t getFileLength(FILE *filp)
{
	int64_t curpos,length;
	curpos = ftell(filp);
	fseek(filp,0,SEEK_END);
	length = ftell(filp);
	rewind(filp);
	return length;
}
int32_t cpScriptContentToArray(FILE *cmdFd, c8_t (*p_scriptCont)[MAX_CMD_LEN])
{
	size_t sizeRead;
	int32_t tmp_index = 0;
	c8_t *temp;
	c8_t *p;
	if (cmdFd == NULL || p_scriptCont == NULL){
        return FAILED;
	}
	temp = (c8_t *) malloc(MAX_CMD_LEN);
	if(temp == NULL){
        return  FAILED;
	}
    while(1){  
        memset(temp,0,MAX_CMD_LEN);
        if(fgets(temp,MAX_CMD_LEN,cmdFd) == NULL){
            break;
        }   
        if(temp[0] == C_POUND || temp[0] == NEWLINE){
            continue;
        }
        if((p = strstr(temp,C_SEMICOLON)) != NULL){
            *p = '\0';//cmdTmp:****;\n\0             
            snprintf(p_scriptCont[tmp_index],MAX_CMD_LEN,"%s",temp);
            tmp_index ++;
        }
        else {
            printf("parse error!\n");
            continue;
        }        
    }
    free(temp);
	return tmp_index -1;
}
int32_t getCmdContent(c8_t *p_scriptCont,const c8_t *scriptCmd[],int32_t *scriptLineNum)
{
	if(p_scriptCont == NULL || scriptCmd == NULL){
		printf("invalid parameter!\n");
		return FAILED;
	}
	*scriptLineNum = getTokens(p_scriptCont,C_SEMICOLON,scriptCmd);
	if(*scriptLineNum == ERROR){
        printf("getCmdContent ERROR\n");
        return FAILED;
	}
	return OK;
}

void cmd_proc(c8_t *cmdLine)
{
    int32_t cmdNum;
    gParamNum = 0;
    gParamNum = getTokens(cmdLine,C_SPACE,gCmdParam);
    if((cmdNum = getCmdCode(gCmdLib,gCmdParam[0])) == FAILED ){
        printf("cmd not found!\n");
        return;
    }
    gCmdLib[cmdNum].func(cmdNum);
    
}
int32_t getCmdCode(const cmd_t *gCmdLib, const c8_t *str) 
{
	int32_t cmd;
	for (cmd = 0; cmd < MAX_CMD_NUM; cmd++) {
		if (strcmp(gCmdLib[cmd].cmdName, str) == 0 || \
		    strcmp(gCmdLib[cmd].nickName, str) == 0) {
			return cmd;
		}
	}
	return FAILED;
}
int32_t getTokens(const c8_t *inStr, const c8_t* delim,const c8_t *pArgv[]) {
	int32_t numOfToken = 0;
	c8_t *t;
	LOCAL c8_t tmp[MAX_SC_CONTENT];
	memset(tmp,0,MAX_SC_CONTENT);
	snprintf(tmp,sizeof(tmp),inStr);
	t = strtok(tmp, delim);
	while(t && numOfToken < MAX_TOKEN) 	{
		pArgv[numOfToken] = t;
        numOfToken ++;
        t = strtok(NULL, delim);
	}
	numOfToken -= 1;
	
	return numOfToken;
}
int32_t getRandValue(int32_t min_rand,int32_t max_rand)
{
    int32_t ret;
    if(max_rand <= 0 || min_rand < 0|| max_rand < min_rand){
        return FAILED;
    }
    ret = (min_rand+(int32_t)((float)max_rand*rand()/(RAND_MAX+1.0))); 
    return ret;
 }
void setCountOfSuccessCall(int type,int clientNum,int32_t value)
{
    if(value < 0){
        return;
    }
    gSuccessCall[type][clientNum] = value;
}
int32_t getCountOfSuccessCall(int type,int clientNum)
{
  return gSuccessCall[type][clientNum];
}
void setCountOfScriptRun(int32_t scriptNum,int32_t value)
{
    if(value < 0){
        return;
    }
    gScriptCount[scriptNum]= value;
}
int32_t getCountOfScriptRun(int32_t scriptNum)
{
  return gScriptCount[scriptNum];
}
void gScriptParamInit()
{
    int32_t i;
   for(i = 0;i < MAX_SCRIPT_THREAD;i ++){
        memset(gScriptName[i],0,sizeof(gScriptName[i]));
   }
   
}
/*cmdfunc property*/
void  showHelp(int32_t cmdNum)
{
    int32_t index;
    for(index = 0;index < MAX_CMD_NUM;index ++){
        if(strlen(gCmdLib[index].example) > 0){
            printf("%s\n",gCmdLib[index].example);
        }
    }
}
void  autoConfig(int32_t cmdNum)
{
    FILE *confFilp;  
	int32_t ret;
	c8_t *p = NULL;
	c8_t cmdTmp[MAX_CMD_LEN];
	if(gParamNum != 1){
        printf("wrong parameter!!\n");
        printf("%s\n",gCmdLib[cmdNum].example);
        return;
    }
	memset(gConfFileName,0,sizeof(gConfFileName));
	snprintf(gConfFileName,sizeof(gConfFileName),gCmdParam[1]);
    confFilp = fopen(gConfFileName,"r");
    if(confFilp == NULL)return;
    while(1){  
        memset(cmdTmp,0,sizeof(cmdTmp));
        if(fgets(cmdTmp,MAX_CMD_LEN,confFilp) == NULL){
            break;
        }
        if(cmdTmp[0] == C_POUND || cmdTmp[0] == NEWLINE){
            continue;
        }
        if((p = strstr(cmdTmp,C_SEMICOLON)) != NULL){
            *p = '\0';//cmdTmp:****;\n\0
        }
        else {
            printf("parse error!\n");
            continue;
        }
        cmd_proc(cmdTmp);
    }
    fclose(confFilp);
}
void  addClient(int32_t cmdNum)
{
    int32_t clientNum;
    c8_t *ipAddr;
    int32_t stat;
    if(gParamNum < 2){
        printf("wrong parameter!!\n");
        printf("%s\n",gCmdLib[cmdNum].example);
        return;
    }
    clientNum = atoi(gCmdParam[1]);
    ipAddr = (c8_t *)gCmdParam[2];
    pthread_mutex_lock(&gMutex);
    stat = getClientFlag(clientNum);
    pthread_mutex_unlock(&gMutex);
    if(USED == stat){
         printf("client num exist! add failed\n");                  
         return;
    }
    if(AT_FAILED== network_config(clientNum,ipAddr)){
        printf("%s\n",gCmdLib[cmdNum].example);
        return;
    }
    pthread_mutex_lock(&gMutex);
    setClientFlag(clientNum, USED);
    setClientIpAddr(clientNum,ipAddr);
    pthread_mutex_unlock(&gMutex);
    printf("add client OK!\n");
}
void  delClient(int32_t cmdNum)
{
    int32_t clientNum;
    int32_t stat;
    clientNum = atoi(gCmdParam[1]);
    if(gParamNum < 1 || clientNum > MAX_CLIENT || clientNum < MIN_CLIENT){
        printf("wrong parameter!!\n");
        printf("%s\n",gCmdLib[cmdNum].example);
        return;
    }
    pthread_mutex_lock(&gMutex);
    stat = getClientFlag(clientNum);
    pthread_mutex_unlock(&gMutex);
    if(stat == UNUSED){
        printf("client %d not configured!\n",clientNum);
        return;
    }
    pthread_mutex_lock(&gMutex);
    setClientFlag(clientNum,UNUSED);
    setClientIpAddr(clientNum, "\0");   
    closeSocket(clientNum,RD_WR);
    delClientSockAddr(clientNum);
    pthread_mutex_unlock(&gMutex);
}
void changeClientIpAddr(int32_t cmdNum)
{
    int32_t clientNum;
    c8_t *ipAddr;
    int32_t stat;
    if(gParamNum < 2){
        printf("wrong parameter!!\n");
        printf("%s\n",gCmdLib[cmdNum].example);
        return;
    }
    clientNum = atoi(gCmdParam[1]);
    ipAddr = (c8_t *)gCmdParam[2];
    pthread_mutex_lock(&gMutex);
    stat = getClientFlag(clientNum);
    pthread_mutex_unlock(&gMutex);
    if(UNUSED == stat){
        printf("client num no exist! add client!\n"); 
        return;
    }
    else {
        if(AT_FAILED == changeClientAddr(clientNum, ipAddr)){
            printf("client edit failed!\n");
            return;
        }
    }    
    pthread_mutex_lock(&gMutex);
    setClientIpAddr(clientNum,ipAddr);
    pthread_mutex_unlock(&gMutex); 
    printf("change client Ip address OK!\n");
}
void  showClient(int32_t cmdNum)
{   
    int32_t index;
    int32_t stat;
    if(gParamNum > 0){
        printf("too many parameter!!\n");
        printf("%s\n",gCmdLib[cmdNum].example);
    }
    printf("autoConf:  %s\n",gConfFileName);
    printf("client Parameter:\n");
    for(index = MIN_CLIENT; index < MAX_CLIENT; index ++){
        printf("%d     ",index);
        pthread_mutex_lock(&gMutex);
        stat = getClientFlag(index);
        pthread_mutex_unlock(& gMutex) ;
        if(stat == UNUSED){
            printf("UNUSED\n");
            continue;
        }
        printf("\"%s\"\n",getClientIpAddr(index));
    }
    printf("script Parameter:\n");
    for(index = 0; index < MAX_SCRIPT_THREAD; index ++){
        printf("%d     ",index);
        if(getScriptThreadID(index) == UNUSED){
            printf("UNUSED\n");
            continue;
        }
        printf("\"%s\"\n",gScriptName[index]);
    }
    
}
void  ioevt(int32_t cmdNum)
{
    cmdObj_t cmdObj;
    msgQ_t msgQ_obj;
    ioevt_t ioevtObj;
    int32_t ioevt_num;
    int32_t keyCodeIndex;
    int32_t index;
    const c8_t *ioevt_param[MAX_IOEVT];
    c8_t keyCodes[2*MAX_IOEVT] = "\0";
    pthread_t sendId;
    int32_t ret;
    int32_t clientNum;
    if(gParamNum != 3 ||(strcmp(gCmdParam[3],KEY_PRESS)!=0 && strcmp(gCmdParam[3],KEY_LONG_PRESS)!=0)){
        printf("wrong parameter!!\n");
        printf("%s\n",gCmdLib[cmdNum].example);
        return;
    }
    if(atoi(gCmdParam[1]) > MAX_CLIENT-1 || atoi(gCmdParam[1]) < MIN_CLIENT){
        printf("valid client number:%d - %d\n",MIN_CLIENT,MAX_CLIENT-1);
        return;
    }
    sendId = getSendThreadID();
    if(sendId == ERROR){
        printf("please start test mode first!\n");
        return;
    }
    memset(&cmdObj,0,sizeof(cmdObj_t));
    memset(&msgQ_obj,0,sizeof(msgQ_t));
    memset(&ioevtObj,0,sizeof(ioevt_t));
    memset(keyCodes,0,sizeof(keyCodes));
    if(strcmp(gCmdParam[0],gCmdLib[cmdNum].nickName) == 0){
        snprintf(cmdObj.cmdCode,sizeof(cmdObj.cmdCode),gCmdLib[cmdNum].cmdName);
    }
    else{
        snprintf(cmdObj.cmdCode,sizeof(cmdObj.cmdCode),gCmdParam[0]);
    }
    clientNum = atoi(gCmdParam[1]);
    if(clientNum > MAX_CLIENT || clientNum < MIN_CLIENT){
        return;
    }
    msgQ_obj.clientNum = clientNum;
    snprintf(ioevtObj.action,sizeof(ioevtObj.action),gCmdParam[3]);
    ioevt_num = 0;    
    snprintf(keyCodes,sizeof(keyCodes),gCmdParam[2]);
    ioevt_num = getTokens(keyCodes,C_COMMA,ioevt_param);
    if(ioevt_num >= MAX_IOEVT){      
       return;
    }
    for(index = 0;index <= ioevt_num;index ++){
       if((keyCodeIndex = getKeyCode(gKeyLib,ioevt_param[index])) == FAILED ){
            printf("keycode not found:%s\n",ioevt_param[index]);
            return;
        }
        snprintf(ioevtObj.ioevt[index],MAX_KEY_LENTH,"%s",gKeyLib[keyCodeIndex].keyCode);
    }  
    memcpy(&cmdObj.cmdParamObj.ioevtObj,&ioevtObj,sizeof(ioevt_t));
    memcpy(&msgQ_obj.cmdObj,&cmdObj,sizeof(cmdObj_t)); 
    
    ret = msgsnd(gMsgid, &msgQ_obj, sizeof(msgQ_t),0);
    //0: BLOCK here when 
    //msgQ full or empty inverse:IPC_NOWAIT 
    //set msgFlag as zero; so we can't use gMutex signal to lock this code.cause it may never unlock.    
    if(ret  == FAILED){
        printf("msg ioevt send failed!");
        return;
    }
}
void  call(int32_t cmdNum)
{
    cmdObj_t cmdObj;
    pthread_t sendId;
    msgQ_t msgQ_obj;
    int32_t clientNum;
    int32_t ret;
    if(gParamNum != 2){
        printf("wrong parameter!!\n");
        printf("%s\n",gCmdLib[cmdNum].example);
        return;
    }
    if(atoi(gCmdParam[1]) > MAX_CLIENT-1 || atoi(gCmdParam[1]) < MIN_CLIENT){
        printf("valid client number:%d - %d\n",MIN_CLIENT,MAX_CLIENT-1);
        return;
    }
    sendId = getSendThreadID();
    if(sendId == ERROR){
        printf("please start test mode first!\n");
        return;
    }
    memset(&cmdObj,0,sizeof(cmdObj_t));
    memset(&msgQ_obj,0,sizeof(msgQ_t));
    snprintf(cmdObj.cmdCode,sizeof(cmdObj.cmdCode),gCmdParam[0]);
    clientNum = atoi(gCmdParam[1]);
    if(clientNum > MAX_CLIENT || clientNum < MIN_CLIENT){
        return;
    }
    msgQ_obj.clientNum = clientNum;
    snprintf(cmdObj.cmdParamObj.shared,sizeof(cmdObj.cmdParamObj.shared),gCmdParam[2]); 
    msgQ_obj.cmdObj = cmdObj;  
    ret = msgsnd(gMsgid, &msgQ_obj, sizeof(msgQ_t),0);
    if(ret  == AT_FAILED){
        printf("msg call send failed!");
        return;
    }
}
void  setSysLogLevel(int32_t cmdNum)
{

}
void  start(int32_t cmdNum)
{
    pthread_t sendId = 0;
    pthread_t recId = 0;
    key_t key;
    int32_t msgFileId;
    int32_t ret;
    if(gParamNum < 1 ||gParamNum > 1){
        printf("wrong parameter!!\n");
        printf("%s\n",gCmdLib[cmdNum].example);
        return;
    }
    sendId = getSendThreadID();
    recId = getRecThreadID();
    if(strcmp(gCmdParam[1],PARAM_REC) == 0){
        if(recId != 0){
            printf("start rec mode already!");
            return;
        }
        ret = pthread_create(&recId, NULL, (void *) ret_receive, NULL);  
        if(ret != 0)  
        {  
            printf ("Create rec pthread error!\n");  
            return;
        }  
        setRecThreadID(recId);
        printf("receiving\n");
        return;
    }
    msgFileId = open(MSG_FILE,O_CREAT|O_RDWR,0);
    if(msgFileId == AT_FAILED){
        printf("\nMSG_FILE open failed!\n");
        return;
    }
    close(msgFileId);
    if((key=ftok(MSG_FILE,'a'))==-1){
        printf("Creat Key Error\n");
        return;
    }
    if((gMsgid = msgget(key, IPC_CREAT | 0666))==-1) {
        printf("Creat Message  Error\n");  
        return;
    }
    gScriptParamInit();
    if(strcmp(gCmdParam[1],PARAM_ALL) == 0){
        if(sendId == 0){
            ret = pthread_create(&sendId, NULL, (void *) cmd_send, NULL);  
            if(ret != 0){  
                printf ("Create test pthread error!\n");  
                return;
            }  
            setSendThreadID(sendId);
            printf("testing!\n");
        }
        else {
            printf("start test mode already!\n");
        }
        if(recId == 0){
            ret = pthread_create(&recId, NULL, (void *) ret_receive, NULL);  
            if(ret != 0){  
                printf ("Create rec pthread error!\n");  
                return;
            }  
            setRecThreadID(recId);
            printf("receiving\n");
        }
        else {
            printf("start rec mode already!");
        }

    }
    else if(strcmp(gCmdParam[1],PARAM_TEST) == 0){       
        if(sendId != 0){
            printf("start test mode already!\n");
            return;
        }
        ret = pthread_create(&sendId, NULL, (void *) cmd_send, NULL);  
        if(ret != 0){  
          
            printf ("Create test pthread error!\n");  
            return;
        }  

        setSendThreadID(sendId);
        printf("testing!\n");
    }  
    else{
        printf("wrong parameter!!\n");
        printf("%s\n",gCmdLib[cmdNum].example);
        return;
	}     
}
void  stop(int32_t cmdNum)
{
    pthread_t sendId = 0;
    pthread_t recId = 0;
    pthread_t scId = 0;
    int32_t index;
    sendId = getSendThreadID();
    recId = getRecThreadID();
    if(gParamNum != 1 && gParamNum != 2){
        printf("wrong parameter!!\n");
        printf("%s\n",gCmdLib[cmdNum].example);
        return;
    }
    if(strcmp(gCmdParam[1],PARAM_ALL) == 0){
        if(sendId != ERROR){
	        pthread_kill(sendId,SIGQUIT);
            pthread_join(sendId,0); 
            setSendThreadID(ERROR);
            printf("test mode stoped!\n");
        }
        if(recId != ERROR){
            pthread_kill(recId,SIGQUIT);
            pthread_join(recId,0);
            setRecThreadID(ERROR);
            printf("rec mode stoped!\n");
        }
        
       for(index = 0;index < MAX_SCRIPT_THREAD;index ++){
           scId = getScriptThreadID(index);
           if(scId == ERROR)continue;
           pthread_kill(scId,SIGQUIT);
           pthread_join(scId,0);
           setScriptThreadID(index,ERROR);
       }
        gScriptParamInit();
        printf("script mode stoped!\n"); 
   
   }
    else if(strcmp(gCmdParam[1],PARAM_TEST) == 0){
        if(sendId != ERROR){
	        pthread_kill(sendId,SIGQUIT);
            pthread_join(sendId,0); 
            setSendThreadID(ERROR);
            printf("test mode stoped!\n");
        }
    }
    else if(strcmp(gCmdParam[1],PARAM_REC) == 0){
        if((recId== ERROR)){
            return;
        }
        pthread_kill(recId,SIGQUIT);	  
        pthread_join(recId,0); 
        setRecThreadID(ERROR);
        printf("rec mode stoped!\n");
        return;
    }
    else if(strcmp(gCmdParam[2],PARAM_SC) == 0){
        if(atoi(gCmdParam[1]) < 0 || atoi(gCmdParam[1]) > MAX_SCRIPT_THREAD){
            printf("wrong parameter\n");
            return;
        }
        scId = getScriptThreadID(atoi(gCmdParam[1]));
        if(scId == ERROR){   
            return;
        }
        pthread_kill(scId,SIGQUIT);
        pthread_join(scId,0);
        setScriptThreadID(atoi(gCmdParam[1]),ERROR);
        printf("script %d stoped!\n",atoi(gCmdParam[1])); 
        return;
    }
	else{
        printf("wrong parameter!!\n");
        printf("%s\n",gCmdLib[cmdNum].example);
	} 
}
void quit(int32_t cmdNum)
{       
    pthread_t sendId = 0;
    pthread_t recId = 0;
    int32_t index;
    FILE *filp;
    int32_t ret;
    int32_t i = 0,j = 0;
    c8_t tmp[MAX_TMP_LENTH];
    sendId = getSendThreadID();
    recId = getRecThreadID();
    if(sendId != ERROR){
        pthread_kill(sendId,SIGQUIT);
        pthread_join(sendId,0); 
        printf("test mode stoped!");
    }
    if(recId != 0){
        pthread_kill(recId,SIGQUIT);
        pthread_join(recId,0);
        printf("rec mode stoped!");
    } 
    filp = fopen(TEST_NOTE_FILE,"a+");
    memset(tmp,0,sizeof(tmp)); 
    getSysTime(tmp);
    tmp[strlen(tmp)] = '\0';
    ret = fwrite(&tmp,1,strlen(tmp),filp);
    for(j = 0; j < MAX_CALL_TYPE;j++){
        memset(tmp,0,sizeof(tmp)); 
        if(j == 0)
            snprintf(tmp,sizeof(tmp),"%s\n","doubleTalk:");
        else 
            snprintf(tmp,sizeof(tmp),"%s\n","trebleTalk:");
        ret = fwrite(&tmp,1,strlen(tmp),filp);
        for(i = 0; i <MAX_CLIENT; i ++){
            if(getCountOfSuccessCall(j,i) <= 1){
                sprintf(tmp,"%s%d: %d %s","client",i,getCountOfSuccessCall(j,i),"time\n");
            }
            else {
                sprintf(tmp,"%s%d: %d %s","client",i,getCountOfSuccessCall(j,i),"times\n");
            }        
            tmp[strlen(tmp)] = '\0';
            ret = fwrite(&tmp,1,strlen(tmp),filp);
            if(ret <= 0){
                printf("call note write failed\n");        
            }
        }
    }
    memset(tmp,0,sizeof(tmp)); 
    for(index = 0;index < MAX_SCRIPT_THREAD;index++){
        if(getScriptThreadID(index) == UNUSED) continue;
        if(getCountOfScriptRun(index) <= 1){
            sprintf(tmp,"%s: %d %s\n",gScriptName[index],getCountOfScriptRun(index),"time\n");
        }
        else {
            sprintf(tmp,"%s: %d %s\n",gScriptName[index],getCountOfScriptRun(index),"times\n");
        }
        tmp[strlen(tmp)] = '\0';
        ret = fwrite(&tmp,1,strlen(tmp),filp);
    }
    fclose(filp);
    if(ret <= 0) printf("script note write failed\n");    
    else printf("all ok!exit!!\n");
}
void  runScript(int32_t cmdNum)
{
	int32_t ret;
	int32_t scriptNum;
	pthread_t scId;
	if(getSendThreadID() == ERROR){
	    printf("please start test mode first!\n");
        return;
	}
	if(gParamNum != 2){
        printf("wrong parameter!!\n");
        printf("%s\n",gCmdLib[cmdNum].example);
        return;
    }
	scriptNum = atoi(gCmdParam[1]);
	if(scriptNum > MAX_SCRIPT_THREAD || scriptNum < 0){
        printf("wrong parameter!!\n");
        printf("%s\n",gCmdLib[cmdNum].example);
        return;
	}
	if((scId = getScriptThreadID(scriptNum)) != ERROR){
        printf("script is running\n");
        return;
	}
	memset(gScriptName[scriptNum],0,sizeof(gScriptName[scriptNum]));
	snprintf(gScriptName[scriptNum],sizeof(gScriptName[scriptNum]),gCmdParam[2]);
    ret = pthread_create(&scId, NULL,(void *) scriptCmdProcess , NULL);  
    if(ret != 0) 
    {  
        printf ("Create runScript pthread error!\n");  
        return;
    }
    setScriptThreadID(scriptNum,scId);
 }
 void delay(int32_t cmdNum)
 {
    int32_t delayTime = 0;
    if(gParamNum != 1){
        printf("wrong parameter!!\n");
        printf("%s\n",gCmdLib[cmdNum].example);
        return;
    }
    delayTime = atoi(gCmdParam[1]);
    if(delayTime > 0){
        sleep(delayTime);
    }
 }
 void randomTime(int32_t cmdNum)
 {
    int32_t maxTime = 0,delayTime = 0;
    int32_t minTime = 0;
    if(gParamNum != 2){
        printf("wrong parameter!!\n");
        printf("%s\n",gCmdLib[cmdNum].example);
        return;
    }
    minTime = atoi(gCmdParam[1]);
    maxTime = atoi(gCmdParam[2]);
    if(minTime >= maxTime){
       printf("wrong parameter!!\n");
       printf("%s\n",gCmdLib[cmdNum].example);
       return;
    }
    delayTime = getRandValue(minTime,maxTime); 
    if(delayTime > 0){      
        sleep(delayTime);
    }
 }
 void randomKey(int32_t cmdNum)
 {
    int32_t randomKeyNum = FAILED;
    c8_t cmdRandomKey[MAX_CMD_LEN];
    if(gParamNum != 1){
        printf("wrong parameter!!\n");
        printf("%s\n",gCmdLib[cmdNum].example);
        return;
    }
    randomKeyNum = getRandValue(MIN_KEY_NUM,MAX_KEY_NUM);
    if(randomKeyNum == FAILED){
        printf("randomKey parse error!\n");
        return;
    }
    while(strlen(gKeyLib[randomKeyNum].keyCode) <= 0){
        randomKeyNum = getRandValue(MIN_KEY_NUM,MAX_KEY_NUM);
    }
    memset(cmdRandomKey,0,sizeof(cmdRandomKey));
    sprintf(cmdRandomKey,"%s %s %s %s","ioevt",gCmdParam[1],gKeyLib[randomKeyNum].keyName,"p");
    cmd_proc(cmdRandomKey);
    usleep(50000);
}
 void look(int32_t cmdNum)
 {
    int32_t index;
    int32_t i = 0,j = 0;
    for(j = 0; j < MAX_CALL_TYPE;j++){
        if(j == 0)
            printf("%s\n","doubleTalk:");
        else 
            printf("%s\n","trebleTalk:");
        for(i = 0; i <MAX_CLIENT; i ++){
            if(getCountOfSuccessCall(j,i) <= 1){
                printf("%s%d: %d %s","client",i,getCountOfSuccessCall(j,i),"time\n");
            }
            else {
                printf("%s%d: %d %s","client",i,getCountOfSuccessCall(j,i),"times\n");
            }        
        }
    }
    for(index = 0;index < MAX_SCRIPT_THREAD;index ++){
        if(getCountOfScriptRun(index) != 0){
            printf("Run script file %s:%d\n",gScriptName[index],getCountOfScriptRun(index));
        }
    }
 }

void syslogConf(int32_t cmdNum)
{
    cmdObj_t cmdObj;
    msgQ_t msgQ_obj;
    pthread_t sendId;
    int32_t ret;
    int32_t clientNum;
    if(gParamNum < 2){
        printf("wrong parameter!!\n");
        printf("%s\n",gCmdLib[cmdNum].example);
        return;
    }
    if(atoi(gCmdParam[1]) > MAX_CLIENT-1 || atoi(gCmdParam[1]) < MIN_CLIENT){
        printf("valid client number:%d - %d\n",MIN_CLIENT,MAX_CLIENT-1);
        return;
    }
    sendId = getSendThreadID();
    if(sendId == ERROR){
        printf("please start test mode first!\n");
        return;
    }
    memset(&cmdObj,0,sizeof(cmdObj_t));
    memset(&msgQ_obj,0,sizeof(msgQ_t));
    if(strcmp(gCmdParam[0],gCmdLib[cmdNum].nickName) == 0){
        snprintf(cmdObj.cmdCode,sizeof(cmdObj.cmdCode),gCmdLib[cmdNum].cmdName);
    }
    else{
        snprintf(cmdObj.cmdCode,sizeof(cmdObj.cmdCode),gCmdParam[0]);
    }
    clientNum = atoi(gCmdParam[1]);
    msgQ_obj.clientNum = clientNum;
    if(gParamNum == 3){
        snprintf(cmdObj.cmdParamObj.shared,sizeof(cmdObj.cmdParamObj.shared),\
                 "%s %s %s %d %s %d",gCmdParam[2],gCmdParam[3],STR_CLIENT,\
                 clientNum,STR_SYSLOG_PORT,getSysLogPortNum());
    }
    else {
        snprintf(cmdObj.cmdParamObj.shared,sizeof(cmdObj.cmdParamObj.shared),\
                "%s %s %d %s %d",gCmdParam[2],STR_CLIENT,clientNum,\
                STR_SYSLOG_PORT,getSysLogPortNum());
    }   
    memcpy(&msgQ_obj.cmdObj,&cmdObj,sizeof(cmdObj_t)); 
    ret = msgsnd(gMsgid, &msgQ_obj, sizeof(msgQ_t),0);
    if(ret  == FAILED){
        printf("msg syslog send failed!");
        return;
    }

}
void hideFeatureProc(int32_t cmdNum)
{
    cmdObj_t cmdObj;
    msgQ_t msgQ_obj;
    pthread_t sendId;
    int32_t ret;
    int32_t clientNum;
    if(gParamNum < 2){
        printf("wrong parameter!!\n");
        printf("%s\n",gCmdLib[cmdNum].example);
        return;
    }
    if(atoi(gCmdParam[1]) > MAX_CLIENT-1 || atoi(gCmdParam[1]) < MIN_CLIENT){
        printf("valid client number:%d - %d\n",MIN_CLIENT,MAX_CLIENT-1);
        return;
    }
    sendId = getSendThreadID();
    if(sendId == ERROR){
        printf("please start test mode first!\n");
        return;
    }
    memset(&cmdObj,0,sizeof(cmdObj_t));
    memset(&msgQ_obj,0,sizeof(msgQ_t));
    if(strcmp(gCmdParam[0],gCmdLib[cmdNum].nickName) == 0){
        snprintf(cmdObj.cmdCode,sizeof(cmdObj.cmdCode),gCmdLib[cmdNum].cmdName);
    }
    else{
        snprintf(cmdObj.cmdCode,sizeof(cmdObj.cmdCode),gCmdParam[0]);
    }
    clientNum = atoi(gCmdParam[1]);
    msgQ_obj.clientNum = clientNum;
    snprintf(cmdObj.cmdParamObj.shared,sizeof(cmdObj.cmdParamObj.shared),"%s",gCmdParam[2]);
    memcpy(&msgQ_obj.cmdObj,&cmdObj,sizeof(cmdObj_t)); 
    ret = msgsnd(gMsgid, &msgQ_obj, sizeof(msgQ_t),0);
    if(ret  == FAILED){
        printf("msg syslog send failed!");
        return;
    }
}

void scriptCmdProcess()
{

    int32_t index;
    int32_t loopTime = 0;
    int32_t ret = 0;
    int32_t scriptLineNum;
    int32_t loopStart,loopEnd;
    c8_t parameter[10] = "\0" ;
    const c8_t *p; 
    FILE *scriptFd;
    int32_t scriptNum;
    
	signal(SIGQUIT,sig_handler); 
	scriptNum = atoi(gCmdParam[1]);
	setCountOfScriptRun(scriptNum, 0);
    scriptFd = fopen(gScriptName[scriptNum],"r");
    if(scriptFd == NULL){
        printf("\nscript file open failed!\n");
        goto exit;     
    }
    gScriptParam[scriptNum] = malloc(sizeof(scriptParam));
    if(gScriptParam[scriptNum] == NULL) {
        goto malloc_content_failed;
    }
    memset(gScriptParam[scriptNum],0,sizeof(scriptParam));
    scriptLineNum = cpScriptContentToArray(scriptFd,gScriptParam[scriptNum]->scriptCmd);
    if(scriptLineNum == FAILED){
        printf("cp error\n");       
        goto cp_content_failed;
    }
    index = 0;

    while(1)
    {   
        if(index > scriptLineNum){
            break;
        }    
        if(strncasecmp(gScriptParam[scriptNum]->scriptCmd[index],CMD_LOOP,strlen(CMD_LOOP)) == 0)              
        {
            p = gScriptParam[scriptNum]->scriptCmd[index];         
            p = strstr(p,C_SPACE);           
            p += strlen(C_SPACE);           
            strcpy(parameter,p);
            loopTime = atoi(parameter); 
            if(loopTime > 0)
            {
                index += 1;//skip line loop n
                while(1)
                {                    
                    if(strncasecmp(gScriptParam[scriptNum]->scriptCmd[index],LOOP_START,strlen(LOOP_START)) == 0){
                        //skip line start
                        loopStart = index +1;                                          
                    }
                    if(strncasecmp(gScriptParam[scriptNum]->scriptCmd[index],LOOP_END,strlen(LOOP_END)) == 0){
                        loopEnd = index -1;//line end                       
                        break;
                    }
                    index ++;                 

                }
                for(loopTime; loopTime > 0;loopTime--)   
                {                           
                        if(loopStart == loopEnd){
                            index = loopEnd;   
                            cmd_proc(gScriptParam[scriptNum]->scriptCmd[index]);  
                        }
                        else if(loopStart < loopEnd){
                            for(index = loopStart;index <= loopEnd;index ++){
                                cmd_proc(gScriptParam[scriptNum]->scriptCmd[index]);                                  
                            }  
                        }
                        else{
                              break;
                        }
                        setCountOfScriptRun(scriptNum,getCountOfScriptRun(scriptNum)+1);
                 }
                 index = loopEnd +1; 
            }
             else {
                    printf("please spacify loop times!\n");
                    setScriptThreadID(scriptNum, ERROR);
                    goto script_error;
             }       
              
        }
        else {
            cmd_proc(gScriptParam[scriptNum]->scriptCmd[index]);  
        }
        index += 1;
   } 
   script_error:
   get_content_faild:
   cp_content_failed:
   malloc_content_failed:
        free(gScriptParam[scriptNum]);
   exit:
        setScriptThreadID(scriptNum, ERROR);
        pthread_exit(0);
}
