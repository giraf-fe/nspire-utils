#pragma once

#include <cstdint>
#include <cstddef>

// wip

namespace ntls::devices::usb {

    // TI-Nspire has 2 FOTG210 USB controllers
    constexpr uintptr_t FOTG210_TopController_BaseAddress = 0xB0000000;
    constexpr uintptr_t FOTG210_BottomController_BaseAddress = 0xB4000000;

    // FOTG210 Register Offsets
    constexpr uintptr_t FOTG210_CommandRegister = 0x00;
    constexpr uintptr_t FOTG210_StatusRegister  = 0x04;
    constexpr uintptr_t FOTG210_InterruptEnable  = 0x08;
    constexpr uintptr_t FOTG210_CurrentMicroframe  = 0x0C;
    constexpr uintptr_t FOTG210_PeriodicListBase  = 0x14;
    constexpr uintptr_t FOTG210_AsyncListAddr  = 0x18;
    constexpr uintptr_t FOTG210_PortStatusControl  = 0x20;
    
    constexpr uintptr_t FOTG210_OTGStatusControl  = 0x70;
    constexpr uintptr_t FOTG210_OTGInterruptStatus  = 0x74;


    struct FOTG210 {
        uintptr_t baseAddress;
        FOTG210(uintptr_t baseAddr);

    };

} // namespace ntls::usb