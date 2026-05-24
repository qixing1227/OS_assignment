# 编译器设置
CC = gcc
# 编译选项：-Wall开启所有警告，-g开启gdb调试信息，-I指定头文件目录
CFLAGS = -Wall -g -I./include -pthread

# 目录设置
SRC_DIR = src
BUILD_DIR = build
INC_DIR = include

# 查找所有的 .c 文件
SRCS = $(wildcard $(SRC_DIR)/*.c)
# 将 .c 替换为对应 build 目录下的 .o 文件
OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))

# 最终可执行文件的名字
TARGET = $(BUILD_DIR)/os_simulation

# 默认目标
all: $(TARGET)

# 链接生成可执行文件
$(TARGET): $(OBJS)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $^

# 编译每个 .c 文件到 .o
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# 清理编译产物
clean:
	rm -rf $(BUILD_DIR)/*.o $(TARGET)

.PHONY: all clean