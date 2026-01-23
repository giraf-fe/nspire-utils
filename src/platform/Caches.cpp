#include <nspire-utils/platform/Caches.hpp>

namespace ntls::platform {
    void DrainWriteBuffer() {
        __asm__ volatile (
            "mcr p15, 0, %0, c7, c10, 4"
            : : "r"(0) : "memory"
        );
    }

    void InvalidateBothCaches() {
        __asm__ volatile (
            "mcr p15, 0, %0, c7, c7, 0"
            : : "r"(0) : "memory"
        );
    }
    void InvalidateTLB() {
        __asm__ volatile (
            "mcr p15, 0, %0, c8, c7, 0"
            : : "r"(0) : "memory"
        );
        DrainWriteBuffer();
    }
  
    void InvalidateICacheRange(uintptr_t start, uintptr_t end) {
        uintptr_t addr = start & ~(CacheLineSize - 1);
        while (addr < end) {
            __asm__ volatile (
                "mcr p15, 0, %0, c7, c5, 1"
                : : "r"(addr) : "memory"
            );
            addr += CacheLineSize;
        }
    }
    
    void FlushDataCacheRange(uintptr_t start, uintptr_t end) {
        uintptr_t addr = start & ~(CacheLineSize - 1);
        while (addr < end) {
            __asm__ volatile (
                "mcr p15, 0, %0, c7, c10, 1"
                : : "r"(addr) : "memory"
            );
            addr += CacheLineSize;
        }
    }
};