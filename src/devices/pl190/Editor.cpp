#include <nspire-utils/devices/PL190.hpp>

namespace ntls::devices {
    PL190VIC_Editor::PL190VIC_Editor(uintptr_t baseAddr) 
        : baseAddress(baseAddr), oldState(SavePL190State(baseAddr)) 
    {}
    PL190VIC_Editor::~PL190VIC_Editor() {
        RestorePL190State(baseAddress, oldState);
    }
    PL190VIC_State PL190VIC_Editor::getCurrentState() {
        return SavePL190State(baseAddress);
    }
    void PL190VIC_Editor::setState(const PL190VIC_State& state) {
        RestorePL190State(baseAddress, state);
    }

    void PL190VIC_Editor::clear() {
        PL190VIC_State clearedState = {};
        RestorePL190State(baseAddress, clearedState);   
    }

    // irq status
    uint32_t PL190VIC_Editor::getIRQStatus() {
        return *reinterpret_cast<volatile uint32_t*>(baseAddress + PL190VIC_IRQStatus);
    }
    bool PL190VIC_Editor::getIRQStatus(uint8_t interruptNumber) {
        uint32_t status = getIRQStatus();
        return (status & (1 << interruptNumber)) != 0;
    }

    // fiq status
    uint32_t PL190VIC_Editor::getFIQStatus() {
        return *reinterpret_cast<volatile uint32_t*>(baseAddress + PL190VIC_FIQStatus);
    }
    bool PL190VIC_Editor::getFIQStatus(uint8_t interruptNumber) {
        uint32_t status = getFIQStatus();
        return (status & (1 << interruptNumber)) != 0;
    }

    // raw interrupt status
    uint32_t PL190VIC_Editor::getRawInterruptStatus() {
        return *reinterpret_cast<volatile uint32_t*>(baseAddress + PL190VIC_RawInterruptStatus);
    }
    bool PL190VIC_Editor::getRawInterruptStatus(uint8_t interruptNumber) {
        uint32_t status = getRawInterruptStatus();
        return (status & (1 << interruptNumber)) != 0;
    }

    // set/get interrupt types
    void PL190VIC_Editor::setInterruptTypes(uint32_t bitmask) {
        *reinterpret_cast<volatile uint32_t*>(baseAddress + PL190VIC_InterruptSelect) = bitmask;
    }
    uint32_t PL190VIC_Editor::getInterruptTypes() {
        return *reinterpret_cast<volatile uint32_t*>(baseAddress + PL190VIC_InterruptSelect);
    }
    void PL190VIC_Editor::setInterruptType(uint8_t interruptNumber, PL190InterruptType type) {
        uint32_t current = getInterruptTypes();
        if (type == PL190InterruptType::FIQ) {
            current |= (1 << interruptNumber);
        } else {
            current &= ~(1 << interruptNumber);
        }
        setInterruptTypes(current);
    }
    PL190InterruptType PL190VIC_Editor::getInterruptType(uint8_t interruptNumber) {
        uint32_t current = getInterruptTypes();
        if (current & (1 << interruptNumber)) {
            return PL190InterruptType::FIQ;
        } else {
            return PL190InterruptType::IRQ;
        }
    }

    // enable/disable interrupts
    void PL190VIC_Editor::enableInterrupts(uint32_t bitmask) {
        *reinterpret_cast<volatile uint32_t*>(baseAddress + PL190VIC_InterruptEnable) = bitmask;
    }
    void PL190VIC_Editor::disableInterrupts(uint32_t bitmask) {
        *reinterpret_cast<volatile uint32_t*>(baseAddress + PL190VIC_InterruptEnableClear) = bitmask;
    }
    uint32_t PL190VIC_Editor::getEnabledInterrupts() {
        return *reinterpret_cast<volatile uint32_t*>(baseAddress + PL190VIC_InterruptEnable);
    }
    void PL190VIC_Editor::enableInterrupt(uint8_t interruptNumber) {
        enableInterrupts(1 << interruptNumber);
    }
    void PL190VIC_Editor::disableInterrupt(uint8_t interruptNumber) {
        disableInterrupts(1 << interruptNumber);
    }
    bool PL190VIC_Editor::isInterruptEnabled(uint8_t interruptNumber) {
        uint32_t enabled = getEnabledInterrupts();
        return (enabled & (1 << interruptNumber)) != 0;
    }

    // software interrupts
    void PL190VIC_Editor::setSoftwareInterrupts(uint32_t bitmask) {
        *reinterpret_cast<volatile uint32_t*>(baseAddress + PL190VIC_SoftwareInt) = bitmask;
    }
    void PL190VIC_Editor::clearSoftwareInterrupts(uint32_t bitmask) {
        *reinterpret_cast<volatile uint32_t*>(baseAddress + PL190VIC_SoftwareIntClear) = bitmask;
    }
    uint32_t PL190VIC_Editor::getSoftwareInterrupts() {
        return *reinterpret_cast<volatile uint32_t*>(baseAddress + PL190VIC_SoftwareInt);
    }
    void PL190VIC_Editor::setSoftwareInterrupt(uint8_t interruptNumber) {
        setSoftwareInterrupts(1 << interruptNumber);
    }
    void PL190VIC_Editor::clearSoftwareInterrupt(uint8_t interruptNumber) {
        clearSoftwareInterrupts(1 << interruptNumber);
    }
    bool PL190VIC_Editor::getSoftwareInterrupt(uint8_t interruptNumber) {
        uint32_t current = getSoftwareInterrupts();
        return (current & (1 << interruptNumber)) != 0;
    }

    // protection
    void PL190VIC_Editor::setProtection(bool protect) {
        if (protect) {
            *reinterpret_cast<volatile uint32_t*>(baseAddress + PL190VIC_Protection) = 1;
        } else {
            *reinterpret_cast<volatile uint32_t*>(baseAddress + PL190VIC_Protection) = 0;
        }
    }
    bool PL190VIC_Editor::isProtected() {   
        uint32_t current = *reinterpret_cast<volatile uint32_t*>(baseAddress + PL190VIC_Protection);
        return (current & 1) != 0;
    }

    // default vector address
    void PL190VIC_Editor::setDefaultVectorAddress(uint32_t address) {
        *reinterpret_cast<volatile uint32_t*>(baseAddress + PL190VIC_DefaultVectAddr) = address;
    }
    uint32_t PL190VIC_Editor::getDefaultVectorAddress() {
        return *reinterpret_cast<volatile uint32_t*>(baseAddress + PL190VIC_DefaultVectAddr);
    }

    // vector address/control table
    void PL190VIC_Editor::setVectorAddress(uint8_t index, uint32_t address) {
        *reinterpret_cast<volatile uint32_t*>(baseAddress + PL190VIC_VectAddrTable + (index * sizeof(uint32_t))) = address;
    }
    uint32_t PL190VIC_Editor::getVectorAddress(uint8_t index) {
        return *reinterpret_cast<volatile uint32_t*>(baseAddress + PL190VIC_VectAddrTable + (index * sizeof(uint32_t)));
    }
    void PL190VIC_Editor::setVectorControl(uint8_t index, uint32_t control) {
        *reinterpret_cast<volatile uint32_t*>(baseAddress + PL190VIC_VectControlTable + (index * sizeof(uint32_t))) = control;
    }
    uint32_t PL190VIC_Editor::getVectorControl(uint8_t index) {
        return *reinterpret_cast<volatile uint32_t*>(baseAddress + PL190VIC_VectControlTable + (index * sizeof(uint32_t)));
    }
};

