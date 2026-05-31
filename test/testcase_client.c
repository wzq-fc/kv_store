#include<stdio.h>
#include<string.h>
#include<stdlib.h>

#include<unistd.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<errno.h>

#define MAX_MSG_LENGTH 1024

int connect_server(const char* ip,unsigned short port)
{
    int connfd=socket(AF_INET,SOCK_STREAM,0);

    struct sockaddr_in addr;
    memset(&addr,0,sizeof(struct sockaddr_in));
    addr.sin_family=AF_INET;
    addr.sin_port=htons(port);
    addr.sin_addr.s_addr=inet_addr(ip);

    if(connect(connfd,(struct sockaddr*)&addr,sizeof(struct sockaddr_in))!=0)
    {
        perror("connect");
        return -1;
    }

    return connfd;
}

int send_msg(int connfd,char* msg,int length)
{
    int res=send(connfd,msg,length,0);
    if(res<0)
    {
        perror("send");
        exit(1);
    }

    return res;
}

int recv_msg(int connfd,char* msg,int length)
{
    int ret=recv(connfd,msg,length-1,0);
    if(ret<=0)
    {
        perror("recv");
        exit(1);
    }

    return ret;
}

void testcase(int connfd,char* msg,char* pattern,char* casename)
{
    if(msg==NULL||pattern==NULL||casename==NULL)
    {
        return;
    }
    send_msg(connfd,msg,strlen(msg));

    char result[MAX_MSG_LENGTH]={0};
    recv_msg(connfd,result,MAX_MSG_LENGTH);

    if(strcmp(result,pattern)==0)
    {
        printf("==> PASS %s\n",casename);
    }
    else
    {
        printf("==> FAILED %s, '%s' != '%s'\n",casename,result,pattern);
    }
    return;
}

void testcase_1(int connfd)
{
    testcase(connfd,"SET TEACHER FXC","OK\r\n","SET-TEACHER");
    testcase(connfd,"GET TEACHER","FXC\r\n","GET-TEACHER");
    testcase(connfd,"MOD TEACHER MYH","OK\r\n","MOD-TEACHER");
    testcase(connfd,"GET TEACHER","MYH\r\n","GET-TEACHER");
    testcase(connfd,"EXIST TEACHER","EXIST\r\n","EXIST-TEACHER");
    testcase(connfd,"DEL TEACHER","OK\r\n","DEL-TEACHER");
    testcase(connfd,"GET TEACHER","ERROR\r\n","GET-TEACHER");
    testcase(connfd,"MOD TEACHER FXC","ERROR\r\n","MOD-TEACHER");
    testcase(connfd,"EXIST TEACHER","NOT EXIST\r\n","EXIST-TEACHER");
}

void testcase_10W(int connfd)
{
    int count=100000;
    for(int i=0;i<count;i++)
    {
        testcase(connfd,"SET TEACHER FXC","OK\r\n","SET-TEACHER");
        testcase(connfd,"GET TEACHER","FXC\r\n","GET-TEACHER");
        testcase(connfd,"MOD TEACHER MYH","OK\r\n","MOD-TEACHER");
        testcase(connfd,"GET TEACHER","MYH\r\n","GET-TEACHER");
        testcase(connfd,"EXIST TEACHER","EXIST\r\n","EXIST-TEACHER");
        testcase(connfd,"DEL TEACHER","OK\r\n","DEL-TEACHER");
        testcase(connfd,"GET TEACHER","ERROR\r\n","GET-TEACHER");
        testcase(connfd,"MOD TEACHER FXC","ERROR\r\n","MOD-TEACHER");
        testcase(connfd,"EXIST TEACHER","NOT EXIST\r\n","EXIST-TEACHER");
    }
}

int main(int argc,char* argv[])
{
    if(argc!=3)
    {
        printf("argc error\n");
        return -1;
    }

    char* ip=argv[1];
    unsigned short port=atoi(argv[2]);

    int connfd=connect_server(ip,port);

    testcase_10W(connfd);

    return 0;
}