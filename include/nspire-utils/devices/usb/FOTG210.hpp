#pragma once

#include <cstdint>
#include <cstddef>

// wip
#include <nspire-utils/devices/usb/FOTG210_Structures.hpp>

namespace ntls::devices::usb {

    // TI-Nspire has 2 FOTG210 USB controllers
    constexpr uintptr_t FOTG210_TopController_BaseAddress = 0xB0000000;
    constexpr uintptr_t FOTG210_BottomController_BaseAddress = 0xB4000000;

    
    constexpr size_t FOTG210_FrameList_Alignment = 4096; // 4KB aligned
    constexpr size_t FOTG210_FrameList_Entries = 1024; // 1024 entries
    struct FOTG210_MemoryStructures {
        uint32_t* frameList;
        QueueHead* asyncScheduleQueueHead;

        FOTG210_MemoryStructures()
            : frameList(nullptr), asyncScheduleQueueHead(nullptr) {}
        ~FOTG210_MemoryStructures();
        void allocate();
    };

    // FOTG210 Class
    class FOTG210 {
        uintptr_t baseAddress;
        uint8_t capLength;
    public:
        FOTG210(uintptr_t baseAddr);

        uint32_t GetCommandRegister() const;
        void SetCommandRegister(uint32_t value);

        uint32_t GetStatusRegister() const;
        void SetStatusRegister(uint32_t value);



        // return true on success
        bool Reset();

        // configure some interrupt stuff
        void ConfigureHostMode();

        // memory structures
        void LinkMemoryStructures(const FOTG210_MemoryStructures& memStructs);

        void ControllerStart();

        void EnableInterrupts(uint32_t interruptBits);
    };

} // namespace ntls::devices::usb