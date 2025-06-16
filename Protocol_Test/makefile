# Makefile
# 编译器
CXX = g++

# 头文件搜索路径,保证make时编译能找到头文件
INCLUDE_DIRS = \
    -I. \
    -I/usr/include \
    -I$(HOME)/local/include \
    -Iinclude \
    -Iutils

# 设置 Conda 环境中的 OpenSSL 库路径
OPENSSL_DIR = /home/zzp/miniconda3/envs/myse
OPENSSL_LIB = $(OPENSSL_DIR)/lib
OPENSSL_INC = $(OPENSSL_DIR)/include

# 编译器选项
CXXFLAGS = -Wall -g -O0 -std=c++17 -I$(OPENSSL_INC)

# 链接器选项
LDFLAGS = \
    -L$(OPENSSL_LIB) \
    -L$(HOME)/local/lib \
    -lgmssl -lgmp -lpq -lssl -lcrypto \
    -Wl,-rpath,$(OPENSSL_LIB):$(HOME)/local/lib

# 可执行文件
TARGET = main

# 源文件按模块划分
UTILS_SRCS = \
    utils/utils.cpp \
    utils/encrypt.cpp \
    utils/memory_utils.cpp

PROTOCOL_SRCS = \
    include/KS2E.cpp

# 汇总所有源文件
SRCS = \
    src/overalltest.cpp \
    $(UTILS_SRCS) \
    $(PROTOCOL_SRCS) \

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(INCLUDE_DIRS)  -o $(TARGET) $(SRCS) $(LDFLAGS)

# 运行目标，并在运行后删除可执行文件
run: $(TARGET)
	./$(TARGET)
	rm -f $(TARGET)

clean:
	rm -f $(TARGET)