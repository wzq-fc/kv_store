#include "kv_store_mem_pool.h"

//创建指定大小的内存池，但是小块内存池不超过一个页面的大小
ngx_pool_t* ngx_create_pool(size_t size)
{
    ngx_pool_t* p;
    p = (ngx_pool_t*)malloc(size);
    if (p == NULL)
    {
        return NULL;
    }
    p->d.last = (u_char*)p + sizeof(ngx_pool_t);
    p->d.end = (u_char*)p + size;
    p->d.next = NULL;
    p->d.failed = 0;
    size = size - sizeof(ngx_pool_t);
    p->max = (size < NGX_MAX_ALLOC_FROM_POOL) ? size : NGX_MAX_ALLOC_FROM_POOL;
    p->current = p;
    p->large = NULL;
    p->cleanup = NULL;
    
    return p;
}

//考虑内存字节对齐，从内存池申请size字节大小的内存
void* ngx_palloc(ngx_pool_t* pool,size_t size)
{
    if (size <= pool->max)
    {
        return ngx_palloc_small(pool,size, 1);
    }
    return ngx_palloc_large(pool,size);
}

//和上面作用一样，但是不考虑内存对齐
void* ngx_pnalloc(ngx_pool_t* pool,size_t size)
{
    if (size <= pool->max)
    {
        return ngx_palloc_small(pool,size, 0);
    }
    return ngx_palloc_large(pool,size);
}

//调用的是ngx_palloc实现内存分配，但是会初始化0
void* ngx_pcalloc(ngx_pool_t* pool,size_t size)
{
    void* p;
    p = ngx_palloc(pool,size);
    if (p)
    {
        ngx_memzero(p, size);
    }
    return p;
}

//小块内存分配
void* ngx_palloc_small(ngx_pool_t* pool,size_t size, ngx_uint_t align)
{
    u_char* m;
    ngx_pool_t* p;
    p = pool->current;
    do
    {
        m = p->d.last;
        //内存对齐操作
        if (align)
        {
            m = ngx_align_ptr(m, NGX_ALIGNMENT);
        }
        if ((size_t)(p->d.end - m) >= size)
        {
            p->d.last = m + size;
            return m;
        }
        p = p->d.next;
    } while (p);
    return ngx_palloc_block(pool,size);
}

//分配新的小块内存池
void* ngx_palloc_block(ngx_pool_t* pool,size_t size)
{
    u_char* m;
    size_t psize;
    ngx_pool_t* p, * newpool;
    psize = (size_t)(pool->d.end - (u_char*)pool);
    m = (u_char*)malloc(psize);
    if (m == NULL)
    {
        return NULL;
    }
    newpool = (ngx_pool_t*)m;
    newpool->d.end = m + psize;
    newpool->d.next = NULL;
    newpool->d.failed = 0;
    m += sizeof(ngx_pool_data_t);
    m = ngx_align_ptr(m, NGX_ALIGNMENT);
    newpool->d.last = m + size;
    for (p = pool->current; p->d.next; p = p->d.next)
    {
        if (p->d.failed++ > 4)
        {
            pool->current = p->d.next;
        }
    }
    p->d.next = newpool;
    return m;
}

//大块内存分配
void* ngx_palloc_large(ngx_pool_t* pool,size_t size)
{
    void* p;
    ngx_uint_t n;
    ngx_pool_large_t* large;

    p = malloc(size);
    if (p == NULL)
    {
        return NULL;
    }

    n = 0;

    for (large = pool->large; large; large = large->next)
    {
        if (large->alloc == NULL)
        {
            large->alloc = p;
            return p;
        }
        if (n++ > 3)
        {
            break;
        }
    }

    large = (ngx_pool_large_t*)ngx_palloc_small(pool,sizeof(ngx_pool_large_t), 1);
    if (large == NULL)
    {
        free(p);
        return NULL;
    }

    large->alloc = p;
    large->next = pool->large;
    pool->large = large;
    return p;
}

//释放大块内存
void ngx_pfree(ngx_pool_t* pool,void* p)
{
    ngx_pool_large_t* l;
    for (l = pool->large; l; l = l->next)
    {
        if (l->alloc == p)
        {
            free(l->alloc);
            l->alloc = NULL;
            return;
        }
    }
    return;
}

//内存重置函数
void ngx_reset_pool(ngx_pool_t* pool)
{
    ngx_pool_t* p;
    ngx_pool_large_t* l;
    for (l = pool->large; l; l = l->next)
    {
        if (l -> alloc)
        {
            free(l->alloc);
        }
    }

    //处理第一块内存池
    p = pool;
    p->d.last = (u_char*)p + sizeof(ngx_pool_t);
    p->d.failed = 0;

    //第二块内存池开始循环到最后一个内存池
    for (p = p->d.next; p; p = p->d.next)
    {
        p->d.last = (u_char*)p + sizeof(ngx_pool_data_t);
        p->d.failed = 0;
    }
    pool->current = pool;
    pool->large = NULL;
}
//内存池的销毁函数
void ngx_destroy_pool(ngx_pool_t* pool)
{
    ngx_pool_t* p, * n;
    ngx_pool_large_t* l;
    ngx_pool_cleanup_t* c;

    for (c = pool->cleanup; c; c = c->next)
    {
        if (c->handler)
        {
            c->handler(c->data);
        }
    }
    for (l = pool->large; l; l = l->next)
    {
        if (l->alloc)
        {
            free(l->alloc);
        }
    }
    for (p = pool, n = p->d.next;; p = n, n = n->d.next)
    {
        free(p);
        if (n == NULL)
        {
            break;
        }
    }
}
//添加回调清理操作函数
ngx_pool_cleanup_t* ngx_pool_cleanup_add(ngx_pool_t* pool,size_t size)
{
    ngx_pool_cleanup_t* c;
    c = (ngx_pool_cleanup_t*)ngx_palloc(pool,sizeof(ngx_pool_cleanup_t));
    if (c == NULL)
    {
        return NULL;
    }
    if (size)
    {
        c->data = ngx_palloc(pool,size);
        if (c->data == NULL)
        {
            return NULL;
        }
    }
    else
    {
        c->data = NULL;
    }
    c->handler = NULL;
    c->next = pool->cleanup;
    pool->cleanup = c;

    return c;
}
