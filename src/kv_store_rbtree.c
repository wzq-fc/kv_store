#include "kv_store_server.h"

//函数前置声明
void clear(Node* node);

//创建一个新的节点
Node* createNode(const char* key,const char* value,
                    Node* left,Node* right,Node* parent,Color color)
{
    Node* node=(Node* )malloc(sizeof(Node));
    memset(node,0,sizeof(Node));
    node->key = strdup(key);
    node->value = strdup(value);
    node->left_=left;
    node->right_=right;
    node->parent_=parent;
    node->color_=color;
    return node;
}

//创建一棵红黑树
RBTree* createRBTree()
{
    RBTree* rb=(RBTree* )malloc(sizeof(RBTree));
    memset(rb,0,sizeof(RBTree));
    rb->root_=NULL;
    rb->size=0;
    return rb;
}

//销毁一棵红黑树
void destroyRBTree(RBTree* rb)
{
    if(rb==NULL)
    {
        return;
    }
    clear(rb->root_);
    rb->root_=NULL;
    rb->size=0;
    free(rb);
}

//返回结点的颜色
Color color(Node* node)
{
    return node==NULL?BLACK:node->color_;
}

//设置节点的颜色
void setColor(Node* node,Color color)
{
    node->color_=color;
}

//返回节点的左孩子
Node* left(Node* node)
{
    return node->left_;
}

//返回节点的右孩子
Node* right(Node* node)
{
    return node->right_;
}

//返回节点的夫节点
Node* parent(Node* node)
{
    return node->parent_;
}

//左旋转
void leftRotate(RBTree*  rb,Node* node)
{
    Node* child=node->right_;
    child->parent_=node->parent_;
    if(node->parent_==NULL)
    {
        //node本身就是父节点
        rb->root_=child;
        child->parent_=NULL;
    }
    else
    {
        if(node->parent_->left_==node)
        {
            //node是父节点的左孩子
            node->parent_->left_=child;
        }
        else
        {
            //node是父节点的右孩子
            node->parent_->right_=child;
        }
    }

    node->right_=child->left_;
    if(child->left_!=NULL)
    {
        child->left_->parent_=node;
    }

    child->left_=node;
    node->parent_=child;
}

//右旋转
void rightRotate(RBTree* rb,Node* node)
{
    Node* child=node->left_;
    child->parent_=node->parent_;
    if(node->parent_==NULL)
    {
        //node本身就是根节点
        rb->root_=child;
        child->parent_=NULL;
    }
    else
    {
        if(node->parent_->left_==node)
        {
            //node是父节点的左孩子
            node->parent_->left_=child;
        }
        else
        {
            //node是父节点的右孩子
            node->parent_->right_=child;
        }
    }
    node->left_=child->right_;
    if(child->right_!=NULL)
    {
        child->right_->parent_=node;
    }
    child->right_=node;
    node->parent_=child;
}

//红黑树的插入调整操作
void fixAfterInsert(RBTree* rb,Node* node)
{
    while(color(parent(node))==RED)
    {
        if(left(parent(parent(node)))==parent(node))
        {
            //插入的节点在左子树当中
            Node* uncle=right(parent(parent(node)));
            if(color(uncle)==RED)
            {
                //情况一
                node->parent_->color_=BLACK;
                node->parent_->parent_->color_=RED;
                uncle->color_=BLACK;
                node=parent(parent(node));  //继续调整
            }
            else
            {
                //先处理情况三
                if(right(parent(node))==node)
                {
                    node=parent(node);
                    leftRotate(rb,node);
                }
                //统一处理情况二
                setColor(parent(node),BLACK);
                setColor(parent(parent(node)),RED);
                rightRotate(rb,parent(parent(node)));
                break;  //调整完成
            }
        }
        else
        {
            //插入的节点在右子树当中
            Node* uncle=left(parent(parent(node)));
            if(color(uncle)==RED)
            {
                //情况一
                node->parent_->color_=BLACK;
                node->parent_->parent_->color_=RED;
                uncle->color_=BLACK;
                node=parent(parent(node));  //继续调整
            }
            else
            {
                //先处理情况三
                if(left(parent(node))==node)
                {
                    node=parent(node);
                    rightRotate(rb,node);
                }
                //统一处理情况二
                setColor(parent(node),BLACK);
                setColor(parent(parent(node)),RED);
                leftRotate(rb,parent(parent(node)));
                break;  //调整完成
            }
        }
    }
    
    //强制root为黑色
    setColor(rb->root_,BLACK);
}

