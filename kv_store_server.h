#ifndef KV_STORE_SERVER_H
#define KV_STORE_SERVER_H

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include<unistd.h>
#include<sys/socket.h>
#include<sys/epoll.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<fcntl.h>
#include<errno.h>

#define BUFFER_SIZE 1024
#define CONN_SIZE 1024
#define EPOLL_SIZE 1024
#define KV_MAX_TOKENS 128
#define KV_ARRAY_SIZE 1024

typedef int(*RCALLBACK)(int fd);
typedef int(*kv_msg_handler)(char* msg,int length,char* response);

struct conn
{
    int fd;
    char rbuffer[BUFFER_SIZE];
    int rlength;
    char wbuffer[BUFFER_SIZE];
    int wlength;

    RCALLBACK send_cb;

    union
    {
        RCALLBACK accept_cb;
        RCALLBACK recv_cb;
    }r_action;

    //int status;
};

typedef struct kv_store_array_item_s
{
    char* key;
    char* value;
}kv_store_array_item_t;

typedef struct kv_store_array_s
{
    kv_store_array_item_t* table;
    int idx;
    int total;
}kv_store_array_t;

//节点的颜色
typedef enum 
{
    BLACK=0,
    RED
}Color;

//节点的类型
typedef struct Node
{
    char* key;
    char* value;
    struct Node* left_;
    struct Node* right_;
    struct Node* parent_;
    Color color_;
}Node;

//红黑树类型
typedef struct RBTree
{
    Node* root_;
    int size;
}RBTree;

Node* createNode(const char* key,const char* value,Node* left,Node* right,Node* parent,Color color);
RBTree* createRBTree();                  
void destroyRBTree(RBTree* rb);
int insert_rb(RBTree* rb,const char* key,const char* value);
int remove_rb(RBTree* rb,const char* key);
char* find_rb(RBTree* rb,const char* key);
int modify_rb(RBTree* rb,const char* key,const char* value);
int exist_rb(RBTree* rb,const char* key);

int set_nonblocking(int fd);
int init_server(unsigned short port);
int set_event(int fd,int event,int flag);
int accept_cb(int fd);
int recv_cb(int fd);
int send_cb(int fd);

void* kv_store_malloc(size_t size);
void kv_store_free(void* ptr);
int kv_store_array_create(kv_store_array_t* instance);
void kv_store_array_destroy(kv_store_array_t* instance);
int kv_store_rbtree_create(RBTree* instance);
void kv_store_rbtree_destroy(RBTree* instance);

int kv_store_spilt_token(char* msg,char* tokens[]);
int kv_store_filter_protocol(char** tokens,int count,char* response);
int kv_store_protocol(char* msg,int length,char* response);

int kv_store_request(struct conn* c);
int kv_store_response(struct conn* c);

int kv_store_array_set(kv_store_array_t* instance,char* key,char* value);
char* kv_store_array_get(kv_store_array_t* instance,char* key);
int kv_store_array_del(kv_store_array_t* instance,char* key);
int kv_store_array_mod(kv_store_array_t* instance,char* key,char* value);
int kv_store_array_exist(kv_store_array_t* instance,char* key);

int kv_store_rbtree_set(RBTree* instance,char* key,char* value);
char* kv_store_rbtree_get(RBTree* instance,char* key);
int kv_store_rbtree_del(RBTree* instance,char* key);
int kv_store_rbtree_mod(RBTree* instance,char* key,char* value);
int kv_store_rbtree_exist(RBTree* instance,char* key);

int init_kv_store_engine(void);

#endif