# 定义编译器
CC = gcc
# 定义编译选项，-Wall 可以让编译器抛出所有潜在的警告
CFLAGS = -Wall -g

# 定义目标可执行文件名
TARGET = kv_store_server
# 定义依赖的源文件
SRCS = kv_store_server.c kv_store.c kv_store_rbtree.c

# 编译规则
$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

# 清理编译结果的规则
clean:
	rm -f $(TARGET)