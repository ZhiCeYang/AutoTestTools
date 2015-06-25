#include <stdlib.h>
#include <stdio.h>
#include <String.h>
#include <sys/stat.h>
#include <memory.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <fcntl.h>  
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "at_common.h"
#include "autoTest.h"
#include "at_network.h"
#include "at_cmdProc.h"

LOCAL pthread_t thr_cmdSend_id = 0;
LOCAL pthread_t thr_retReceive_id = 0;
LOCAL pthread_t thr_script_id[MAX_SCRIPT_THREAD];

IMPORT int32_t syslogSockFd;
IMPORT struct sockaddr_in syslogServer_addr;
IMPORT struct sockaddr_in syslogClient_addr;
LOCAL int32_t gSyslogPort = ERROR;
IMPORT int32_t gMsgid;
pthread_mutex_t gMutex;

LOCAL char gLogStrLib[MAX_LOG_INDEX][MAX_LOG_STR_LEN] = {
    [LOG_SUSPEND]           = {"suspended"},
    [LOG_TALK_BUILD]        = {"doubleTalk"},
    [LOG_TALK_CONFERENCE]   = {"trebleTalk"}
};

int32_t main(int32_t argc, c8_t argv[])
{  
    char* input, shell_prompt[100];
	pthread_mutex_init (&gMutex,NULL);
	time_t tmpTime;
    time (&tmpTime);
    srand((unsigned)GetTickCount());
	printf("input h/help  for help\n");
	initialize_readline();
    // Configure readline to auto-complete paths when the tab key is hit.
    //rl_bind_key('\t', rl_complete); //no need to bind because  it is a default value.
	snprintf(shell_prompt, sizeof(shell_prompt), "%s:%s $ ", getenv("USER"), getcwd(NULL, 1024));
    while(1)
	{
        // Display prompt and read input (NB: input must be freed after use)...
        input = readline(shell_prompt);
        // Check for EOF.
        if (!input)
            break;
        // Add input to history.
		if(strlen(input))
			add_history(input);	
	    else continue;
        // Do stuff...
        cmd_proc(input);/* user cmd */	
        // Free input.
        if(!strncmp(input,"quit",strlen("quit"))){
			free(input);
			exit(0);
		}
        free(input);
    }
   

}

void cmd_send(void)  
{       
    cmdObj_t cmdObj;
    msgQ_t msgQ_obj;
    int32_t addr_len;
    int32_t size;
    int32_t cNum,socketFd;
    int32_t useFlag;
    struct sockaddr_in clientAddr;
    int32_t index;
    signal(SIGQUIT,sig_handler);
    addr_len = sizeof(struct sockaddr_in);
    while(1)
    {   
        memset(&msgQ_obj,0,sizeof(msgQ_t));
        memset(&cmdObj,0,sizeof(cmdObj_t));
        memset(&clientAddr,0,sizeof(struct sockaddr_in));
        size = msgrcv(gMsgid, &msgQ_obj, sizeof(msgQ_t), 0,IPC_NOWAIT);
        if(size == sizeof(msgQ_t)){
            memcpy(&cmdObj,&msgQ_obj.cmdObj,sizeof(cmdObj_t));  
            cNum = msgQ_obj.clientNum;
            pthread_mutex_lock(&gMutex);            
            socketFd = getSocket(cNum);         
            clientAddr = getClientSockAddr(cNum);
            useFlag = getClientFlag(cNum);
            pthread_mutex_unlock(&gMutex) ;
            if(useFlag == UNUSED){
                continue;
            }
            size = sendto(socketFd,&cmdObj,sizeof(cmdObj_t),0,\
                         (const struct sockaddr *)&clientAddr,addr_len);            
        }
    }
} 
void ret_receive(void)
{   
    int32_t ret;
    c8_t *p;
    int32_t count;
    int32_t addr_len;
    FILE *logFd;
    c8_t logPath[MAX_PATH_LENTH];
    c8_t tmp[MAX_PATH_LENTH];
    c8_t buf[MAX_LOG_SIZE];
    c8_t log[MAX_BUFF_SIZE];
    signal(SIGQUIT,sig_handler);
    if(FAILED ==  syslogServerInit()){  
            pthread_exit(0);
    }
    addr_len = sizeof(struct sockaddr_in);
    while(1)
    {
        memset(tmp,0,sizeof(tmp));
        memset(buf,0,sizeof(buf));
        memset(log,0,sizeof(log));
        memset(logPath,0,sizeof(logPath));
        ret = recvfrom(syslogSockFd,buf,sizeof(buf),MSG_DONTWAIT,\
                       (struct sockaddr *)&syslogClient_addr,&addr_len);
        if((ret > 0) && (strlen(buf) > 0)){
            /*process syslog*/     
           buf[sizeof(buf) -1] = '\0';            
           if((p = strstr(buf,CMD_RECORD)) != NULL){
                sprintf(logPath,"%s/%s_%s",LOG_PATH,inet_ntoa(syslogClient_addr.sin_addr),CMD_RECORD);
                logPath[strlen(logPath)] = '\0';
                p += strlen(CMD_RECORD);                
                snprintf(log,MAX_BUFF_SIZE,"%s",p);
           }
           else{
               sprintf(logPath,"%s/%s",LOG_PATH,inet_ntoa(syslogClient_addr.sin_addr));
               logPath[strlen(logPath)] = '\0';
               getSysTime(tmp);
               tmp[strlen(tmp)-1] = '\0';
               sprintf(log,"%s: %s",tmp,buf);
           }
           logFd = fopen(logPath,"a+");
           if(logFd == 0){
                printf("file %s open failed\n",logPath);
                continue;
           }
           log[strlen(log)] = '\n';
           log[strlen(log)+1] = '\0';
           log[sizeof(log)] = '\0';
           ret = fwrite(log,1,strlen(log),logFd);
           if(ret < strlen(log)){
                printf("log write error\n");
           }
           if(logFd){
                fclose(logFd);
           }
           syslogProcess(log);
        }  
    }
}
LOCAL int32_t syslogServerInit()
{
    int32_t portnumber = SYSLOG_PORT;
    int32_t i = 0,tmpPort = 0;
    syslogSockFd = 0;
    
    if((syslogSockFd=socket(AF_INET,SOCK_DGRAM,0))==AT_ERROR)
    { 
        printf("Socket create error\n"); 
        return FAILED; 
    } 	 
    for(i = 0;i < 20; i ++){
        portnumber += i;
        memset(&syslogServer_addr,0,sizeof(struct sockaddr_in)); 
        memset(&syslogClient_addr,0,sizeof(struct sockaddr_in)); 
        syslogServer_addr.sin_family = AF_INET; 	
        syslogServer_addr.sin_addr.s_addr = htonl(INADDR_ANY); 	
        syslogServer_addr.sin_port = htons(portnumber); 	
        if(AT_OK == bind(syslogSockFd,(struct sockaddr *)(&syslogServer_addr),\
                      sizeof(struct sockaddr_in))){ 
                gSyslogPort = portnumber;
                break;
        } 	
    }
    return OK;

}

