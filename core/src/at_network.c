#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <stdio.h>
#include <stdlib.h>
#include <String.h>
#include <sys/stat.h>
#include <memory.h>
#include <unistd.h>
#include <errno.h>
#include "at_network.h"
#include "at_common.h"
#include "autotest.h"

LOCAL int32_t gSockets[MAX_CLIENT] = {0};
LOCAL c8_t gClientIpAddr[MAX_CLIENT][MAX_IP_LENTH] = {"\0"};
LOCAL struct sockaddr_in clientSockAddr[MAX_CLIENT];
struct sockaddr_in syslogServer_addr;
struct sockaddr_in syslogClient_addr;
int32_t syslogSockFd;
LOCAL int32_t gClientFlag[MAX_CLIENT] = {UNUSED};
void setSocket(int32_t clientNum,int32_t socket)
{
    if(clientNum < MIN_CLIENT || clientNum > MAX_CLIENT-1 \
            || socket == 0 || gClientFlag[clientNum] == USED){
        return;
    }
    gSockets[clientNum] = socket;
}
int32_t getSocket(int32_t clientNum)
{
    if(clientNum < MIN_CLIENT || clientNum > MAX_CLIENT-1 \
            || gClientFlag[clientNum] == UNUSED){
        return ERROR;
    }
    return gSockets[clientNum];
}
void setClientIpAddr(int32_t clientNum,c8_t *ipAddr)
{
    if(clientNum < MIN_CLIENT || clientNum > MAX_CLIENT-1 ||\
            ipAddr == NULL){
        return;
    }
    strncpy(gClientIpAddr[clientNum],ipAddr,strlen(ipAddr));
    gClientIpAddr[clientNum][strlen(ipAddr)] = '\0';
}
c8_t *getClientIpAddr(int32_t clientNum)
{
    if(clientNum < MIN_CLIENT || clientNum > MAX_CLIENT-1 ){
        return ERROR;
    }
    return gClientIpAddr[clientNum];
}
void setClientFlag(int32_t clientNum,int32_t flag)
{
    if(clientNum < MIN_CLIENT || clientNum > MAX_CLIENT-1){
        return;
    }
    gClientFlag[clientNum] = flag;
}
int32_t getClientFlag(int32_t clientNum)
{   
    
    if(clientNum < MIN_CLIENT || clientNum > MAX_CLIENT-1){
        return ERROR;
    }
    return gClientFlag[clientNum];
}
void setClientSockAddr(int32_t clientNum,struct sockaddr_in *addr)
{
    if(clientNum < MIN_CLIENT || clientNum > MAX_CLIENT-1 || addr == NULL){
        return;
    }
    memcpy(&clientSockAddr[clientNum],addr,sizeof(struct sockaddr_in));
}
struct sockaddr_in getClientSockAddr(int32_t clientNum)
{
    if(clientNum < MIN_CLIENT || clientNum > MAX_CLIENT-1 ){
        return ;
    }
    return clientSockAddr[clientNum];
}
void delClientSockAddr(int32_t clientNum)
{
    if(clientNum < MIN_CLIENT || clientNum > MAX_CLIENT-1 ){
        return ;
    }
    memset(&clientSockAddr[clientNum],0,sizeof(struct sockaddr_in));
}
void closeSocket(int32_t clientNum,int32_t flag)
{   
    
    if(gSockets[clientNum] != ERROR){
        shutdown(gSockets[clientNum],flag);
        close(gSockets[clientNum]);
        printf("client deleted!\n");
    }
}
atstat_e network_config(int32_t cNum, c8_t  *ipAddr)
{
    struct sockaddr_in clientAddr;
    int32_t portnumber;
    int32_t addr_len;
    atbool_e ipCheck;
    if(cNum < MIN_CLIENT|| cNum > MAX_CLIENT-1 || ipAddr == NULL){
        printf("invalid parameter!\n");
        return AT_FAILED;
    }
    ipCheck = isValidIp(ipAddr);
    if(ipCheck == AT_FALSE){
        return AT_FAILED;
    }
	if(gClientFlag[cNum] == USED){
        printf("client using\n");
        return AT_FAILED;
    }
	portnumber = SERVER_PORT;
    bzero(&clientAddr,sizeof(struct sockaddr_in));
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_port = htons(portnumber);
    if(inet_aton(ipAddr,&clientAddr.sin_addr)<0) 
    { 
        fprintf(stderr,"Ip error:%s\n",strerror(errno)); 
        return AT_FAILED;
    } 
    
	addr_len = sizeof(struct sockaddr_in);
    gSockets[cNum] = socket(AF_INET,SOCK_DGRAM,0);
    if(gSockets[cNum] == FAILED){
        printf("create socket failed!\n");
        return AT_FAILED;
    } 
	setClientSockAddr(cNum, &clientAddr);
	return AT_OK;
 
}
atstat_e changeClientAddr(int32_t cNum, c8_t *ipAddr)
{
    if(AT_FAILED == network_config(cNum,ipAddr)){
         return AT_FAILED;
    }
    return AT_OK;
}
void clear_stdin_buff(void)
{
    while(getchar() != '\n'){
        ;
    }
}
atstat_e get_cNumAndIpAddr(int32_t *clientNum, c8_t *ipAddr)
{
    int32_t ret;
    printf("please input the client num\n");
    printf("%d-%d is valid:",MIN_CLIENT,MAX_CLIENT -1);
    ret = scanf("%d",clientNum);
    if(ret != OK){                
        printf("wrong client num!\n");
        clear_stdin_buff();
        return AT_FAILED;
    }
    printf("Num:%d\n",*clientNum);
    printf("please input the IP address\n");
    clear_stdin_buff();
    printf("ipAddr:");
    fgets(ipAddr, MAX_IP_LENTH, stdin);
    if(strlen(ipAddr) <= 0){
        printf("wrong client IP address\n");
        strcpy(ipAddr,"localhost");
        return AT_FAILED;
    }
    ipAddr[strlen(ipAddr)-1] = '\0';
    printf("ip:%s\n",ipAddr);
    return AT_OK;
}
atbool_e isValidIp(c8_t* inStr) {	
	struct sockaddr_in sa;
	int32_t result = inet_pton(AF_INET, inStr, &(sa.sin_addr));
	if (result == 0) {
		printf("Incorrect network address! i.e. ip=192.168.10.1\n\n");
		return AT_FALSE;
	}
	
	return AT_TRUE;
}
