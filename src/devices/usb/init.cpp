#include <nspire-utils/devices/usb/FOTG210.hpp>

namespace ntls::devices::usb {

    void FOTG210::ConfigureHostMode() {
        // setup global interrupt mask
        *reinterpret_cast<volatile uint32_t*>(baseAddress + capLength + FOTG210_GlobalInterruptMask) =
            static_cast<uint32_t>(FOTG210_GlobalInterruptMaskBits::MaskDeviceInterrupts) |
            static_cast<uint32_t>(FOTG210_GlobalInterruptMaskBits::MaskOTGInterrupts) |
            static_cast<uint32_t>(FOTG210_GlobalInterruptMaskBits::InterruptPolarity); // active high


        // request host mode / A-bus
        uint32_t otgStatus = *reinterpret_cast<volatile uint32_t*>(baseAddress + capLength + FOTG210_OTGStatusControl);
        otgStatus &= ~static_cast<uint32_t>(FOTG210_OTGStatusControlBits::A_Device_BusDrop); // clear bus drop
        otgStatus |= static_cast<uint32_t>(FOTG210_OTGStatusControlBits::A_Device_BusRequest); // set bus request
        *reinterpret_cast<volatile uint32_t*>(baseAddress + capLength + FOTG210_OTGStatusControl) = otgStatus;
    }

    void FOTG210::LinkMemoryStructures(const FOTG210_MemoryStructures& memStructs) {
        // link frame list
        *reinterpret_cast<volatile uint32_t*>(baseAddress + capLength + FOTG210_PeriodicListBase) =
            reinterpret_cast<uintptr_t>(memStructs.frameList);

        // link async schedule
        *reinterpret_cast<volatile uint32_t*>(baseAddress + capLength + FOTG210_AsyncListAddr) =
            reinterpret_cast<uintptr_t>(memStructs.asyncScheduleQueueHead);
    }

    void FOTG210::ControllerStart() {
        this->SetCommandRegister(static_cast<uint32_t>(FOTG210_CommandRegisterBits::Run));
        this->GetCommandRegister(); // read to ensure posted write completes
    }

    void FOTG210::EnableInterrupts(uint32_t interruptBits) {
        *reinterpret_cast<volatile uint32_t*>(baseAddress + capLength + FOTG210_InterruptEnable) = interruptBits;
    }

} // namespace ntls::devices::usb