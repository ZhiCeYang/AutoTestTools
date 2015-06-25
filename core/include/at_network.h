#ifndef __AT_NETWORK_H__
#define __AT_NETWORK_H__
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "at_common.h"
#include "stdio.h"

#define MAX_CLIENT      10
#define MIN_CLIENT      0
#define MAX_IP_LENTH    20
#define MAX_TMP_LENTH   60
#define MAX_PATH_LENTH  50
#define MAX_CALL_TYPE   2
#define SERVER_PORT     5000
#define SYSLOG_PORT     5001

#define   S_ALIVE   1
#define  S_ERROR    0

#define USED        1
#define UNUSED      0

void setClientIpAddr(int32_t clientNum,c8_t *ipAddr);
c8_t *getClientIpAddr(int32_t clientNum);
void setClientFlag(int32_t clientNum,int32_t flag);
int32_t getClientFlag(int32_t clientNum);
void setClientSockAddr(int32_t clientNum,struct sockaddr_in *addr);
struct sockaddr_in getClientSockAddr(int32_t clientNum);
void delClientSockAddr(int32_t clientNum);
void closeSocket(int32_t clientNum,int32_t flag);
atstat_e client_config(void);
atstat_e network_config(int32_t cNum, c8_t  *ipAddr);
atstat_e changeClientAddr(int32_t cNum, c8_t *ipAddr);
void clear_stdin_buff(void);
atstat_e get_cNumAndIpAddr(int32_t *clientNum, c8_t *ipAddr);
atbool_e isValidIp(c8_t * inStr);
#endif
