#pragma once

#include <cstdint>
#include <cstddef>

namespace ntls::mem {
    void* aligned_alloc(std::size_t alignment, std::size_t size);
    void aligned_free(void* ptr);


    // compatibility wrapper
    static inline void* AlignedAllocate(size_t alignment, size_t size) {
        return aligned_alloc(alignment, size);
    }
    struct AlignedDeleter {
        inline void operator()(void* ptr) const {
            aligned_free(ptr);
        }
    };

} // namespace ntls::mem
