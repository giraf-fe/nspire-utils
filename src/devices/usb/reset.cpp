#include <nspire-utils/devices/usb/FOTG210.hpp>

namespace ntls::devices::usb {

    bool FOTG210::Reset() {
        // need to force halt, then issue reset
        // disable interrupts
        *reinterpret_cast<volatile uint32_t*>(baseAddress + capLength + FOTG210_InterruptEnable) = 0;

        // read current command register
        uint32_t commandReg = this->GetCommandRegister();
        commandReg &= ~(static_cast<uint32_t>(FOTG210_CommandRegisterBits::Run)); // clear Run bit to halt
        commandReg &= ~(static_cast<uint32_t>(FOTG210_CommandRegisterBits::IAAD_Doorbell)); // clear IAAD/Doorbell bit
        this->SetCommandRegister(commandReg);

        // wait for halt

        uint32_t spinCountdown = 10000000; // arbitrary large number to prevent infinite loop
        while (!(this->GetStatusRegister() & static_cast<uint32_t>(FOTG210_StatusRegisterBits::HostControllerHalted))) {
            // spin
            spinCountdown--;
            if (spinCountdown == 0) {
                // timeout
                return false;
            }
        }

        // issue reset
        commandReg = this->GetCommandRegister();
        commandReg |= static_cast<uint32_t>(FOTG210_CommandRegisterBits::Reset); // set Reset bit
        this->SetCommandRegister(commandReg);

        spinCountdown = 100000000; // reset countdown
        while(this->GetCommandRegister() & static_cast<uint32_t>(FOTG210_CommandRegisterBits::Reset)) {
            // spin
            spinCountdown--;
            if (spinCountdown == 0) {
                // timeout
                return false;
            }
        }

        return true;
    }

} // namespace ntls::devices::usb