//红黑树的插入操作
int insert_rb(RBTree* rb,const char* key,const char* value)
{
    if(rb==NULL||key==NULL||value==NULL)
    {
        return -1;
    }
    if(rb->root_==NULL)
    {
        rb->size++;
        rb->root_=createNode(key,value,NULL,NULL,NULL,BLACK);
        return 0;
    }

    Node* parent=NULL;
    Node* cur=rb->root_;
    int cmp=0;
    while(cur!=NULL)
    {
        cmp=strcmp(key,cur->key);
        if(cmp<0)
        {
            parent=cur;
            cur=cur->left_;
        }
        else if(cmp>0)
        {
            parent=cur;
            cur=cur->right_;
        }
        else 
        {
            return -1;
        }
    }

    //设置当前节点的parent和颜色
    rb->size++;
    Node* node=createNode(key,value,NULL,NULL,parent,RED);
    cmp=strcmp(parent->key,key);
    if(cmp>0)
    {
        parent->left_=node;
    }
    else
    {
        parent->right_=node;
    }

    //如果父节点也是红色,需要进行红黑树的调整
    if(color(parent)==RED)
    {
        fixAfterInsert(rb,node);
    }
    return 0;
}

//红黑树的删除调整操作
void fixAfterRemove(RBTree* rb,Node* node)
{
    while(node!=rb->root_&&color(node)==BLACK)
    {
        if(left(parent(node))==node)
        {
            //删除的黑色节点在左子树
            Node* brother=right(parent(node));
            //先处理情况四
            if(color(brother)==RED)
            {
                setColor(parent(node),RED);
                setColor(brother,BLACK);
                leftRotate(rb,parent(node));
                brother=right(parent(node));
            }

            //情况三
            if(color(left(brother))==BLACK&&color(right(brother))==BLACK)
            {
                setColor(brother,RED);
                node=parent(node);
            }
            else
            {
                if(color(right(brother))!=RED)  //情况二
                {
                    setColor(brother,RED);
                    setColor(left(brother),BLACK);
                    rightRotate(rb,brother);
                    brother=right(parent(node));
                }

                //归结到情况一
                setColor(brother,color(parent(node)));
                setColor(parent(node),BLACK);
                setColor(right(brother),BLACK);
                leftRotate(rb,parent(node));
                break;
            }
        }
        else
        {
            //删除的黑色节点在右子树
            Node* brother=left(parent(node));
            //先处理情况四
            if(color(brother)==RED)
            {
                setColor(parent(node),RED);
                setColor(brother,BLACK);
                rightRotate(rb,parent(node));
                brother=left(parent(node));
            }

            //情况三
            if(color(left(brother))==BLACK&&color(right(brother))==BLACK)
            {
                setColor(brother,RED);
                node=parent(node);
            }
            else
            {
                if(color(left(brother))!=RED)  //情况二
                {
                    setColor(brother,RED);
                    setColor(right(brother),BLACK);
                    leftRotate(rb,brother);
                    brother=left(parent(node));
                }

                //归结到情况一
                setColor(brother,color(parent(node)));
                setColor(parent(node),BLACK);
                setColor(left(brother),BLACK);
                rightRotate(rb,parent(node));
                break;
            }
        }
    }

    //如果发现node指向的是红色，直接涂成黑色，调整结束
    setColor(node,BLACK);
}

