# 手动执行.cpp命令: g++ -o a demo-exp.cpp -lgmp -lssl -lcrypto
# Makefile
# 编译器
CXX = g++

# 头文件搜索路径
INCLUDE_DIRS = -I. -I./utils -I./include

# 设置 Conda 环境中的 OpenSSL 和 GMP 库路径
CONDA_ENV_DIR = $(HOME)/miniconda3/envs/Lr-env
LIB_DIR = $(CONDA_ENV_DIR)/lib
INC_DIR = $(CONDA_ENV_DIR)/include

# 编译器选项
CXXFLAGS = -Wall -g -std=c++17 -I$(INC_DIR)

# 链接器选项
LDFLAGS = -L$(LIB_DIR) -lgmp -lssl -lcrypto -Wl,-rpath=$(LIB_DIR)

# 可执行文件
TARGET = main

# 源文件按模块划分
UTILS_SRCS = \
    utils/utils.cpp

include_SRCS = \
    include/LPSI.cpp \
    include/receiver.cpp \
    include/sender.cpp \
    include/encrypt.cpp

# 汇总所有源文件
SRCS = \
    src/main.cpp \
    $(UTILS_SRCS) \
    $(include_SRCS) \

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(INCLUDE_DIRS)  -o $(TARGET) $(SRCS) $(LDFLAGS)

# 运行目标，并在运行后删除可执行文件
run: $(TARGET)
	./$(TARGET)
	rm -f $(TARGET)

clean:
	rm -f $(TARGET)

