# 手动执行.cpp命令: g++ -o a demo-exp.cpp -lgmp -lssl -lcrypto
# Makefile
# 编译器
CXX = g++

# 头文件搜索路径
INCLUDE_DIRS = -I. -Iinclude -Iutils -I/usr/include

# 设置 Conda 环境中的 OpenSSL 库路径
OPENSSL_DIR = /root/miniconda3/envs/cenv
OPENSSL_LIB = $(OPENSSL_DIR)/lib
OPENSSL_INC = $(OPENSSL_DIR)/include

# 编译器选项
CXXFLAGS = -Wall -g -O0 -std=c++17 -I$(OPENSSL_INC)

# 链接器选项
LDFLAGS = -L$(OPENSSL_LIB) -lgmp -lpq -lssl -lcrypto -Wl,-rpath,$(OPENSSL_LIB)

# 可执行文件
TARGET = main

# 源文件按模块划分
UTILS_SRCS = \
    utils/utils.cpp \
    utils/encrypt.cpp \
    utils/inter.cpp \
    utils/memory_utils.cpp

PROTOCOL_SRCS = \
    include/KS2E.cpp

# 汇总所有源文件
SRCS = \
    src/main.cpp \
    $(UTILS_SRCS) \
    $(PROTOCOL_SRCS) \

# SRCS = \
#     test/DBOGdemo.cpp \

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(INCLUDE_DIRS)  -o $(TARGET) $(SRCS) $(LDFLAGS)

# 运行目标，并在运行后删除可执行文件
run: $(TARGET)
	./$(TARGET)
	rm -f $(TARGET)

clean:
	rm -f $(TARGET)

