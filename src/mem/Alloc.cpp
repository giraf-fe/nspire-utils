#include <nspire-utils/mem/Alloc.hpp>
#include <os.h>
#include <syscall.h>

namespace {
    // alignment should be power of two
    int is_power_of_two_size_t(size_t x) {
        return x && ((x & (x - 1)) == 0);
    }
}

namespace ntls::mem {
    
    void* platform_malloc(std::size_t size) {
        return syscall<e_malloc, void*, unsigned int>(size);
    }
    void platform_free(void* ptr) {
        syscall<e_free, void, void*>(ptr);
    }

    void* aligned_alloc(std::size_t alignment, std::size_t size) {
        // sanity checks
        if(size == 0) return nullptr;
        if(!is_power_of_two_size_t(alignment)) return nullptr;
        if(alignment < sizeof(void*)) return nullptr;

        // allocate extra space to store original pointer and to align
        size_t trueBlockSize = size + alignment - 1 + sizeof(void*);
        void* rawPtr = platform_malloc(trueBlockSize);
        if(rawPtr == nullptr) return nullptr;
        
        // calculate aligned address
        uintptr_t start = (uintptr_t)rawPtr + sizeof(void*);
        uintptr_t aligned = (start + (alignment - 1)) & ~(uintptr_t)(alignment - 1); // masks lower bits

        // store original pointer just before aligned address
        reinterpret_cast<void**>(aligned)[-1] = rawPtr;
        return reinterpret_cast<void*>(aligned);
    }

    void aligned_free(void* ptr) {
        if(ptr == nullptr) return;
        void* rawPtr = reinterpret_cast<void**>(ptr)[-1];
        platform_free(rawPtr);
    }

} // namespace ntls