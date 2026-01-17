#pragma once

#include <cstdint>
#include <cstddef>

namespace ntls::devices {
    // PL190 at 0xDC000000
    // https://new.hackspire.org/Memory-mapped_IO_ports_on_CX_II/#dc000000---interrupt-controller
    constexpr uintptr_t PL190_BaseAddress = 0xDC000000;

    // TI Nspire interrupt sources
    // https://new.hackspire.org/Interrupts/
    // https://github.com/nspire-emus/firebird/blob/master/core/interrupt.h
    enum class TiNspireInterruptSource : uint8_t {
        SerialUART = 1,
        WatchdogTimer = 3,
        RTC = 4,
        GPIO = 7,
        USB = 8, 
        ADC = 11,
        PMU = 15,
        Keypad = 16,
        FastTimer = 17,
        FirstTimer = 18, // Timer1
        SecondTimer = 19, // Timer2
        I2C = 20,
        LCDController = 21
    };

    // Helper to create bitmask from interrupt sources
    template<TiNspireInterruptSource... sources>
    constexpr uint32_t TiNspireInterruptSourceBitmask = ((1 << static_cast<uint8_t>(sources)) | ...);

    enum class PL190InterruptType : uint8_t {
        IRQ = 0,
        FIQ = 1
    };

    // PL190 Register Offsets
    constexpr uintptr_t PL190_VICIRQStatus              = 0x00;
    constexpr uintptr_t PL190_VICFIQStatus              = 0x04;
    constexpr uintptr_t PL190_VICRawInterruptStatus     = 0x08;
    constexpr uintptr_t PL190_VICInterruptSelect        = 0x0C;
    constexpr uintptr_t PL190_VICInterruptEnable        = 0x10;
    constexpr uintptr_t PL190_VICInterruptEnableClear   = 0x14; // Write to clear, this allows for atomic disable
    constexpr uintptr_t PL190_VICSoftwareInt            = 0x18;
    constexpr uintptr_t PL190_VICSoftwareIntClear       = 0x1C;
    constexpr uintptr_t PL190_VICProtection             = 0x20;
    constexpr uintptr_t PL190_VICVectAddr               = 0x30;
    constexpr uintptr_t PL190_VICDefaultVectAddr        = 0x34;

    constexpr uintptr_t PL190_VICVectAddrTable          = 0x100; // Start of vector address table
    constexpr uintptr_t PL190_VICVectControlTable       = 0x200; // Start of vector control registers

    constexpr uint32_t PL190_VICVectControlEnabledBit = (1 << 5);

    struct PL190VIC_State {
        uint32_t intSelect;
        uint32_t intEnable;
        uint32_t softwareInt;
        uint32_t protection;
        uint32_t vectAddr[16];
        uint32_t vectControl[16];
    };
    PL190VIC_State SavePL190State(const uintptr_t baseAddress);

    // Check MMU before restoring!
    void RestorePL190State(const uintptr_t baseAddress, const PL190VIC_State& state);

    // Copies the Interrupt controller state and allows editing before restoring.
    // Restored on destruction.
    class PL190VIC_Editor {
        uintptr_t baseAddress;
        PL190VIC_State oldState;
    public:
        PL190VIC_Editor(uintptr_t baseAddr);
        ~PL190VIC_Editor();

        // sets everything to 0, all interrupts disabled
        void clear();

        uint32_t getIRQStatus();
        uint32_t getFIQStatus();
        uint32_t getRawInterruptStatus();
        void setInterruptTypes(uint32_t bitmask);
        uint32_t getInterruptTypes();
        void enableInterrupts(uint32_t bitmask);
        void disableInterrupts(uint32_t bitmask);
        uint32_t getEnabledInterrupts();
        void setSoftwareInterrupts(uint32_t bitmask);
        void clearSoftwareInterrupts(uint32_t bitmask);
        uint32_t getSoftwareInterrupts();

        bool getIRQStatus(uint8_t interruptNumber);
        bool getFIQStatus(uint8_t interruptNumber);
        bool getRawInterruptStatus(uint8_t interruptNumber);
        void setInterruptType(uint8_t interruptNumber, PL190InterruptType type);
        PL190InterruptType getInterruptType(uint8_t interruptNumber);
        void enableInterrupt(uint8_t interruptNumber);
        void disableInterrupt(uint8_t interruptNumber);
        bool isInterruptEnabled(uint8_t interruptNumber);
        void setSoftwareInterrupt(uint8_t interruptNumber);
        void clearSoftwareInterrupt(uint8_t interruptNumber);
        bool getSoftwareInterrupt(uint8_t interruptNumber);

        void setProtection(bool protect);
        bool isProtected();

        // There is no get/set for vectAddr, please read/ack directly when needed
        void setDefaultVectorAddress(uint32_t address);
        uint32_t getDefaultVectorAddress();

        // for the table entries
        void setVectorAddress(uint8_t index, uint32_t address);
        uint32_t getVectorAddress(uint8_t index);
        void setVectorControl(uint8_t index, uint32_t control);
        uint32_t getVectorControl(uint8_t index);


        // helpers for state, does not affect old state
        PL190VIC_State getCurrentState();
        void setState(const PL190VIC_State& state);
    };
};