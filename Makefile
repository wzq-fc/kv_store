# 1. 定义编译器和编译选项
CC = gcc
# -Wall 开启所有警告，-g 开启调试信息
# -I./include 是最关键的！告诉编译器去当前目录的 include 文件夹里找头文件
CFLAGS = -Wall -g -I./include

# 2. 定义目录
SRC_DIR = src

# 3. 自动查找所有的 .c 文件和生成对应的 .o 文件名
# $(wildcard src/*.c) 会自动找出 src 目录下所有的 .c 文件
SRCS = $(wildcard $(SRC_DIR)/*.c)
# 将 SRCS 里的 .c 替换为 .o
OBJS = $(SRCS:.c=.o)

# 4. 定义最终生成的可执行文件名称
TARGET = kv_server

# 5. 默认目标 (敲击 make 时默认执行这里)
all: $(TARGET)

# 6. 链接目标文件生成可执行文件
# $@ 代表目标文件 (TARGET)
# $^ 代表所有的依赖文件 (OBJS)
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# 7. 编译每一个 .c 文件为 .o 文件
# $< 代表第一个依赖文件 (即对应的 .c 文件)
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# 8. 清理编译生成的中间文件和可执行文件 (敲击 make clean 时执行)
# .PHONY 声明 clean 是一个伪目标，防止目录下真的有个文件叫 clean 时产生冲突
.PHONY: clean
clean:
	rm -f $(OBJS) $(TARGET)