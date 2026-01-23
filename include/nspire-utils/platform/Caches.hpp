#pragma once

#include <cstdint>

namespace ntls::platform {
    // in bytes
    constexpr uint32_t CacheLineSize = 32;

    void DrainWriteBuffer();

    void InvalidateBothCaches();
    void InvalidateTLB();
    
    void InvalidateICacheRange(uintptr_t start, uintptr_t end);

    void FlushDataCacheRange(uintptr_t start, uintptr_t end);
};