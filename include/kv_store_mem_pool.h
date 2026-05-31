#ifndef _KV_STORE_MEM_POOL_H_
#define _KV_STORE_MEM_POOL_H_

#include<stdlib.h>
#include<string.h>
#include<stdint.h>

//把数值d调整到临近a的倍数
#define ngx_align(d,a) (((d)+(a-1))&~(a-1))
//把指针p调整到a的临近的倍数
#define ngx_align_ptr(p,a) \
(u_char*)(((uintptr_t)(p)+(uintptr_t)a - 1) & ~(uintptr_t)(a - 1))
//小块内存分配考虑字节对齐时的单位
#define NGX_ALIGNMENT sizeof(unsigned int) //与平台有关，32位为4字节，64位为8字节
//buf缓冲区清零
#define ngx_memzero(buf,n) (void)memset(buf,0,n)

//类型前置声明
typedef struct ngx_pool_s   ngx_pool_t;
typedef struct ngx_pool_large_s     ngx_pool_large_t;

//常量定义
//默认一个物理界面的大小 4k
const int ngx_pagesize = 4096;
//ngx小块内存池可分配的最大空间
const int NGX_MAX_ALLOC_FROM_POOL = ngx_pagesize - 1;
//一个默认的nginx内存池开辟的大小
const int NGX_DEFAULT_POOL_SIZE = 16 * 1024; //16k
//内存池大小按照16字节对齐
const int NGX_POOL_ALIGNMENT = 16;

//类型重定义
typedef unsigned char u_char;
typedef unsigned int ngx_uint_t;

//清理函数(回调函数)的类型
typedef void(*ngx_pool_cleanup_pt)(void* data);
typedef struct ngx_pool_cleanup_s
{
    ngx_pool_cleanup_pt handler;//定义了一个函数指针，保存清理操作的回调函数
    void* data;//传给回调函数的参数
    struct ngx_pool_cleanup_s* next;//所有的cleanup清理操作都被串到了一条链表上
}ngx_pool_cleanup_t;

//大块内存的头部信息
typedef struct ngx_pool_large_s
{
    struct ngx_pool_large_s* next;//所有的大块内存被串到了一条链表上
    void* alloc;//保存分配出去的大块内存的起始地址
}ngx_pool_large_t;

//分配小块内存的内存池的头部数据信息
typedef struct ngx_pool_data_s
{
    u_char* last;//小块内存池可用内存的起始地址
    u_char* end;//小块内存池可用内存的末尾地址
    ngx_pool_t* next;//所有小块内存池被串在了一条链表上
    ngx_uint_t failed;//小块内存池分配内存失败的次数
}ngx_pool_data_t;

//ngx内存池的头部信息和管理成员信息
typedef struct ngx_pool_s
{
    ngx_pool_data_t d;//存储的是当前小块内存池的使用情况
    size_t max;//小块内存和大块内存的分界线
    struct ngx_pool_s* current;//指向第一个提供小块内存分配的小块内存池
    ngx_pool_large_t* large;//指向大块内存(链表)的入口地址
    ngx_pool_cleanup_t* cleanup;//指向所有预置的清理操作回调函数(链表)的入口地址
}ngx_pool_t;

//nginx小块内存池最小的size调整成NGX_POOL_AIGNMENT的临近倍数
//因为大块内存的头部信息是在小块内存当中存的，我们规定大小不得小于两个大块内存头部信息的大小
//否则如果直接分配大块内存的话，可能出现还需要再开辟大块内存来存储头部信息的情况
const int NGX_MIN_POOL_SIZE =
ngx_align((sizeof(ngx_pool_t) + 2 * sizeof(ngx_pool_large_t)),
    NGX_POOL_ALIGNMENT);

//小块内存分配
void* ngx_palloc_small(ngx_pool_t* pool,size_t size, ngx_uint_t align);
//大块内存分配
void* ngx_palloc_large(ngx_pool_t* pool,size_t size);
//分配新的小块内存池
void* ngx_palloc_block(ngx_pool_t* pool,size_t size);
//创建指定大小的内存池，但是小块内存池不超过一个页面的大小
ngx_pool_t* ngx_create_pool(size_t size);
//考虑内存字节对齐，从内存池申请size字节大小的内存
void* ngx_palloc(ngx_pool_t* pool,size_t size);
//和上面作用一样，但是不考虑内存对齐
void* ngx_pnalloc(ngx_pool_t* pool,size_t size);
//调用的是ngx_palloc实现内存分配，但是会初始化0
void* ngx_pcalloc(ngx_pool_t* pool,size_t size);
//释放大块内存
void ngx_pfree(ngx_pool_t* pool,void* p);
//内存重置函数
void ngx_reset_pool(ngx_pool_t* pool);
//内存池的销毁函数
void ngx_destroy_pool(ngx_pool_t* pool);
//添加回调清理操作函数
ngx_pool_cleanup_t* ngx_pool_cleanup_add(ngx_pool_t* pool,size_t size);

#endif