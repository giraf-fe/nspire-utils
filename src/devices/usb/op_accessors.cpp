#include <nspire-utils/devices/usb/FOTG210.hpp>

namespace ntls::devices::usb {

    uint32_t FOTG210::GetCommandRegister() const {
        return *reinterpret_cast<volatile uint32_t*>(baseAddress + capLength + FOTG210_CommandRegister);
    }

    void FOTG210::SetCommandRegister(uint32_t value) {
        *reinterpret_cast<volatile uint32_t*>(baseAddress + capLength + FOTG210_CommandRegister) = value;
    }

    uint32_t FOTG210::GetStatusRegister() const {
        return *reinterpret_cast<volatile uint32_t*>(baseAddress + capLength + FOTG210_StatusRegister);
    }

    void FOTG210::SetStatusRegister(uint32_t value) {
        *reinterpret_cast<volatile uint32_t*>(baseAddress + capLength + FOTG210_StatusRegister) = value;
    }

} // namespace ntls::devices::usb