int32_t getSysLogPortNum()
{
    return gSyslogPort;
}
LOCAL int32_t getLogMsgIssueId(char *log)
{
    int i = 0;
    char *p = NULL;
    if (log == NULL) return;
    for(;i < MAX_LOG_INDEX; i ++){
        p = strstr(log,gLogStrLib[i]);        
        if(p == NULL)
            continue;
        else 
            return i;
    }
    return RET_ERROR;
}
LOCAL int32_t getLogClientNum(char *log)
{
    int32_t ret = AT_ERROR;
    char *p = NULL;
    int32_t i = 0;
    char tmpBuff[3] = {0};
    if (log == NULL) return AT_ERROR;
    p = strstr(log,STR_CLIENT);
    if(p == NULL) return AT_ERROR;
    p += strlen(STR_CLIENT) + 1;
	while(*p != ' '){	
	    *(tmpBuff+i) = *p;	
	    i ++;	
	    p ++;  	
	}
    ret = atoi(tmpBuff);
    return ret;   
}
LOCAL void syslogProcess(char *log)
{   
    int32_t issueId = RET_ERROR;
    int32_t clientNum = RET_ERROR;
    int32_t retNum = RET_ERROR;
    if (log == NULL) return ;
    issueId = getLogMsgIssueId(log);
    if(issueId == RET_ERROR) return;
    clientNum = getLogClientNum(log);
    if(clientNum > MIN_CLIENT && clientNum < MAX_CLIENT){
        switch(issueId){
            case LOG_SUSPEND:            
                {           
                    pthread_mutex_lock(&gMutex);
                    setClientFlag(clientNum,UNUSED);
                    setClientIpAddr(clientNum, "\0");   
                    closeSocket(clientNum,RD_WR);
                    delClientSockAddr(clientNum);
                    pthread_mutex_unlock(&gMutex);  
                }
                break;
            case LOG_TALK_BUILD:
                retNum = getCountOfSuccessCall(0,clientNum);
                setCountOfSuccessCall(0,clientNum,retNum + 1);
                break;
            case LOG_TALK_CONFERENCE:
                retNum = getCountOfSuccessCall(1,clientNum);
                setCountOfSuccessCall(1,clientNum,retNum + 1);            
                break;
            default :
                break;
              
        }
    }        
}
void sig_handler(int32_t sig)
{
    if(sig == SIGQUIT){
        printf("sub thread exit\n");
        pthread_exit(0);//线程返回，带参数
    }
}
pthread_t getSendThreadID(void)
{   
    return thr_cmdSend_id;
}
void setSendThreadID(pthread_t id)
{   
   if(id < 0) return;
   thr_cmdSend_id = id;
}
pthread_t getRecThreadID(void)
{
    return thr_retReceive_id;
}
void setRecThreadID(pthread_t id)
{
    if(id < 0) return;
    thr_retReceive_id = id;
}
pthread_t getScriptThreadID(int32_t scriptNum)
{
    return thr_script_id[scriptNum];
}
void setScriptThreadID(int32_t scriptNum,pthread_t id)
{
    if(id < 0 || scriptNum < MIN_CLIENT || scriptNum > MAX_CLIENT) return;
    thr_script_id[scriptNum] = id;
}
void getSysTime(c8_t *p_time)
{
     time_t timeObj;
     time (&timeObj); 
     sprintf(p_time,"%s",ctime(&timeObj));
}