//红黑树的删除操作
int remove_rb(RBTree* rb,const char* key)
{
    if(rb==NULL||rb->root_==NULL||key==NULL)
    {
        return -1;
    }

    Node* cur=rb->root_;
    int cmp=0;
    while(cur!=NULL)
    {
        cmp=strcmp(key,cur->key);
        if(cmp<0)
        {
            cur=cur->left_;
        }
        else if(cmp>0)
        {
            cur=cur->right_;
        }
        else 
        {
            break;
        }
    }

    if(cur==NULL)
    {
        //没找到
        return -1;
    }
    
    //删除cur节点----->BST的删除
    //情况三------>该节点有两个孩子
    if(cur->left_!=NULL&&cur->right_!=NULL)
    {
        Node* pre=cur->left_;
        //寻找前驱节点
        while(pre->right_!=NULL)
        {
            pre=pre->right_;
        }
        char* temp_key=cur->key;
        cur->key=pre->key;
        pre->key=temp_key;
        char* temp_value=cur->value;
        cur->value=pre->value;
        pre->value=temp_value;
        cur=pre;    //cur指向当前前驱节点
    }

    //删除cur指向的节点 情况一和情况二
    Node* child=cur->left_;
    if(cur->left_==NULL)
    {
        child=cur->right_;
    }

    if(child!=NULL)
    {
        child->parent_=cur->parent_;
        if(cur->parent_==NULL)
        {
            rb->root_=child;
        }
        else
        {
            if(cur==cur->parent_->left_)
            {
                cur->parent_->left_=child;
            }
            else
            {
                cur->parent_->right_=child;
            }
        }

        Color c=color(cur);
        rb->size--;
        free(cur->key);
        free(cur->value);
        free(cur);

        if(c==BLACK)    //删除的是黑色节点，要进行删除调整操作
        {
            fixAfterRemove(rb,child);
        }
    }
    else    //child==nullptr
    {
        if(cur->parent_==NULL)
        {
            rb->size--;
            free(cur->key);
            free(cur->value);
            free(cur);
            rb->root_=NULL;
            return 0;
        }
        else
        {
            //cur是叶子节点
            if(color(cur)==BLACK)
            {
                fixAfterRemove(rb,cur);
            }

            if(left(parent(cur))==cur)
            {
                cur->parent_->left_=NULL;
            }
            else
            {
                cur->parent_->right_=NULL;
            }

            rb->size--;
            free(cur->key);
            free(cur->value);
            free(cur);
        }
    }
    return 0;
}

//红黑树的查找操作
char* find_rb(RBTree* rb,const char* key)
{
    if(rb==NULL||rb->root_==NULL||key==NULL)
    {
        //没找到
        return NULL;
    }
    Node* node=rb->root_;
    int cmp=0;
    while(node!=NULL)
    {
        cmp=strcmp(key,node->key);
        if(cmp<0)
        {
            node=node->left_;
        }
        else if(cmp>0)
        {
            node=node->right_;
        }
        else    
        {
            return node->value;
        }
    }

    return NULL;
}

//红黑树的修改操作
int modify_rb(RBTree* rb,const char* key,const char* value)
{
    if(rb==NULL||rb->root_==NULL||key==NULL||value==NULL)
    {
        return -1;
    }
    Node* node=rb->root_;
    int cmp=0;
    while(node!=NULL)
    {
        cmp=strcmp(key,node->key);
        if(cmp<0)
        {
            node=node->left_;
        }
        else if(cmp>0)
        {
            node=node->right_;
        }
        else
        {
            //找到了
            free(node->value);
            node->value=strdup(value);
            return 0;
        }
    }
    //没找到
    return -1;
}

//红黑树的判断是否存在操作
int exist_rb(RBTree* rb,const char* key)
{
    if(rb==NULL||rb->root_==NULL||key==NULL)
    {
        //不存在
        return-1;
    }
    Node* node=rb->root_;
    int cmp=0;
    while(node!=NULL)
    {
        cmp=strcmp(key,node->key);
        if(cmp<0)
        {
            node=node->left_;
        }
        else if(cmp>0)
        {
            node=node->right_;
        }
        else
        {
            //找到了
            return 0;
        }
    }
    //没找到
    return -1;
}

//后序遍历销毁红黑树辅助函数
void clear(Node* node)
{
    if(node==NULL)
    {
        return;
    }
    clear(node->left_);
    clear(node->right_);
    free(node->key);
    free(node->value);
    free(node);
}   