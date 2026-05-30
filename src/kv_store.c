#include "kv_store_server.h"

const char* kv_command[]={
    "SET","GET","DEL","MOD","EXIST"
};

enum{
    KV_STORE_CMD_START=0,
    KV_STORE_CMD_SET=KV_STORE_CMD_START,
    KV_STORE_CMD_GET,
    KV_STORE_CMD_DEL,
    KV_STORE_CMD_MOD,
    KV_STORE_CMD_EXIST,
    KV_STORE_CMD_COUNT
};

const char* response[]={0};

//单例模式
#if 0
kv_store_array_t array={0};
#elif 1
RBTree rb;
#endif

void* kv_store_malloc(size_t size)
{
    return malloc(size);
}

void kv_store_free(void* ptr)
{
    free(ptr);
    return;
}

int kv_store_array_create(kv_store_array_t* instance)
{
    if(!instance)
    {
        return -1;
    }
    if(instance->table)
    {
        printf("table has been created\n");
        return -1;
    }
    instance->table=kv_store_malloc(KV_ARRAY_SIZE * sizeof(kv_store_array_item_t));
    if(!instance->table)
    {
        return -1;
    }
    memset(instance->table,0,KV_ARRAY_SIZE * sizeof(kv_store_array_item_t));

    instance->idx=0;
    instance->total=0;

    return 0;
}

void kv_store_array_destroy(kv_store_array_t* instance)
{
    if(!instance)
    {
        return;
    }

    if(instance->table)
    {
        kv_store_free(instance->table);
    }
    return;
}

int kv_store_array_set(kv_store_array_t* instance,char* key,char* value)
{
    if(instance==NULL||key==NULL||value==NULL)
    {
        return -1;
    }
    if(instance->total>=KV_ARRAY_SIZE)
    {
        return -1;
    }

    for(int i=0;i<instance->total;i++)
    {
        if(instance->table[i].key==NULL)
        {
            continue;
        }
        if(strcmp(instance->table[i].key,key)==0)
        {
            printf("the item which you want to set is already existed\n");
            return -1;
        }
    }

    char* kcopy=kv_store_malloc(strlen(key)+1);
    if(kcopy==NULL)
    {
        return -1;
    }
    memset(kcopy,0,strlen(key)+1);
    strncpy(kcopy,key,strlen(key));

    char* kvalue=kv_store_malloc(strlen(value)+1);
    if(kvalue==NULL)
    {
        return -1;
    }
    memset(kvalue,0,strlen(value)+1);
    strncpy(kvalue,value,strlen(value));

    int i=0;
    for(i=0;i<instance->total;i++)
    {
        if(instance->table[i].key==NULL)
        {
            instance->table[i].key=kcopy;
            instance->table[i].value=kvalue;
            instance->total++;
            return 0;
        }
    }

    if(i>=KV_ARRAY_SIZE)
    {
        printf("array is full\n");
        return -1;
    }
    else if(i==instance->total)
    {
        instance->table[i].key=kcopy;
        instance->table[i].value=kvalue;
        instance->total++;
    }

    return 0;
}

char* kv_store_array_get(kv_store_array_t* instance,char* key)
{
    if(instance==NULL||key==NULL)
    {
        printf("the item you want to get is not exist\n");
        return NULL;
    }
    for(int i=0;i<instance->total;i++)
    {
        if(instance->table[i].key==NULL)
        {
            continue;
        }
        if(strcmp(instance->table[i].key,key)==0)
        {
            return instance->table[i].value;
        }
    }
    printf("the item you want to get is not exist\n");
    return NULL;
}

int kv_store_array_del(kv_store_array_t* instance,char* key)
{
    if(instance==NULL||key==NULL)
    {
        printf("the item you want to delete is not exist\n");
        return -1;
    }
    for(int i=0;i<instance->total;i++)
    {
        if(instance->table[i].key==NULL)
        {
            continue;
        }
        if(strcmp(instance->table[i].key,key)==0)
        {
            kv_store_free(instance->table[i].key);
            instance->table[i].key=NULL;
            kv_store_free(instance->table[i].value);
            instance->table[i].value=NULL;

            return 0;
        }
    }
    printf("the item you want to delete is not exist\n");
    return -1;
}

int kv_store_array_mod(kv_store_array_t* instance,char* key,char* value)
{
    if(instance==NULL||key==NULL||value==NULL)
    {
        printf("the item you want to modify is not exist\n");
        return -1;
    }
    for(int i=0;i<instance->total;i++)
    {
        if(instance->table[i].key==NULL)
        {
            continue;
        }
        if(strcmp(instance->table[i].key,key)==0)
        {
            kv_store_free(instance->table[i].value);

            char* kvalue=kv_store_malloc(strlen(value)+1);
            if(kvalue==NULL)
            {
                return -1;
            }
            memset(kvalue,0,strlen(value)+1);
            strncpy(kvalue,value,strlen(value));
            instance->table[i].value=kvalue;
            return 0;
        }
    }
    printf("the item you want to modify is not exist\n");
    return -1;
}

int kv_store_array_exist(kv_store_array_t* instance,char* key)
{
    if(instance==NULL||key==NULL)
    {
        return -1;
    }
    for(int i=0;i<instance->total;i++)
    {
        if(instance->table[i].key==NULL)
        {
            continue;
        }
        if(strcmp(instance->table[i].key,key)==0)
        {
            return 0;
        }
    }
    return -1;
}

int kv_store_rbtree_create(RBTree* instance)
{
    instance=createRBTree();
    return 0;
}

void kv_store_rbtree_destroy(RBTree* instance)
{
    destroyRBTree(instance);
    return;
}

