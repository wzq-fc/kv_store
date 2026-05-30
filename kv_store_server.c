#include "kv_store_server.h"

struct conn conn_list[CONN_SIZE];
//以fd为下标

int epfd;

int set_nonblocking(int fd)
{
    int flags=fcntl(fd,F_GETFL,0);
    if(flags==-1)
    {
        return -1;
    }
    return fcntl(fd,F_SETFL,flags | O_NONBLOCK);
}

int init_server(unsigned short port)
{
    int listenfd=socket(AF_INET,SOCK_STREAM,0);
    if(listenfd<0)
    {
        printf("listenfd create failed");
        return -1;
    }

    struct sockaddr_in addr;
    memset(&addr,0,sizeof(struct sockaddr_in));
    addr.sin_family=AF_INET;
    addr.sin_port=htons(port);
    addr.sin_addr.s_addr=INADDR_ANY;

    //端口复用,防止服务端重启时报错
    int opt = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if(bind(listenfd,(struct sockaddr*)&addr,sizeof(struct sockaddr))<0)
    {
        printf("listenfd bind() failed\n");
        return -1;
    }

    if(listen(listenfd,10)<0)
    {
        printf("listenfd listen() failed");
        return -1;
    }

    set_nonblocking(listenfd);

    return listenfd;
}

//flag 1-->在epoll红黑树中添加一个节点
//flag 0-->在epoll红黑树中修改一个节点
int set_event(int fd,int event,int flag)
{
    if(flag!=0&&flag!=1)
    {
        printf("set_event flag input error\n");
        return -1;
    }
    if(flag)
    {
        struct epoll_event ev;
        ev.data.fd=fd;
        ev.events=event;
        epoll_ctl(epfd,EPOLL_CTL_ADD,fd,&ev);
    }
    else
    {
        struct epoll_event ev;
        ev.data.fd=fd;
        ev.events=event;
        epoll_ctl(epfd,EPOLL_CTL_MOD,fd,&ev);
    }
    return 0;
}

int accept_cb(int fd)
{
    struct sockaddr_in client_addr;
    socklen_t client_len=sizeof(client_addr);

    int clientfd=accept(fd,(struct sockaddr*)&client_addr,&client_len);
    printf("clientfd : %d\n",clientfd);

    set_nonblocking(clientfd);

    conn_list[clientfd].fd=clientfd;
    conn_list[clientfd].r_action.recv_cb=recv_cb;
    conn_list[clientfd].send_cb=send_cb;
    set_event(clientfd,EPOLLIN,1);

    memset(&conn_list[clientfd].rbuffer,0,BUFFER_SIZE);
    conn_list[clientfd].rlength=0;
    memset(&conn_list[clientfd].wbuffer,0,BUFFER_SIZE);
    conn_list[clientfd].wlength=0;

    return 0;
}

int recv_cb(int fd)
{
    memset(&conn_list[fd].rbuffer,0,BUFFER_SIZE);
    conn_list[fd].rlength=0;
    int count=recv(fd,conn_list[fd].rbuffer,BUFFER_SIZE,0);
    if(count==0)
    {
        printf("clientfd : %d disconnected\n",fd);
        close(fd);
        epoll_ctl(epfd,EPOLL_CTL_DEL,fd,NULL);

        //未完成
        return -1;
    }

    conn_list[fd].rlength=count;

    kv_store_request(&conn_list[fd]);

    set_event(fd,EPOLLOUT,0);
    return count;
}

int send_cb(int fd)
{
    kv_store_response(&conn_list[fd]);
    set_event(fd,EPOLLIN,0);
    return 0;
}

int main()
{

    init_kv_store_engine();

    unsigned short port=2000;
    int listenfd=init_server(port);

    epfd=epoll_create(1);
    conn_list[listenfd].fd=listenfd;
    conn_list[listenfd].r_action.accept_cb=accept_cb;
    set_event(listenfd,EPOLLIN,1);

    while(1)
    {
        struct epoll_event events[EPOLL_SIZE]={0};
        int nready=epoll_wait(epfd,events,EPOLL_SIZE,-1);
        for(int i=0;i<nready;i++)
        {
            int conn_fd=events[i].data.fd;
            if(events[i].events & EPOLLIN)
            {
                conn_list[conn_fd].r_action.recv_cb(conn_fd);
            }
            if(events[i].events & EPOLLOUT)
            {
                conn_list[conn_fd].send_cb(conn_fd);
            }
        }
    }
    return 0;
}


