#include <nspire-utils/platform/EVT.hpp>

namespace {

    uint32_t ReadSystemControlRegister() {
        uint32_t value;
        asm volatile ("MRC p15, 0, %0, c1, c0, 0" : "=r" (value));
        return value;
    }
    void WriteSystemControlRegister(uint32_t value) {
        asm volatile ("MCR p15, 0, %0, c1, c0, 0" : : "r" (value));
    }

};

namespace ntls::platform {

    void SetEVTPosition(EVT_Position position) {
        uint32_t sctlr = ReadSystemControlRegister();
        if (position == EVT_Position::High) {
            sctlr |= (1 << 13);
        } else {
            sctlr &= ~(1 << 13);
        }
        WriteSystemControlRegister(sctlr);
    }

    EVT_Position GetEVTPosition() {
        uint32_t sctlr = ReadSystemControlRegister();
        if (sctlr & (1 << 13)) {
            return EVT_Position::High;
        } else {
            return EVT_Position::Low;
        }
    }

    void SetEVTValue(EVT_Vector vector, EVT_Position position, uint32_t instruction) {
        const uintptr_t baseAddress = static_cast<uintptr_t>(position);
        const uintptr_t vectorAddress = baseAddress + static_cast<uintptr_t>(vector);
        *reinterpret_cast<volatile uint32_t*>(vectorAddress) = instruction;
    }

    uint32_t GetEVTValue(EVT_Vector vector, EVT_Position position) {
        const uintptr_t baseAddress = static_cast<uintptr_t>(position);
        const uintptr_t vectorAddress = baseAddress + static_cast<uintptr_t>(vector);
        return *reinterpret_cast<volatile uint32_t*>(vectorAddress);
    }
}