int kv_store_rbtree_set(RBTree* instance,char* key,char* value)
{
    return insert_rb(instance,key,value);
}

char* kv_store_rbtree_get(RBTree* instance,char* key)
{
    return find_rb(instance,key);
}

int kv_store_rbtree_del(RBTree* instance,char* key)
{
    return remove_rb(instance,key);
}

int kv_store_rbtree_mod(RBTree* instance,char* key,char* value)
{
    return modify_rb(instance,key,value);
}

int kv_store_rbtree_exist(RBTree* instance,char* key)
{
    return exist_rb(instance,key);
}

int kv_store_spilt_token(char* msg,char* tokens[])
{
    if(msg==NULL||tokens==NULL)
    {
        return -1;
    }

    char *p=strchr(msg,'\r');
    if(p) 
    {
        *p='\0';
    }
    p=strchr(msg,'\n');
    if(p) 
    {
        *p='\0';
    }

    int idx=0;
    char* token=strtok(msg," ");

    while(token!=NULL)
    {
        printf("idx : %d %s\n",idx,token);

        tokens[idx++]=token;
        token=strtok(NULL," ");
    }

    return idx;
}

int kv_store_filter_protocol(char** tokens,int count,char* response)
{
    if(tokens[0]==NULL||count<=0||response==NULL)
    {
        return -1;
    }
    int cmd=KV_STORE_CMD_START;
    for(;cmd<KV_STORE_CMD_COUNT;cmd++)
    {
        if(strcmp(tokens[0],kv_command[cmd])==0)
        {
            break;
        }
    }

    int length=0;
    char* key=tokens[1];
    char* value=tokens[2];
#if 0
    switch(cmd)
    {
        case KV_STORE_CMD_SET:
        if(kv_store_array_set(&array,key,value)==0)
        {
            length=sprintf(response,"OK\r\n");
        }
        else
        {
            length=sprintf(response,"ERROR\r\n");
        }
        break;
        case KV_STORE_CMD_GET:
        {
            char* ret=kv_store_array_get(&array,key);
            if(ret)
            {
                length=sprintf(response,"%s\r\n",ret);
            }
            else
            {
                length=sprintf(response,"ERROR\r\n");
            }
        }
        break;
        case KV_STORE_CMD_DEL:
        if(kv_store_array_del(&array,key)==0)
        {
            length=sprintf(response,"OK\r\n");
        }
        else
        {
            length=sprintf(response,"ERROR\r\n");
        }
        break;
        case KV_STORE_CMD_MOD:
        if(kv_store_array_mod(&array,key,value)==0)
        {
            length=sprintf(response,"OK\r\n");
        }   
        else
        {
            length=sprintf(response,"ERROR\r\n");
        }
        break;
        case KV_STORE_CMD_EXIST:
        if(kv_store_array_exist(&array,key)==0)
        {
            length=sprintf(response,"EXIST\r\n");
        }
        else
        {
            length=sprintf(response,"NOT EXIST\r\n");
        }
        break;
    }
#elif 1
    switch(cmd)
    {
        case KV_STORE_CMD_SET:
        if(kv_store_rbtree_set(&rb,key,value)==0)
        {
            length=sprintf(response,"OK\r\n");
        }
        else
        {
            length=sprintf(response,"ERROR\r\n");
        }
        break;
        case KV_STORE_CMD_GET:
        {
            char* ret=kv_store_rbtree_get(&rb,key);
            if(ret)
            {
                length=sprintf(response,"%s\r\n",ret);
            }
            else
            {
                length=sprintf(response,"ERROR\r\n");
            }
        }
        break;
        case KV_STORE_CMD_DEL:
        if(kv_store_rbtree_del(&rb,key)==0)
        {
            length=sprintf(response,"OK\r\n");
        }
        else
        {
            length=sprintf(response,"ERROR\r\n");
        }
        break;
        case KV_STORE_CMD_MOD:
        if(kv_store_rbtree_mod(&rb,key,value)==0)
        {
            length=sprintf(response,"OK\r\n");
        }   
        else
        {
            length=sprintf(response,"ERROR\r\n");
        }
        break;
        case KV_STORE_CMD_EXIST:
        if(kv_store_rbtree_exist(&rb,key)==0)
        {
            length=sprintf(response,"EXIST\r\n");
        }
        else
        {
            length=sprintf(response,"NOT EXIST\r\n");
        }
        break;
    }

#endif

    return length;
}

int kv_store_protocol(char* msg,int length,char* response)
{
    if(msg==NULL||length<=0||response==NULL)
    {
        return -1;
    }
    char* tokens[KV_MAX_TOKENS]={0};
    int count=kv_store_spilt_token(msg,tokens);
    if(count==-1)
    {
        return -1;
    }
    int ret=kv_store_filter_protocol(tokens,count,response);
    if(ret==-1)
    {
        return -1;
    }
    return strlen(response);
}

int kv_store_request(struct conn* c)
{
    memset(&c->wbuffer,0,BUFFER_SIZE);
    c->wlength=0;
    c->wlength=kv_store_protocol(c->rbuffer,c->rlength,c->wbuffer);

    return c->wlength;
}

int kv_store_response(struct conn* c)
{
    return send(c->fd,c->wbuffer,c->wlength,0);
}

int init_kv_store_engine(void)
{
#if 0
    memset(&array,0,sizeof(kv_store_array_t));
    kv_store_array_create(&array);

#elif 1
    memset(&rb,0,sizeof(RBTree));
    kv_store_rbtree_create(&rb);
#endif
    return 0;
}