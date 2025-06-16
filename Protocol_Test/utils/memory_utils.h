// memory_utils.h
#ifndef MEMORY_UTILS_H
#define MEMORY_UTILS_H

#include <cstddef>

/**
 * @brief 获取当前进程在整个生命周期内的最大内存使用量（以 KB 为单位）
 * @return 返回最大内存使用量（KB）
 */
size_t get_peak_memory_usage();

#endif // MEMORY_UTILS_H