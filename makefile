# 编译器设置
CC = gcc
# 编译选项：
# -g: 生成调试信息 (GDB必须)
# -Wall: 开启所有警告 (帮你看代码)
# -I include: 告诉编译器去 include 目录找头文件
CFLAGS = -g -Wall -I include

# 目标文件
TARGET = build/tinykv
# 找到 src 目录下所有的 .c 文件
SRCS = $(wildcard src/*.c)
# 把 .c 替换成 .o，并放在 build 目录下
OBJS = $(patsubst src/%.c, build/%.o, $(SRCS))

# 默认目标：直接编译
all: $(TARGET)

# 链接步骤
$(TARGET): $(OBJS)
	@mkdir -p build
	$(CC) $(CFLAGS) -o $@ $^

# 编译步骤
build/%.o: src/%.c
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

# --- 工具指令 ---

# 1. 运行程序
run: $(TARGET)
	./$(TARGET)

# 2. 清理垃圾
clean:
	rm -rf build dump.db

# 3. 内存检查 (Valgrind) - 你的救命稻草
memcheck: $(TARGET)
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(TARGET)

# 4. 调试模式 (GDB)
debug: $(TARGET)
	gdb ./$(TARGET)

.PHONY: all run clean memcheck debug