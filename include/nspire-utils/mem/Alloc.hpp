#pragma once

#include <cstdint>
#include <cstddef>

#include <numeric>
#include <stdexcept>

namespace ntls::mem {
    // uses syscalls to allocate and free memory
    void* platform_malloc(std::size_t size);
    void platform_free(void* ptr);

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

    // c++ style allocator
    template <typename ItemType>
    struct Allocator {
        Allocator() = default;

        template <typename OtherType>
        Allocator(const Allocator<OtherType>&) noexcept {}

        [[nodiscard]] ItemType* allocate(std::size_t n) {
            if (n > std::numeric_limits<std::size_t>::max() / sizeof(ItemType))
                throw std::bad_array_new_length();
            
            void* ptr = aligned_alloc(alignof(ItemType), n * sizeof(ItemType));
            if (!ptr)
                throw std::bad_alloc();

            return static_cast<ItemType*>(ptr);
        }
        void deallocate(ItemType* ptr, std::size_t) noexcept {
            aligned_free(static_cast<void*>(ptr));
        }

        template <typename OtherType>
        bool operator==(const Allocator<OtherType>&) const noexcept {
            return true;
        }
        template <typename OtherType>
        bool operator!=(const Allocator<OtherType>&) const noexcept {
            return false;
        }
    };

} // namespace ntls::mem
