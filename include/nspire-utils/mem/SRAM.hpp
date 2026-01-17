#pragma once

#include <cstdint> 
#include <cstddef>

// Used to access the SRAM memory region on the TI-Nspire calculators.
// It does this by allocating a 1MB aligned 256kB block of memory on the heap,
// then copies the contents of the SRAM region into that block.
// Then, the MMU is used to remap the original SRAM address to that heap block, 
// so the OS doesn't know the difference.
// The MMU then maps the physical SRAM to a different address, which can now be
// used for any purpose.
// Since the SRAM is on-chip memory, access times and bandwidth are much better
// than normal RAM, making it suitable for performance-intensive operations, like
// large lookup tables, scratch buffers, the stack, etc...

#include <nspire-utils/platform/MMUEditor.hpp>
#include <stdexcept>

namespace ntls::mem {

    constexpr uintptr_t SRAM_PhysicalAddress = 0xA4000000;
    constexpr size_t SRAM_Size = 256 * 1024; // 256kB

    // TODO: better name
    class SRAM_Accessor {
        platform::MMUEditor& mmuEditor;
        uint8_t originalSRAMDomain = 0;
        void* clonedSRAMBlock = nullptr;

        uintptr_t newSRAMAddress = 0;
        uint32_t (*disableInterrupts)() = nullptr;
        void (*enableInterrupts)(uint32_t) = nullptr;
        bool initSuccessful = false;

    public:
        // Throws std::bad_alloc if allocation fails.
        // Throws std::runtime_error if the SRAM region is not currently identity mapped.
        // On destruction, newSRAMAddress will be mapped to fault, choose an unused address.
        SRAM_Accessor(platform::MMUEditor& mmuEditor, uintptr_t newSRAMAddress, 
            // During the remapping, these callbacks will be called to disable/enable interrupts.
            // This is to avoid issues with interrupts occurring while the MMU tables are being modified.
            uint32_t(*disableInterrupts)() = nullptr, void(*enableInterrupts)(uint32_t) = nullptr,
            platform::MMUAccessPermission sramAccessPermission = platform::MMUAccessPermission::FullAccess,
            platform::MMUCachePolicy sramCachePolicy = platform::MMUCachePolicy::WriteBackCache,
            uint8_t sramDomain = 0
        );
        ~SRAM_Accessor();

        // Returns the address where the SRAM is now mapped to.
        uintptr_t getNewSRAMAddress() const { return newSRAMAddress; }
    };
};