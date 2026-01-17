#include <nspire-utils/mem/SRAM.hpp>
#include <nspire-utils/mem/AlignedAlloc.hpp>
#include <nspire-utils/platform/Caches.hpp>

#include <cstring> // for std::memcpy

namespace ntls::mem {

    SRAM_Accessor::SRAM_Accessor(platform::MMUEditor& mmuEditor, uintptr_t newSRAMAddress, 
        uint32_t(*disableInterrupts)(), void(*enableInterrupts)(uint32_t),
        platform::MMUAccessPermission sramAccessPermission,
        platform::MMUCachePolicy sramCachePolicy,
        uint8_t sramDomain
    )
        : mmuEditor(mmuEditor), newSRAMAddress(newSRAMAddress), disableInterrupts(disableInterrupts), enableInterrupts(enableInterrupts)
    {
        this->clonedSRAMBlock = ntls::mem::aligned_alloc(0x100000, SRAM_Size); // 1MB aligned
        if(this->clonedSRAMBlock == nullptr) {
            throw std::bad_alloc();
        }

        // Copy the current SRAM contents into the allocated block.
        std::memcpy(this->clonedSRAMBlock, reinterpret_cast<void*>(SRAM_PhysicalAddress), SRAM_Size);

        // flush the data cache to ensure all data is written
        platform::FlushDataCacheRange(
            reinterpret_cast<uintptr_t>(this->clonedSRAMBlock),
            reinterpret_cast<uintptr_t>(this->clonedSRAMBlock) + SRAM_Size);
        platform::DrainWriteBuffer();

        // Check if the sram address is currently identity mapped.
        uint32_t sram_current_entry = mmuEditor.getEntry(SRAM_PhysicalAddress);
        if (sram_current_entry == 0xFFFFFFFF) {
            throw std::runtime_error("SRAM region is not currently mapped in MMU");
        }
        if(platform::MMUL1EntryMappedAddress(sram_current_entry) != SRAM_PhysicalAddress) {
            throw std::runtime_error("SRAM region is not identity mapped");
        }
        this->originalSRAMDomain = platform::MMUDomainFromEntry(sram_current_entry);

        // Remap the SRAM to the new block.
        uint32_t interruptMask = [this]() -> uint32_t {
            if(this->disableInterrupts) return this->disableInterrupts();
            return 0;
        }();
        mmuEditor.MapSection(SRAM_PhysicalAddress, reinterpret_cast<uintptr_t>(this->clonedSRAMBlock),
            static_cast<uint32_t>(platform::MMUAccessPermission::FullAccess) |
            static_cast<uint32_t>(platform::MMUCachePolicy::NonCachedNonBuffered) |
            platform::MMUSelectDomain(this->originalSRAMDomain)
        );
        mmuEditor.MapSection(0x00000000, reinterpret_cast<uintptr_t>(this->clonedSRAMBlock),
            static_cast<uint32_t>(platform::MMUAccessPermission::FullAccess) |
            static_cast<uint32_t>(platform::MMUCachePolicy::NonCachedNonBuffered) |
            platform::MMUSelectDomain(this->originalSRAMDomain)
        );

        // Map the physical SRAM to the new address.
        mmuEditor.MapSection(this->newSRAMAddress, SRAM_PhysicalAddress,
            static_cast<uint32_t>(sramAccessPermission) |
            static_cast<uint32_t>(sramCachePolicy) |
            platform::MMUSelectDomain(sramDomain)
        );
        if (this->enableInterrupts) this->enableInterrupts(interruptMask);    

        this->initSuccessful = true;
    }   

    SRAM_Accessor::~SRAM_Accessor() {
        if(this->clonedSRAMBlock) {
            if(this->initSuccessful) {
                // Restore the original SRAM mapping + contents.
                uint32_t interruptMask = [this]() -> uint32_t {
                    if(this->disableInterrupts) return this->disableInterrupts();
                    return 0;
                }();

                std::memcpy(reinterpret_cast<void*>(this->newSRAMAddress), this->clonedSRAMBlock, SRAM_Size);
                platform::FlushDataCacheRange(
                    reinterpret_cast<uintptr_t>(this->clonedSRAMBlock),
                    reinterpret_cast<uintptr_t>(this->clonedSRAMBlock) + SRAM_Size);
                platform::DrainWriteBuffer();

                mmuEditor.MapSection(SRAM_PhysicalAddress, SRAM_PhysicalAddress,
                    static_cast<uint32_t>(platform::MMUAccessPermission::FullAccess) |
                    static_cast<uint32_t>(platform::MMUCachePolicy::WriteBackCache) |
                    platform::MMUSelectDomain(this->originalSRAMDomain)
                );

                // Unmap the new SRAM address.
                mmuEditor.map(this->newSRAMAddress, 0x00000000); // fault
                
                if (this->enableInterrupts) this->enableInterrupts(interruptMask);
            }

            ntls::mem::aligned_free(this->clonedSRAMBlock);
            this->clonedSRAMBlock = nullptr;
        }
    }
        

}