#pragma once
#include <stddef.h>
#include <stdint.h>

#ifdef MEM_LEAK_DETECTION

#ifdef new
#undef new
#endif

struct MemRecord {
    void* ptr;          // 分配的内存地址
    size_t size;        // 分配的大小
    const char* file;   // 源文件名
    int line;           // 行号
    uint32_t id;        // 分配的唯一ID，用于区分相同地址的多次分配
    // 可扩展：调用栈信息、线程ID、时间戳
};

void InitializeMemTracker();
void DumpMemoryLeaks();
void* TrackMalloc(size_t size, const char* file, int line);
void TrackFree(void* ptr);

// 重载全局 new/delete
void* operator new(size_t size);
void* operator new[](size_t size);
void* operator new(size_t size, const char* file, int line);
void* operator new[](size_t size, const char* file, int line);
void operator delete(void* ptr) noexcept;
void operator delete[](void* ptr) noexcept;

//// 使用宏在 new 调用点自动插入文件名和行号
//#define new new(__FILE__, __LINE__)

#endif // MEM_LEAK_DETECTION