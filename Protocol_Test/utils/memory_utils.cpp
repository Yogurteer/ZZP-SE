// memory_utils.cpp
#include "memory_utils.h"

#if defined(__linux__) || defined(__APPLE__)
#include <unistd.h>
#include <sys/resource.h>

size_t get_peak_memory_usage() {
    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) == 0) {
        // ru_maxrss 是以 KB 为单位的最大 RSS（Resident Set Size）
        return static_cast<size_t>(usage.ru_maxrss);
    }
    return 0;
}

#elif defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#include <psapi.h>

size_t get_peak_memory_usage() {
    HANDLE hProcess = GetCurrentProcess();
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {
        return static_cast<size_t>(pmc.PeakWorkingSetSize) / 1024; // 转换为 KB
    }
    return 0;
}

#else
#error "Unsupported platform"
#endif