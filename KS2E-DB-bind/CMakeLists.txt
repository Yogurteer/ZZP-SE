cmake_minimum_required(VERSION 3.12)
project(My_Mod)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)

# 设置 OpenSSL 的根目录
set(OPENSSL_ROOT_DIR "/root/base/envs/cenv")
find_package(OpenSSL REQUIRED)

# 查找 libpq 库
find_library(LIBPQ_LIBRARY pq)

# 设置 OpenSSL 相关路径
set(OPENSSL_DIR "/root/base/envs/cenv")
set(OPENSSL_LIB "${OPENSSL_DIR}/lib")

# 设置源文件
set(SOURCE_FILES
    src/i_owner.cpp
    utils/utils.cpp
    utils/encrypt.cpp
    utils/inter.cpp
    include/KS2E.cpp
)

# 设置头文件搜索路径
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/utils
)

# 查找 Python 库和头文件
find_package(Python3 REQUIRED COMPONENTS Interpreter Development)

# 创建 Python 扩展模块
add_library(KS2E_Owner MODULE ${SOURCE_FILES})

# 设置 Python C API 相关的头文件
include_directories(${Python3_INCLUDE_DIRS})

# 链接 Python 库和其他依赖
target_link_libraries(KS2E_Owner PRIVATE ${Python3_LIBRARIES} OpenSSL::Crypto OpenSSL::SSL ${LIBPQ_LIBRARY})

# 设置链接器选项
target_link_options(KS2E_Owner PRIVATE
    -L${OPENSSL_LIB}
    -Wl,-rpath,${OPENSSL_LIB}
)

# 如果需要，可以指定 Python 模块的输出目录
set_target_properties(KS2E_Owner PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/cpython_modules"
)
